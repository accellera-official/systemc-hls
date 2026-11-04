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
sim_views/sc_hls_sync_sim_view.h

Rendezvous Synchronization — Simulation View Implementations

This file holds the simulation-view class bodies for the Sync protocol,
separated from the public primary template declarations in sc_hls_sync.h so
that the class bodies are not visible to the HLS tool under __SYNTHESIS__.

Contents:
- sync_in / sync_out / sync_channel<sim_view_tag> — cycle-accurate rendezvous
  ports/channel. Unlike msg/msg_dv, these do not participate in the pre/post
  scheduling and use SystemC wait()/signal primitives directly.

Included only from the #ifndef __SYNTHESIS__ block of the umbrella <systemc-hls>.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"
#include "sc_hls_interfaces.h"
#include "sc_hls_sync.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// sim_view_tag Specializations
// ============================================================================

// ========================================================================
// sync_channel<sim_view_tag> - Channel Implementation
// ========================================================================

template <>
class sync_channel<sim_view_tag> : public sc_channel, public trace_marker_if {
public:
    sc_signal<bool> SC_NAMED(rdy);
    sc_signal<bool> SC_NAMED(vld);

public:
    sync_channel(const sc_module_name& nm) : sc_channel(nm) {}

    virtual void set_trace(sc_trace_file* tf) override {
        sc_trace(tf, vld, std::string(name()) + ".vld");
        sc_trace(tf, rdy, std::string(name()) + ".rdy");
    }

    // Reader side of the rendezvous: drives rdy, waits for vld.
    template <typename... T> void sync_read(T&... t) {
        do {
            rdy.write(true);
            wait();
        } while (vld.read() != true);
        rdy.write(false);
    }

    // Writer side of the rendezvous: drives vld, waits for rdy.
    template <typename... T> void sync_write(T&... t) {
        do {
            vld.write(true);
            wait();
        } while (rdy.read() != true);
        vld.write(false);
    }

    void reset_pop() {
        rdy = 0;
    }

    void reset_push() {
        vld = 0;
    }
};

// ========================================================================
// sync_in<sim_view_tag> - Input Port
// ========================================================================

template <>
class sync_in<sim_view_tag> : public sc_module {
public:
    sc_in<bool> SC_NAMED(vld);
    sc_out<bool> SC_NAMED(rdy);

public:
    sync_in(const sc_module_name& n = sc_gen_unique_name("sync_in")) : sc_module(n) {}

    template <typename... T> void sync(T & ... t) {
        do {
            rdy.write(true);
            wait();
        } while (vld.read() != true);
        rdy.write(false);
    }

    void reset_pop() {
        rdy = 0;
    }

    template <typename C> void bind(C& rhs) {
        vld(rhs.vld);
        rdy(rhs.rdy);
    }

    template <typename C> void operator()(C& rhs) { bind(rhs); }
};

// ========================================================================
// sync_out<sim_view_tag> - Output Port
// ========================================================================

template <>
class sync_out<sim_view_tag> : public sc_module {
public:
    sc_out<bool> SC_NAMED(vld);
    sc_in<bool> SC_NAMED(rdy);

public:
    sync_out(const sc_module_name& n = sc_gen_unique_name("sync_out")) : sc_module(n) {}

    template <typename... T> void sync(T & ... t) {
        do {
            vld.write(true);
            wait();
        } while (rdy.read() != true);
        vld.write(false);
    }

    void reset_push() {
        vld = 0;
    }

    template <typename C> void bind(C& rhs) {
        vld(rhs.vld);
        rdy(rhs.rdy);
    }

    template <typename C> void operator()(C& rhs) { bind(rhs); }
};

} } // namespace sc_hls::msg_lib
