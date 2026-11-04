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
sc_hls_manager.h

Clock and Channel Management

This file contains the clock and channel management infrastructure for the
sc_hls library, implementing the centralized coordination mechanism that
enables throughput-accurate simulation.

Key Components:

1. clock_manager: Manages the global simulation clock and timing
   - Stores references to all design clocks (sc_clock*)
   - Defines epsilon timing for pre/post scheduling
   - Initialized via sc_hls::msg_lib::add_clk(sc_clock*)

2. channel_manager: Tracks and coordinates all channels during simulation
   - Maintains separate lists of reader and writer endpoints, indexed by clock domain
   - Implements the pre/post scheduling protocol
   - Coordinates timing across all msg and msg_dv ports/channels

Library Internals - pre/post Scheduling Protocol:

The channel_manager implements a two-phase protocol executed each clock cycle:

Phase 1: pre() - Sample Phase (before clock edge)
   - Called on all tracked endpoints (writers first, then readers)
   - Writers sample 'rdy' signals (msg only) and update internal state
   - Readers sample 'vld' and 'dat' signals into internal buffers
   - Timing: executed at (clock_edge - 2*epsilon)

Phase 2: post() - Drive Phase (after clock edge)
   - Called on all tracked endpoints (writers first, then readers)
   - Writers drive 'vld' and 'dat' signals based on internal buffers
   - Readers drive 'rdy' signals (msg only) based on buffer availability
   - Timing: executed at (clock_edge + epsilon)

This pre/post protocol ensures:
- Consistent delta-cycle ordering across all channels
- Throughput-accurate timing that matches synthesized RTL behavior
- Proper handshake coordination for both the msg and msg_dv protocols

Endpoint Registration:

Each port or channel endpoint registers itself with channel_manager by calling
reset_pop() or reset_push() during reset. The registration uses a compile-time
IS_WRITER flag to place the endpoint in the appropriate tracking list.
The calling process handle is recorded as the owning process, enforcing that
all subsequent API calls come from the same process.

Note: The Sync protocol (sync_channel, sync_in, sync_out) does not participate
in this pre/post scheduling. It implements rendezvous synchronization directly
using SystemC primitives.

Annotation validation is always compiled in. When no annotations are configured,
validate_annotations() returns immediately on an empty table — negligible overhead.

Version: 3.0
*/

#pragma once

// SC_INCLUDE_DYNAMIC_PROCESSES must be defined before the first systemc.h include
// to enable sc_spawn(). If it is not yet defined, define it now and re-include
// the reentrant systemc header. If it is already defined, nothing extra is needed.
#include <systemc.h>
#if !defined(SC_INCLUDE_DYNAMIC_PROCESSES)
  #define SC_INCLUDE_DYNAMIC_PROCESSES
  #include <systemc>
#endif

#include <vector>
#include <map>
#include <sstream>
#include "sc_hls_interfaces.h"
#include "sc_hls_sim_helper.h"
#include "sc_hls_synth_util.h"   // SC_HLS_SYNTH_NAME (shared with the FIFO headers)

// Macro definitions
#ifndef SC_HLS_ASSERT_MSG
  #ifndef __SYNTHESIS__
    #define SC_HLS_ASSERT_MSG(cond, msg) \
      do { if (!(cond)) { SC_REPORT_ERROR("SC_HLS", msg); } } while(0)
  #else
    #define SC_HLS_ASSERT_MSG(cond, msg)
  #endif
#endif

namespace sc_hls { namespace msg_lib {

    // Forward declarations of public API
    inline void add_clk(sc_clock *clk_ptr);
    inline void add_clock_alias(sc_clock &real_clk,
                                const sc_core::sc_signal_in_if<bool> &alias_sig);
    inline void start();

    // ========================================================================
    // annotation_entry
    // ========================================================================
    // Tracks annotation state for validation at end of simulation.
    // Stored in channel_manager::annotation_table keyed by port/channel name.
    struct annotation_entry {
        int latency = 0;
        int capacity = 0;
        bool was_reset = false;
    };

    // ========================================================================
    // clock_manager
    // ========================================================================

    class clock_manager
    {
    public:
        clock_manager() {}

        void set(sc_clock *clk_ptr_)
        {
            auto it = std::find(clock_vector.begin(), clock_vector.end(), clk_ptr_);
            if (it == clock_vector.end()) {
                clock_vector.push_back(clk_ptr_);
            }
        }

        void start_of_simulation()
        {
            epsilon = sc_time(0.01, SC_NS);

            if (clock_vector.empty()) {
                auto_discover_clocks();
            }

            SC_REPORT_INFO("SC_HLS", "sc_hls library started");

            for (auto* clk : clock_vector) {
                SC_REPORT_INFO("SC_HLS",
                    (std::string("Using clock: ") + clk->name() +
                     " Period: " + clk->period().to_string()).c_str());
            }

            if (clock_vector.empty()) {
                SC_REPORT_ERROR("SC_HLS",
                    "No clocks found - ensure at least one sc_clock exists in design");
            }
        }

        sc_core::sc_time epsilon;

        // Vector of all discovered/registered clocks.
        // Index i is used as the clock domain identifier throughout the system.
        // This index is stored in sim_helper_if::clock_number for each port,
        // and is used to index into tracked_rd[i] and tracked_wr[i] in channel_manager.
        std::vector<sc_clock*> clock_vector;

        // Clock aliasing.
        // A design whose clock is not an sc_clock (e.g. a hand-toggled
        // sc_signal<bool>) has no discoverable clock. add_clock_alias() lets the
        // user pair such a signal's posedge event with a companion sc_clock's
        // posedge event. During start(), the alias event is mapped to the same
        // clock domain as the companion clock, so processes statically sensitive
        // to the toggled signal resolve into that domain.
        //
        // Note: the pre/post scheduler still free-runs on the companion clock's
        // period/phase (see run_for_clock) — it is NOT synchronized to the
        // toggled signal's edges. Cycle accuracy requires the companion clock's
        // period and phase to match the toggled signal.
        struct clock_alias_info {
            const sc_event* clock_event;   // real sc_clock posedge
            const sc_event* alias_event;   // toggled-signal posedge
        };
        std::vector<clock_alias_info> clock_alias_vector;

        void add_clock_alias(const sc_event& clock_event, const sc_event& alias_event)
        {
            clock_alias_vector.push_back({ &clock_event, &alias_event });
        }

        void find_clocks(sc_object *obj)
        {
            sc_clock *clk = dynamic_cast<sc_clock *>(obj);
            if (clk) {
                auto it = std::find(clock_vector.begin(), clock_vector.end(), clk);
                if (it == clock_vector.end()) {
                    clock_vector.push_back(clk);
                    SC_REPORT_INFO("SC_HLS",
                        (std::string("Auto-discovered clock: ") + clk->name() +
                         " Period: " + clk->period().to_string()).c_str());
                }
            }

            std::vector<sc_object *> children = obj->get_child_objects();
            for (unsigned i = 0; i < children.size(); i++) {
                if (children[i]) {
                    find_clocks(children[i]);
                }
            }
        }

        void auto_discover_clocks()
        {
            const std::vector<sc_object *> tops = sc_get_top_level_objects();
            for (unsigned i = 0; i < tops.size(); i++) {
                if (tops[i]) {
                    find_clocks(tops[i]);
                }
            }
        }
    };

    // Global accessor functions
    clock_manager &get_clkMgr();

    // ========================================================================
    // channel_manager
    // ========================================================================

    class channel_manager
#if IEEE_1666_SYSTEMC > 201101L
      : public sc_core::sc_stage_callback_if
#else
      : public sc_module
#endif
    {
    public:
        // Per-clock tracking lists for multi-clock support.
        // Outer index i corresponds to clock_vector[i] in clock_manager.
        // tracked_rd[i] contains all reader ports in clock domain i.
        // tracked_wr[i] contains all writer ports in clock domain i.
        std::vector<std::vector<sim_helper_if<0>*>> tracked_rd;
        std::vector<std::vector<sim_helper_if<1>*>> tracked_wr;

        std::map<void*, int> map_port_to_clock;

        // Event-to-clock mapping (value is clock # + 1, so 0 indicates not found)
        std::map<const sc_event*, int> map_event_to_clock;

        bool started = false;

        // Annotation tracking table: maps port/channel name to annotation state.
        // Populated by register_annotation() when set_latency_capacity() is called.
        // Updated by mark_annotation_reset() when reset_push() is called.
        // Validated by validate_annotations() at end of simulation.
        // Empty when no annotations are configured — validate_annotations() returns
        // immediately in that case.
        std::map<std::string, annotation_entry> annotation_table;

    public:
#if IEEE_1666_SYSTEMC > 201101L
        channel_manager() {}
#else
        channel_manager(const sc_module_name &name = sc_gen_unique_name("channel_manager"))
            : sc_module(name) {}
#endif

        template<unsigned IS_WRITER>
        void add_tracked(sim_helper_if<IS_WRITER>* ptr, std::vector<sim_helper_if<IS_WRITER>*>& tracked_vec)
        {
            auto it = std::find(tracked_vec.begin(), tracked_vec.end(), ptr);
            if (it == tracked_vec.end()) {
                tracked_vec.push_back(ptr);
            }
        }

        // Retrieve the calling process's static sensitivity event via a
        // friend-bypassing accessor subclass, so add() can resolve which
        // clock domain a port belongs to.
        static const sc_event* get_first_static_event(sc_process_b* b)
        {
            class process_accessor : public sc_process_b {
            public:
                const sc_event* get_first_static_event() {
                    if (this->m_static_events.size() != 1) {
                        return nullptr;
                    }
                    return this->m_static_events[0];
                }
            };
            process_accessor* pa = static_cast<process_accessor*>(b);
            return pa->get_first_static_event();
        }

        // Add port to tracking list with automatic clock domain resolution.
        // Determines which clock domain the port belongs to based on the calling
        // process's static sensitivity event, then registers the port in the
        // appropriate per-clock tracking list (tracked_rd[clk_idx] or tracked_wr[clk_idx]).
        // Records the calling process as the owning process for single-process enforcement.

        template<unsigned IS_WRITER>
        void add(sim_helper_if<IS_WRITER> *ptr, int &found_clk)
        {
            sc_process_handle h = sc_get_current_process_handle();
            sc_object* o = h.get_process_object();
            sc_process_b* b = dynamic_cast<sc_process_b*>(o);

            if (!b) {
                SC_REPORT_ERROR("SC_HLS",
                    "Failed to get process object for clock resolution");
                return;
            }

            const sc_event* e = get_first_static_event(b);

            if (!e) {
                std::ostringstream ss;
                ss << "Process " << h.name() << " does not have exactly 1 static event. "
                   << "Ensure process has static sensitivity to exactly one clock.";
                SC_REPORT_ERROR("SC_HLS", ss.str().c_str());
                return;
            }

            int clk_idx = map_event_to_clock[e];
            if (clk_idx <= 0) {
                std::ostringstream ss;
                ss << "Failed to resolve clock for process " << h.name() << ". "
                   << "Ensure process is sensitive to a registered sc_clock.";
                SC_REPORT_ERROR("SC_HLS", ss.str().c_str());
                return;
            }

            clk_idx--;  // Undo +1 encoding

            map_port_to_clock[ptr] = clk_idx;
            ptr->was_reset = true;
            ptr->clock_number = clk_idx;
            ptr->owning_process = h;
            found_clk = clk_idx;
        }

        // Separate add_writer and add_reader functions required to support C++11.

        void add_writer(sim_helper_if<1> *ptr) {
            int clk_idx;
            this->add<1>(ptr, clk_idx);
            // Check for signal conflicts - multiple writers to same signal.
            // Iterate all clock domains since a conflict can span domains.
            // Skip self (existing == ptr) to allow reset_push() to be called multiple times.
            sc_process_handle h = sc_get_current_process_handle();
            for (size_t i = 0; i < tracked_wr.size(); i++) {
                for (auto* existing : tracked_wr[i]) {
                    if (existing != ptr && existing->driven_signal == ptr->driven_signal) {
                        std::ostringstream ss;
                        ss << "Multiple writers detected on same signal.\n"
                           << "  First writer registered by process: " << existing->owning_process.name() << "\n"
                           << "  Second writer registered by process: " << h.name() << "\n"
                           << "  Each signal can only have one writer.";
                        SC_REPORT_ERROR("SC_HLS-102", ss.str().c_str());
                    }
                }
            }
            add_tracked<1>(ptr, tracked_wr[clk_idx]);
        }

        void add_reader(sim_helper_if<0> *ptr) {
            int clk_idx;
            this->add<0>(ptr, clk_idx);
            add_tracked<0>(ptr, tracked_rd[clk_idx]);
        }

        void start()
        {
            if (started) {
                return;
            }

            get_clkMgr().start_of_simulation();

            if (get_clkMgr().clock_vector.empty()) {
                SC_REPORT_ERROR("SC_HLS",
                    "No clocks available. Either call sc_hls::msg_lib::add_clk() or ensure sc_clock exists in design.");
                exit(-1);
            }

            tracked_rd.resize(get_clkMgr().clock_vector.size());
            tracked_wr.resize(get_clkMgr().clock_vector.size());

            for (size_t i = 0; i < get_clkMgr().clock_vector.size(); i++) {
                map_event_to_clock[&(get_clkMgr().clock_vector[i]->posedge_event())] = i + 1;
            }

            // Resolve clock aliases: map each alias event to the same domain as
            // its companion clock. Must run after the real-clock map above is
            // fully built (aliases copy from it).
            for (const auto& a : get_clkMgr().clock_alias_vector) {
                int resolved = map_event_to_clock[a.clock_event];
                if (resolved) {
                    map_event_to_clock[a.alias_event] = resolved;
                } else {
                    SC_REPORT_ERROR("SC_HLS",
                        "Could not resolve alias clock - the referenced sc_clock was not found in the design");
                }
            }

            for (size_t i = 0; i < get_clkMgr().clock_vector.size(); i++) {
                std::string thread_name = "sc_hls_manager_run_clk" + std::to_string(i);
                sc_spawn(sc_bind(&channel_manager::run_for_clock, this, i), thread_name.c_str());
            }

            started = true;

#if IEEE_1666_SYSTEMC > 201101L
            // Register for end-of-simulation callback to validate annotations.
            // SC_PRE_STOP is called once when sc_stop() is invoked.
            // validate_annotations() returns immediately if no annotations were set.
            sc_core::sc_register_stage_callback(*this, sc_core::SC_PRE_STOP);
#endif
        }

        // Called by set_latency_capacity() to register an annotation.
        // Must be called during elaboration (before sc_start()).
        void register_annotation(const std::string& name, int latency, int capacity) {
            annotation_entry& entry = annotation_table[name];
            entry.latency = latency;
            entry.capacity = capacity;
            // Note: was_reset is NOT set here - it's set by mark_annotation_reset()
        }

        // Called by reset_push() to mark that this port/channel is active.
        // Only updates the entry if it exists (i.e., annotation was set).
        void mark_annotation_reset(const std::string& name) {
            auto it = annotation_table.find(name);
            if (it != annotation_table.end()) {
                it->second.was_reset = true;
            }
        }

        // Called at end of simulation to check for inactive annotations.
        // Reports an error for each annotation that was set but never activated
        // (i.e., set_latency_capacity() was called but reset_push() was not).
        // Returns immediately when no annotations were configured.
        void validate_annotations() {
            if (annotation_table.empty()) {
                return;
            }

            for (const auto& kv : annotation_table) {
                const std::string& name = kv.first;
                const annotation_entry& entry = kv.second;
                if (entry.latency > 0 && !entry.was_reset) {
                    std::ostringstream ss;
                    ss << "Annotation on inactive port/channel: " << name
                       << " (latency=" << entry.latency
                       << ", capacity=" << entry.capacity << ")\n"
                       << "  Annotations must be applied to ports that call reset_push(),\n"
                       << "  not to channels they are bound to.\n"
                       << "  Hint: If '" << name << "' is a msg_channel, annotate the\n"
                       << "  msg_out port that is bound to it instead.";
                    SC_REPORT_ERROR("SC_HLS", ss.str().c_str());
                }
            }
        }

#if IEEE_1666_SYSTEMC > 201101L
        // sc_stage_callback_if implementation.
        // Called by SystemC at simulation phase transitions (SC_PRE_STOP).
        void stage_callback(const sc_core::sc_stage& stage) override {
            if (stage == sc_core::SC_PRE_STOP) {
                validate_annotations();
            }
        }
#else
        // SystemC 2.3.4 does not support stage callbacks; use a
        // module and end-of-simulation callbacks.
        virtual void end_of_simulation() {
            validate_annotations();
        }
#endif

        void run_for_clock(size_t clock_index)
        {
            sc_clock* clk = get_clkMgr().clock_vector[clock_index];
            sc_time epsilon = get_clkMgr().epsilon;
            sc_time period = clk->period();
            sc_time post2pre_delay = period - (2 * epsilon);
            sc_time pre2post_delay = (2 * epsilon);

            wait(epsilon);

            while (1)
            {
                // post() - Drive outputs after clock edge
                for (auto& it : tracked_wr[clock_index]) {
                    it->post();
                }
                for (auto& it : tracked_rd[clock_index]) {
                    it->post();
                }

                wait(post2pre_delay);

                // pre() - Sample inputs before clock edge
                for (auto& it : tracked_wr[clock_index]) {
                    it->pre();
                }
                for (auto& it : tracked_rd[clock_index]) {
                    it->pre();
                }

                wait(pre2post_delay);
            }
        }
    };


    // ========================================================================
    // Global singleton accessors
    // ========================================================================

    template <class Dummy>
    struct channel_manager_statics
    {
        static clock_manager clkMgr;
        static channel_manager chanMgr;
    };

    template <class Dummy>
    clock_manager channel_manager_statics<Dummy>::clkMgr;

    template <class Dummy>
    channel_manager channel_manager_statics<Dummy>::chanMgr;

    inline clock_manager &get_clkMgr()
    {
        return channel_manager_statics<void>::clkMgr;
    }

    inline channel_manager &get_chanMgr()
    {
        return channel_manager_statics<void>::chanMgr;
    }

    inline void add_clk(sc_clock *clk_ptr)
    {
        channel_manager_statics<void>::clkMgr.set(clk_ptr);
    }

    // Alias a non-sc_clock clock (e.g. a hand-toggled sc_signal<bool>) to a
    // companion sc_clock. Processes statically sensitive to alias_sig's posedge
    // resolve into real_clk's timing domain. real_clk must have a period and
    // phase matching alias_sig (not validated — see clock_manager notes).
    inline void add_clock_alias(sc_clock &real_clk,
                                const sc_core::sc_signal_in_if<bool> &alias_sig)
    {
#ifndef __SYNTHESIS__
        channel_manager_statics<void>::clkMgr.add_clock_alias(
            real_clk.posedge_event(), alias_sig.posedge_event());
#endif
    }

    inline void start() {
        channel_manager_statics<void>::chanMgr.start();
    }

    inline void ensure_started() {
        if (!channel_manager_statics<void>::chanMgr.started) {
            channel_manager_statics<void>::chanMgr.start();
        }
    }

} } // namespace sc_hls::msg_lib
