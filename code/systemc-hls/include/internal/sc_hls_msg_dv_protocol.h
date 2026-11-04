/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*
sc_hls_msg_dv_protocol.h

Message Passing without Back-Pressure: Protocol Implementation (Internal)

This file contains the internal CRTP-based protocol implementations for the
msg_dv (dat/vld) ports and channels. These are implementation details that
users should not need to interact with directly.

The protocol engines implement the pre/post scheduling logic that coordinates
with the channel_manager for throughput-accurate simulation.

msg_dv Reader Semantics
-----------------------
Message passing without back-pressure has no rdy signal: the producer drives
data whenever it has data available and cannot be stalled by the consumer. The
reader buffers an unconsumed beat ("sticky" buffer) until the user calls
pop()/pop_nb(). If a new vld arrives while the buffer is still occupied,
the new value overwrites the previous one (bus-faithful: in real dat/vld
hardware the previous beat would be lost on the wire).

This is the correct behavior for legitimate msg_dv use cases such as decimators
or samplers, but is also a common bug for tight 1:1 consumers. The reader
can therefore report each overrun event with a configurable severity:
  - dv_overrun_policy::Error
  - dv_overrun_policy::Warning
  - dv_overrun_policy::Mute    (default, overruns are not reported)

The policy can be set globally via set_default_dv_overrun_policy() and
overridden per-port via set_overrun_policy().
EXPERIMENTAL: the overrun policy API is not part of the draft standard.

Note: Random stall injection (rand_stall) is intentionally NOT supported on
the msg_dv reader. It would require a rdy back-pressure path that msg_dv does
not provide; injecting stalls would silently drop data on the wire.
Latency back-annotation on the writer is always compiled in but remains
dormant unless explicitly configured at runtime.

Version: 3.1
*/

#pragma once

#include <systemc.h>
#include <optional>
#include <vector>
#include "sc_hls_interfaces.h"
#include "sc_hls_logging.h"
#include "sc_hls_rand_stall.h"
#include "sc_hls_manager.h"
#include "sc_hls_msg_dv.h"

namespace sc_hls { namespace msg_lib {

    // ========================================================================
    // dv_overrun_policy and global default
    // ========================================================================
    // The dv_overrun_policy enum, the dv_overrun_policy_value helper, and the
    // global-default free functions (set_default_dv_overrun_policy /
    // get_default_dv_overrun_policy) are the public API and are defined in
    // sc_hls_msg_dv.h, included above. The protocol engine below refers to them
    // directly.

    // ========================================================================
    // Reader Protocol Implementation (for msg_dv_in ports and msg_dv_channel readers)
    // ========================================================================

    template <typename Derived, typename Message>
    class msg_dv_reader_protocol : public sc_hls::msg_lib::msg_in_if<Message>,
                              public sc_hls::msg_lib::sim_helper_if<0>,
                              public virtual sc_hls::msg_lib::logger<Message>
    {
    public:
        // Internal buffer state - made public for tracing/debugging purposes.
        // These represent the buffered data in the input port and are useful
        // for waveform analysis and protocol verification.
        // Note: While public, these should not be modified by user code.
        Message in_buf_dat;
        bool in_buf_vld = false;

    protected:
        // Per-port overrun policy override. When unset, the global default
        // (sc_hls::msg_lib::get_default_dv_overrun_policy()) is used.
        dv_overrun_policy_value local_overrun_policy_;

        // Port-access mode (see enable_port_access()). When true, the endpoint
        // is NOT registered with the channel_manager (so its pre()/post() are
        // never scheduled) and the user process reads vld/dat directly. The
        // msg_dv reader has no rdy pin, so there is no idle pin to initialize.
        // pop()/pop_nb()/peek_nb() are forbidden in this mode.
        bool port_access_ = false;

        Derived& derived() { return static_cast<Derived&>(*this); }
        const Derived& derived() const { return static_cast<const Derived&>(*this); }

        // Verify Reset was called and that the caller is the owning process.
        // All API functions must call this before doing any work.
        void check_api_preconditions(const char* func_name)
        {
            if (port_access_) {
                SC_REPORT_ERROR("SC_HLS",
                    (std::string(func_name) + "() is forbidden on port <" +
                     std::string(derived().name()) +
                     ">: port access is enabled; read vld/dat directly.").c_str());
                return;
            }

            if (!this->was_reset) {
                SC_REPORT_FATAL("SC_HLS-101",
                    (std::string(func_name) + "() called on port that was not reset: " +
                     std::string(derived().name())).c_str());
            }

            sc_process_handle current = sc_get_current_process_handle();
            if (current != this->owning_process) {
                std::ostringstream ss;
                ss << func_name << "() called from wrong process.\n"
                   << "  Port: " << derived().name() << "\n"
                   << "  Expected process: " << this->owning_process.name() << "\n"
                   << "  Actual process: " << current.name() << "\n"
                   << "  Each port must be accessed from only one process.";
                SC_REPORT_ERROR("SC_HLS-102", ss.str().c_str());
            }
        }

        // Resolve effective overrun policy: local override if set, else global.
        dv_overrun_policy effective_overrun_policy() const {
            if (local_overrun_policy_.has_value()) {
                return *local_overrun_policy_;
            }
            return sc_hls::msg_lib::get_default_dv_overrun_policy();
        }

        // Report a buffer overrun (new vld arrived while previous beat still
        // unconsumed) at the user-configured severity.
        void report_overrun() {
            dv_overrun_policy p = effective_overrun_policy();
            if (p == dv_overrun_policy::Mute) return;
            if (p == dv_overrun_policy::Unset) return;

            std::ostringstream ss;
            ss << "msg_dv_in consumer overrun on port: " << derived().name()
               << " (new vld arrived while buffer still holds previous beat; "
               << "previous data is lost). This is legitimate for decimators/samplers; "
               << "set policy to Mute or Warning if intentional.";

            if (p == dv_overrun_policy::Error) {
                SC_REPORT_ERROR("SC_HLS-401", ss.str().c_str());
            } else {
                SC_REPORT_WARNING("SC_HLS-401", ss.str().c_str());
            }
        }

    public:
        // ====================================================================
        // Port-access mode
        // ====================================================================
        // Opt in to reading the vld/dat pins directly from the user process
        // instead of using pop()/pop_nb()/peek_nb(). Must be called during
        // elaboration (i.e. in the module constructor). In this mode the reset
        // function (reset_pop) is forbidden; the user process reads the pins,
        // and the endpoint is never registered with the channel_manager. The
        // msg_dv reader has no rdy pin, so there is no idle pin to drive.
        void enable_port_access() {
            if (sc_core::sc_get_status() != sc_core::SC_ELABORATION) {
                SC_REPORT_ERROR("SC_HLS",
                    (std::string("enable_port_access() must be called during elaboration "
                     "(in the module constructor) on port <") +
                     std::string(derived().name()) + ">.").c_str());
                return;
            }
            port_access_ = true;
        }

        bool port_access_enabled() const { return port_access_; }

        // ====================================================================
        // Per-port overrun policy API
        // ====================================================================

        // Set an explicit overrun policy on this port/channel reader.
        // Overrides the global default for this port only.
        void set_overrun_policy(dv_overrun_policy p) {
            local_overrun_policy_ = p;
        }

        // Returns the *effective* policy in use for this port (local if set,
        // otherwise the current global default).
        dv_overrun_policy get_overrun_policy() const {
            return effective_overrun_policy();
        }

        // Clear any local override; this port will subsequently follow the
        // global default again.
        void clear_overrun_policy() {
            local_overrun_policy_.reset();
        }

        // pop: Blocking read - waits until data is available
        virtual Message pop() override
        {
            check_api_preconditions("pop");

            if (!in_buf_vld)
            {
                do
                {
                    sc_core::wait();
                } while (!in_buf_vld);
            }
            in_buf_vld = false;
            this->emit(in_buf_dat, "POP");
            return in_buf_dat;
        }

        // pop_nb: Non-blocking read - returns true if data was available
        virtual bool pop_nb(Message &m) override
        {
            check_api_preconditions("pop_nb");

            if (in_buf_vld)
            {
                m = in_buf_dat;
                in_buf_vld = false;
                this->emit(m, "POP_NB");
                return true;
            } else {
                return false;
            }
        }

        // peek_nb: Non-blocking peek - returns true if data is available (doesn't consume)
        virtual bool peek_nb(Message &m) override
        {
            check_api_preconditions("peek_nb");

            if (in_buf_vld)
            {
                m = in_buf_dat;
                return true;
            } else {
                return false;
            }
        }

        // pre: Called before delta cycle - samples input signals.
        //
        // msg_dv semantics:
        //  - vld=0: buffer is unchanged (sticky — holds last beat until consumed).
        //  - vld=1, buffer empty: accept new beat into buffer.
        //  - vld=1, buffer full: overrun. New beat overwrites previous one
        //    (bus-faithful, since msg_dv has no rdy back-pressure on the wire).
        //    The overrun is reported at the configured policy severity.
        virtual void pre() override
        {
            if (derived().vld.read())
            {
                if (in_buf_vld) {
                    // Previous beat not yet consumed; new beat overwrites it.
                    report_overrun();
                }
                in_buf_vld = true;
                in_buf_dat = derived().dat.read();
            }
            // else: buffer is sticky — leave in_buf_vld / in_buf_dat alone.
        }

        // post: Called after delta cycle - no ready signal in the msg_dv protocol
        virtual void post() override
        {
        }

        // reset_pop: Reset the reader state and register with channel_manager
        virtual void reset_pop() override
        {
            ensure_started();
            sc_hls::msg_lib::logger<Message>::port_name_ = derived().name();

            if (port_access_) {
                // Port access is enabled: reset is forbidden. The user process
                // reads vld/dat directly.
                SC_REPORT_ERROR("SC_HLS",
                    (std::string("reset_pop() is forbidden on port <") +
                     std::string(derived().name()) +
                     ">: port access is enabled; do not call reset (read vld/dat directly).").c_str());
                return;
            }

            get_chanMgr().add_reader(this);
            in_buf_vld = false;
        }
    };

    // ========================================================================
    // Writer Protocol Implementation (for msg_dv_out ports and msg_dv_channel writers)
    // ========================================================================

    template <typename Derived, typename Message>
    class msg_dv_writer_protocol : public sc_hls::msg_lib::msg_out_if<Message>,
                              public sc_hls::msg_lib::sim_helper_if<1>,
                              public sc_hls::msg_lib::back_annotation_if,
                              public virtual sc_hls::msg_lib::logger<Message>
    {
    public:
        // Internal buffer state - made public for tracing/debugging purposes.
        // These represent the buffered data in the output port and are useful
        // for waveform analysis and protocol verification.
        // Note: While public, these should not be modified by user code.
        Message out_buf_dat;
        bool out_buf_vld = false;

    protected:
        // ====================================================================
        // Back-Annotation Support (Circular Buffer + Cycle Counter)
        // ====================================================================
        //
        // When annotated (annotated_=true), data written via push() is delayed
        // by 'latency_' clock cycles before appearing on the output signal (vld/dat).
        //
        // When not annotated (annotated_=false, the default), the hot path is
        // a single bool check that the branch predictor eliminates after a few
        // cycles.
        //
        // The msg_dv protocol has no back-pressure (no rdy signal). The consumer must
        // accept data every cycle when vld is asserted. Therefore:
        //   - out_buf_vld is cleared every cycle in pre() (data always consumed)
        //   - The circular buffer still provides the latency delay
        //
        // See sc_hls_msg_protocol.h for full documentation of the
        // circular buffer + cycle counter design.
        // ====================================================================

        // Slot in the circular delay buffer
        struct annotation_slot {
            Message data;
            bool valid = false;
            uint64_t output_cycle = 0;  // Cycle when this data should be output
        };

        std::vector<annotation_slot> delay_buffer_;
        size_t write_idx_ = 0;          // Next write position (circular)
        size_t read_idx_ = 0;           // Next read position (circular)
        size_t items_in_buffer_ = 0;    // Current occupancy
        size_t capacity_ = 1;           // Buffer size (max items in flight)
        uint64_t current_cycle_ = 0;    // Global cycle counter
        int latency_ = 0;               // Delay in cycles
        bool annotated_ = false;        // True if latency > 0

        // Port-access mode (see enable_port_access()). When true, the endpoint
        // is NOT registered with the channel_manager (so its pre()/post() are
        // never scheduled) and the user process drives vld/dat directly.
        // push()/push_nb() are forbidden in this mode.
        bool port_access_ = false;

        Derived& derived() { return static_cast<Derived&>(*this); }
        const Derived& derived() const { return static_cast<const Derived&>(*this); }

        // Verify Reset was called and that the caller is the owning process.
        // All API functions must call this before doing any work.
        void check_api_preconditions(const char* func_name)
        {
            if (port_access_) {
                SC_REPORT_ERROR("SC_HLS",
                    (std::string(func_name) + "() is forbidden on port <" +
                     std::string(derived().name()) +
                     ">: port access is enabled; drive vld/dat directly.").c_str());
                return;
            }

            if (!this->was_reset) {
                SC_REPORT_FATAL("SC_HLS-101",
                    (std::string(func_name) + "() called on port that was not reset: " +
                     std::string(derived().name())).c_str());
            }

            sc_process_handle current = sc_get_current_process_handle();
            if (current != this->owning_process) {
                std::ostringstream ss;
                ss << func_name << "() called from wrong process.\n"
                   << "  Port: " << derived().name() << "\n"
                   << "  Expected process: " << this->owning_process.name() << "\n"
                   << "  Actual process: " << current.name() << "\n"
                   << "  Each port must be accessed from only one process.";
                SC_REPORT_ERROR("SC_HLS-102", ss.str().c_str());
            }
        }

    public:

        // ====================================================================
        // Port-access mode
        // ====================================================================
        // Opt in to driving the vld/dat pins directly from the user process
        // instead of using push()/push_nb(). Must be called during elaboration
        // (i.e. in the module constructor). In this mode the reset function
        // (reset_push) is forbidden; the user process drives the pins, including
        // their idle values, and the endpoint is never registered with the
        // channel_manager (so multiple-writer detection is also skipped — the
        // user owns the pins).
        void enable_port_access() {
            if (sc_core::sc_get_status() != sc_core::SC_ELABORATION) {
                SC_REPORT_ERROR("SC_HLS",
                    (std::string("enable_port_access() must be called during elaboration "
                     "(in the module constructor) on port <") +
                     std::string(derived().name()) + ">.").c_str());
                return;
            }
            port_access_ = true;
        }

        bool port_access_enabled() const { return port_access_; }



        // Configure latency and capacity for back-annotation.
        // Must be called before simulation starts (typically during elaboration).
        // latency: Number of clock cycles to delay data (0 = no delay)
        // capacity: Maximum items in flight (must be >= 1)
        void set_latency_capacity(int latency, int capacity) override {
            if (latency <= 0) {
                annotated_ = false;
                return;
            }

            if (capacity < 1) {
                capacity = 1;
            }

            latency_ = latency;
            capacity_ = static_cast<size_t>(capacity);
            delay_buffer_.resize(capacity_);

            // Initialize all slots as invalid
            for (auto& slot : delay_buffer_) {
                slot.valid = false;
                slot.output_cycle = 0;
            }

            write_idx_ = 0;
            read_idx_ = 0;
            items_in_buffer_ = 0;
            current_cycle_ = 0;
            annotated_ = true;

            // Register annotation in channel_manager for end-of-simulation validation.
            // This allows detection of annotations set on inactive objects (channels
            // that are never reset because a bound port is used instead).
            get_chanMgr().register_annotation(derived().name(), latency, capacity);
        }

        // Get the configured latency
        int get_latency() const override {
            return latency_;
        }

        // Get the configured capacity
        int get_capacity() const override {
            return static_cast<int>(capacity_);
        }

        // Get the hierarchical path name for annotation file matching
        std::string get_annotation_path() const override {
            return std::string(derived().name());
        }

        // push: Blocking write - waits until buffer has space
        virtual void push(const Message &m) override
        {
            check_api_preconditions("push");

            if (annotated_) {
                // ----------------------------------------------------------------
                // Annotated mode: Write to circular buffer with delayed output.
                // Wait until buffer has space (items_in_buffer_ < capacity_).
                // ----------------------------------------------------------------
                while (items_in_buffer_ >= capacity_) {
                    sc_core::wait();
                }

                // Write data to current write position with output timestamp
                delay_buffer_[write_idx_].data = m;
                delay_buffer_[write_idx_].valid = true;
                delay_buffer_[write_idx_].output_cycle = current_cycle_ + latency_;

                // Advance write index (circular wrap using comparison, not modulo)
                ++write_idx_;
                if (write_idx_ >= capacity_) {
                    write_idx_ = 0;
                }

                ++items_in_buffer_;
                this->emit(m, "PUSH");
                return;
            }

            // ----------------------------------------------------------------
            // Non-annotated mode: Original behavior.
            // Wait until output buffer is free, then write directly.
            // ----------------------------------------------------------------
            if (out_buf_vld)
            {
                do
                {
                    sc_core::wait();
                } while (out_buf_vld);
            }
            out_buf_vld = true;
            out_buf_dat = m;
            this->emit(m, "PUSH");
        }

        // push_nb: Non-blocking write - returns true if write succeeded
        virtual bool push_nb(const Message &m) override
        {
            check_api_preconditions("push_nb");

            if (annotated_) {
                // ----------------------------------------------------------------
                // Annotated mode: Try to write to circular buffer.
                // Returns false if buffer is full.
                // ----------------------------------------------------------------
                if (items_in_buffer_ >= capacity_) {
                    return false;
                }

                // Write data to current write position with output timestamp
                delay_buffer_[write_idx_].data = m;
                delay_buffer_[write_idx_].valid = true;
                delay_buffer_[write_idx_].output_cycle = current_cycle_ + latency_;

                // Advance write index (circular wrap using comparison, not modulo)
                ++write_idx_;
                if (write_idx_ >= capacity_) {
                    write_idx_ = 0;
                }

                ++items_in_buffer_;
                this->emit(m, "PUSH_NB");
                return true;
            }

            // ----------------------------------------------------------------
            // Non-annotated mode: Original behavior.
            // ----------------------------------------------------------------
            if (!out_buf_vld)
            {
                out_buf_vld = true;
                out_buf_dat = m;
                this->emit(m, "PUSH_NB");
                return true;
            }
            else
            {
                return false;
            }
        }

        // pre: Called before delta cycle - manages buffer (msg_dv has no rdy signal)
        virtual void pre() override
        {
            if (annotated_) {
                // ----------------------------------------------------------------
                // Annotated mode: Circular buffer management.
                //
                // Step 1: Increment cycle counter
                // Step 2: msg_dv has no handshake - output is always consumed each cycle
                // Step 3: If head item is ready, output it
                // ----------------------------------------------------------------

                // Step 1: Increment cycle counter
                ++current_cycle_;

                // Step 2: msg_dv has no back-pressure - output is consumed every cycle
                out_buf_vld = false;

                // Step 3: If head item is ready, output it
                if (items_in_buffer_ > 0) {
                    annotation_slot& head = delay_buffer_[read_idx_];

                    // Output only when current cycle reaches the item's output_cycle
                    if (head.valid && current_cycle_ >= head.output_cycle) {
                        out_buf_vld = true;
                        out_buf_dat = head.data;
                        head.valid = false;

                        // Advance read index (circular wrap using comparison, not modulo)
                        ++read_idx_;
                        if (read_idx_ >= capacity_) {
                            read_idx_ = 0;
                        }

                        --items_in_buffer_;
                    }
                }
                return;
            }

            // ----------------------------------------------------------------
            // Non-annotated mode: Original behavior.
            // msg_dv has no handshake - data is consumed every cycle.
            // ----------------------------------------------------------------
            out_buf_vld = false;
        }

        // post: Called after delta cycle - drives valid and data signals
        virtual void post() override
        {
            if (out_buf_vld)
            {
                derived().vld.write(true);
                derived().dat.write(out_buf_dat);
            }
            else
            {
                derived().vld.write(false);
            }
        }

        // reset_push: Reset the writer state and register with channel_manager
        virtual void reset_push() override
        {
            ensure_started();

            // Mark this port/channel as active in the annotation table.
            // If an annotation was set on this object, it is now confirmed active.
            get_chanMgr().mark_annotation_reset(derived().name());

            sc_hls::msg_lib::logger<Message>::port_name_ = derived().name();

            if (port_access_) {
                // Port access is enabled: reset is forbidden. The user process
                // drives vld/dat directly (including idle values).
                SC_REPORT_ERROR("SC_HLS",
                    (std::string("reset_push() is forbidden on port <") +
                     std::string(derived().name()) +
                     ">: port access is enabled; do not call reset (drive vld/dat directly).").c_str());
                return;
            }
            get_chanMgr().add_writer(this);
            out_buf_vld = false;
            out_buf_dat = Message{};

            // Reset circular buffer state
            write_idx_ = 0;
            read_idx_ = 0;
            items_in_buffer_ = 0;
            current_cycle_ = 0;
            for (auto& slot : delay_buffer_) {
                slot.valid = false;
                slot.output_cycle = 0;
            }
        }
    };

} } // namespace sc_hls::msg_lib
