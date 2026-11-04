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
sc_hls_msg_protocol.h

Message Passing with Flow Control: Protocol Implementation (Internal)

This file contains the internal CRTP-based protocol implementations for the
msg (dat/vld/rdy) ports and channels. These are implementation details that
users should not need to interact with directly.

The protocol engines implement the pre/post scheduling logic that coordinates
with the channel_manager for throughput-accurate simulation.

Random stall injection and latency back-annotation are always compiled in.
Both features remain dormant unless explicitly configured at runtime.

Version: 3.0
*/

#pragma once

#include <systemc.h>
#include <optional>
#include <vector>
#include "sc_hls_interfaces.h"
#include "sc_hls_logging.h"
#include "sc_hls_rand_stall.h"
#include "sc_hls_manager.h"

namespace sc_hls { namespace msg_lib {

    // ========================================================================
    // Reader Protocol Implementation (for msg_in ports and msg_channel readers)
    // ========================================================================

    template <typename Derived, typename Message>
    class msg_reader_protocol : public sc_hls::msg_lib::msg_in_if<Message>,
                               public sc_hls::msg_lib::sim_helper_if<0>,
                               public sc_hls::msg_lib::set_rand_force_if,
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
        // Random stall injection support.
        // local_rand_ is null when stall injection is not configured.
        // force_zero_ is false when not stalling — the hot-path check is a single
        // null-pointer test that the branch predictor eliminates after a few cycles.
        std::shared_ptr<rand_force_if> local_rand_;
        bool force_zero_ = false;

        // Port-access mode (see enable_port_access()). When true, the endpoint
        // is NOT registered with the channel_manager (so its pre()/post() are
        // never scheduled) and the user process drives rdy/vld/dat directly.
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
                     ">: port access is enabled; drive rdy/vld/dat directly.").c_str());
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
        // Opt in to driving the rdy/vld/dat pins directly from the user process
        // instead of using pop()/pop_nb()/peek_nb(). Must be called during
        // elaboration (i.e. in the module constructor). In this mode the reset
        // functions (reset_pop) are forbidden; the user process drives the pins,
        // including their idle values, and the endpoint is never registered with
        // the channel_manager.
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
        // set_rand_force_if Implementation
        // ====================================================================
        // Random stall injection is a reader-side feature that delays data
        // acceptance by forcing rdy=0 or delaying buffer acceptance.

        void set_rand_force(std::shared_ptr<rand_force_if> rif) override {
            local_rand_ = rif;
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
        // When stalling (force_zero_=true), data is not accepted even if valid.
        virtual void pre() override
        {
            if (!in_buf_vld && derived().vld.read() && !force_zero_)
            {
                in_buf_vld = true;
                in_buf_dat = derived().dat.read();
            }
        }

        // post: Called after delta cycle - drives ready signal.
        // Queries the stall generator each cycle (null check is free when not configured).
        virtual void post() override
        {
            if (local_rand_) {
                force_zero_ = local_rand_->force_zero();
            }

            // When stalling (force_zero_=true), force rdy=0
            if (!in_buf_vld && !force_zero_) {
                derived().rdy.write(true);
            } else {
                derived().rdy.write(false);
            }
        }

        // reset_pop: Reset the reader state and register with channel_manager
        virtual void reset_pop() override
        {
            ensure_started();
            sc_hls::msg_lib::logger<Message>::port_name_ = derived().name();

            if (port_access_) {
                // Port access is enabled: reset is forbidden. The user process
                // drives rdy/vld/dat directly (including idle values).
                SC_REPORT_ERROR("SC_HLS",
                    (std::string("reset_pop() is forbidden on port <") +
                     std::string(derived().name()) +
                     ">: port access is enabled; do not call reset (drive rdy/vld/dat directly).").c_str());
                return;
            }

            get_chanMgr().add_reader(this);
            in_buf_vld = false;

            // Reset stall state
            force_zero_ = false;
            if (local_rand_) {
                local_rand_->reset();
            }
        }
    };

    // ========================================================================
    // Writer Protocol Implementation (for msg_out ports and msg_channel writers)
    // ========================================================================

    template <typename Derived, typename Message>
    class msg_writer_protocol : public sc_hls::msg_lib::msg_out_if<Message>,
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
        // by 'latency_' clock cycles before appearing on the output signals
        // (vld/dat). This models HLS pipeline latency.
        //
        // When not annotated (annotated_=false, the default), the hot path is
        // a single bool check that the branch predictor eliminates after a few
        // cycles.
        //
        // Implementation uses a circular buffer combined with a cycle counter:
        //
        //   delay_buffer_: Circular buffer of size 'capacity_'.
        //                  Preserves FIFO ordering of items in flight.
        //   write_idx_:    Next position to write (advances on push).
        //   read_idx_:     Next position to read (advances on output).
        //   items_in_buffer_: Count of items currently in buffer.
        //   current_cycle_:   Global cycle counter, incremented each pre().
        //   slot.output_cycle: The cycle when that item should be output.
        //
        // Example with latency=20, capacity=5, II=4:
        //   Cycle 0:  push(A), A.output_cycle=20, items=1
        //   Cycle 4:  push(B), B.output_cycle=24, items=2
        //   Cycle 8:  push(C), C.output_cycle=28, items=3
        //   Cycle 12: push(D), D.output_cycle=32, items=4
        //   Cycle 16: push(E), E.output_cycle=36, items=5 (buffer full)
        //   Cycle 20: current_cycle >= A.output_cycle → Output A, items=4
        //             push(F), F.output_cycle=40, items=5
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
        // never scheduled) and the user process drives vld/dat directly (and
        // reads rdy). push()/push_nb() are forbidden in this mode.
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
        // (and reading rdy) instead of using push()/push_nb(). Must be called
        // during elaboration (i.e. in the module constructor). In this mode the
        // reset functions (reset_push) are forbidden; the user process drives
        // the pins, including their idle values, and the endpoint is never
        // registered with the channel_manager (so multiple-writer detection is
        // also skipped — the user owns the pins).
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

        // pre: Called before delta cycle - samples ready signal and manages buffer
        virtual void pre() override
        {
            if (annotated_) {
                // ----------------------------------------------------------------
                // Annotated mode: Circular buffer management.
                //
                // Step 1: Increment cycle counter (tracks simulation time in cycles)
                // Step 2: Check if current output was consumed (vld/rdy handshake)
                // Step 3: If output free, check if head item is ready to output
                // ----------------------------------------------------------------

                // Step 1: Increment cycle counter
                ++current_cycle_;

                // Step 2: Check if current output was consumed (vld/rdy handshake)
                // rdy=1 means consumer accepted the data this cycle
                if (out_buf_vld && derived().rdy.read()) {
                    out_buf_vld = false;
                }

                // Step 3: If output is free, check if head item is ready
                if (!out_buf_vld && items_in_buffer_ > 0) {
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
            // Check vld/rdy handshake to clear output buffer.
            // ----------------------------------------------------------------
            if (derived().vld.read() && derived().rdy.read()) {
                out_buf_vld = false;
            }
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
