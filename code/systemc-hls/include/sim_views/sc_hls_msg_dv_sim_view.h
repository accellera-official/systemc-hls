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
sim_views/sc_hls_msg_dv_sim_view.h

Message Passing without Back-Pressure (msg_dv) — Simulation View Implementations

This file holds the simulation-view class bodies for the msg_dv protocol, separated
from the public primary template declarations in sc_hls_msg_dv.h so that none of
this (nor the pre/post scheduling infrastructure it depends on) is visible to
the HLS tool under __SYNTHESIS__.

Contents:
- msg_dv_in / msg_dv_out / msg_dv_channel<Message, sim_view_tag> — cycle-accurate,
  signal-based msg_dv (valid-only) ports/channel using the CRTP protocol engines in
  internal/sc_hls_msg_dv_protocol.h.

Included only from the #ifndef __SYNTHESIS__ block of the umbrella <systemc-hls>.

Version: 4.1 - Sticky reader buffer + dv_overrun_policy
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"
#include "sc_hls_interfaces.h"
#include "sc_hls_rand_stall.h"
#include "sc_hls_msg_dv.h"
#include "internal/sc_hls_msg_dv_protocol.h"

namespace sc_hls { namespace msg_lib {

// Protocol implementations (msg_dv_reader_protocol / msg_dv_writer_protocol)
// live in internal/sc_hls_msg_dv_protocol.h, already in this msg_lib namespace.

// ============================================================================
// SimView Specializations
// ============================================================================

// ========================================================================
// msg_dv_in<Message, sim_view_tag> - Input Port
// ========================================================================

template <typename Message>
class msg_dv_in<Message, sim_view_tag>
    : public sc_module,
      public msg_dv_reader_protocol<msg_dv_in<Message, sim_view_tag>, Message>
{
public:
    using message_type = Message;

    // Signal-level interface (no ready signal):
    sc_in<bool>     SC_NAMED(vld);
    sc_in<Message>  SC_NAMED(dat);

    // Constructor:
    msg_dv_in(const sc_module_name &n = sc_gen_unique_name("msg_dv_in")) : sc_module(n) {}

    // Bindings:
    template <typename C>
    void bind(C &rhs)
    {
        vld(rhs.vld);
        dat(rhs.dat);
    }

    template <typename C>
    void operator()(C &rhs) { bind(rhs); }
};

// ========================================================================
// msg_dv_out<Message, sim_view_tag> - Output Port
// ========================================================================

template <typename Message>
class msg_dv_out<Message, sim_view_tag>
    : public sc_module,
      public msg_dv_writer_protocol<msg_dv_out<Message, sim_view_tag>, Message>
{
public:
    typedef msg_dv_writer_protocol<msg_dv_out<Message, sim_view_tag>, Message> base_writer_type;
    using message_type = Message;

    // Signal-level interface (no ready signal):
    sc_out<bool>    SC_NAMED(vld);
    sc_out<Message> SC_NAMED(dat);

    // Constructor:
    msg_dv_out(const sc_module_name &n = sc_gen_unique_name("msg_dv_out")) : sc_module(n) {}

    // Bindings:
    template <typename C>
    void bind(C &rhs)
    {
        vld(rhs.vld);
        dat(rhs.dat);
    }

    template <typename C>
    void operator()(C &rhs) { bind(rhs); }

    virtual void reset_push() override {
        this->base_writer_type::reset_push();
        this->base_writer_type::driven_signal = vld.get_interface();
    }
    // back_annotation_if is inherited from msg_dv_writer_protocol - no delegation needed
};

// ========================================================================
// msg_dv_channel<Message, sim_view_tag> - Channel Implementation
// ========================================================================

template <typename Message>
class msg_dv_channel<Message, sim_view_tag>
    : public sc_channel,
      public msg_dv_reader_protocol<msg_dv_channel<Message, sim_view_tag>, Message>,
      public msg_dv_writer_protocol<msg_dv_channel<Message, sim_view_tag>, Message>,
      public trace_marker_if
{
public:
    typedef msg_dv_writer_protocol<msg_dv_channel<Message, sim_view_tag>, Message> base_writer_type;
    using message_type = Message;

    sc_signal<bool>     SC_NAMED(vld);
    sc_signal<Message>  SC_NAMED(dat);

    msg_dv_channel(const sc_module_name &nm) : sc_channel(nm) {}

    // Port-access mode for portless msg_dv channels. Because msg_dv_channel inherits
    // both reader and writer protocols, each defines enable_port_access(); these
    // disambiguated wrappers select the side. Enable a side to drive that side's
    // pins directly (writer: vld/dat; reader has no rdy pin) instead of using
    // push()/pop(). Must be called before the corresponding
    // reset_push()/reset_pop().
    void enable_port_access_writer() {
        msg_dv_writer_protocol<msg_dv_channel<Message, sim_view_tag>, Message>::enable_port_access();
    }
    void enable_port_access_reader() {
        msg_dv_reader_protocol<msg_dv_channel<Message, sim_view_tag>, Message>::enable_port_access();
    }

    virtual void reset_push() override {
        this->base_writer_type::reset_push();
        this->base_writer_type::driven_signal = static_cast<sc_interface*>(&vld);
    }

    // trace_marker_if implementation
    virtual void set_trace(sc_trace_file *tf) override {
        sc_trace(tf, vld, std::string(name()) + ".vld");
        sc_trace(tf, dat, std::string(name()) + ".dat");
    }

    // back_annotation_if is inherited from msg_dv_writer_protocol - no delegation needed
    // Per-port dv_overrun_policy API is inherited from msg_dv_reader_protocol.
};

} } // namespace sc_hls::msg_lib
