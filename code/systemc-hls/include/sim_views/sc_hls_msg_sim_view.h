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
sim_views/sc_hls_msg_sim_view.h

Message Passing with Flow Control (msg) — Simulation View Implementations

This file holds the simulation-view class bodies for the msg protocol,
separated from the public primary template declarations in sc_hls_msg.h so that
none of this (nor the pre/post scheduling infrastructure it depends on) is
visible to the HLS tool under __SYNTHESIS__.

Contents:
- msg_in / msg_out / msg_channel<Message, sim_view_tag> — cycle-accurate,
  signal-based msg ports/channel using the CRTP protocol engines in
  internal/sc_hls_msg_protocol.h.
- msg_in / msg_out / msg_channel<Message, tlm_view_tag> — fast TLM-FIFO based
  functional view (simulation-only).

Included only from the #ifndef __SYNTHESIS__ block of the umbrella <systemc-hls>.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#ifndef __SYNTHESIS__
#include <tlm.h>
#endif
#include "sc_hls_view_tag.h"
#include "sc_hls_interfaces.h"
#include "sc_hls_rand_stall.h"
#include "sc_hls_msg.h"
#include "internal/sc_hls_msg_protocol.h"

namespace sc_hls { namespace msg_lib {

// Protocol implementations (msg_reader_protocol / msg_writer_protocol) live in
// internal/sc_hls_msg_protocol.h, already in this same msg_lib namespace.

// ============================================================================
// SimView Specializations
// ============================================================================

// ========================================================================
// msg_in<Message, sim_view_tag> - Input Port
// ========================================================================

template <typename Message>
class msg_in<Message, sim_view_tag>
    : public sc_module,
      public msg_reader_protocol<msg_in<Message, sim_view_tag>, Message>
{
public:
    using message_type = Message;

public:
    // Signal-level interface:
    sc_out<bool>    SC_NAMED(rdy);
    sc_in<bool>     SC_NAMED(vld);
    sc_in<Message>  SC_NAMED(dat);

    // Constructor:
    msg_in(const sc_module_name &n = sc_gen_unique_name("msg_in")) : sc_module(n) {}

    // Bindings:
    template <typename C>
    void bind(C &rhs)
    {
        rdy(rhs.rdy);
        vld(rhs.vld);
        dat(rhs.dat);
    }

    template <typename C>
    void operator()(C &rhs) { bind(rhs); }
};

// ========================================================================
// msg_out<Message, sim_view_tag> - Output Port
// ========================================================================

template <typename Message>
class msg_out<Message, sim_view_tag>
    : public sc_module,
      public msg_writer_protocol<msg_out<Message, sim_view_tag>, Message>
{
public:
    using message_type = Message;
    typedef msg_writer_protocol<msg_out<Message, sim_view_tag>, Message> base_writer_type;

public:
    // Signal-level interface:
    sc_in<bool>     SC_NAMED(rdy);
    sc_out<bool>    SC_NAMED(vld);
    sc_out<Message> SC_NAMED(dat);

    // Constructor:
    msg_out(const sc_module_name &n = sc_gen_unique_name("msg_out")) : sc_module(n) {}

    // Bindings:
    template <typename C>
    void bind(C &rhs)
    {
        rdy(rhs.rdy);
        vld(rhs.vld);
        dat(rhs.dat);
    }

    template <typename C>
    void operator()(C &rhs) { bind(rhs); }

    virtual void reset_push() override {
        this->base_writer_type::reset_push();
        this->base_writer_type::driven_signal = vld.get_interface();
    }
    // back_annotation_if is inherited from msg_writer_protocol - no delegation needed
};

// ========================================================================
// msg_channel<Message, sim_view_tag> - Channel Implementation
// ========================================================================

template <typename Message>
class msg_channel<Message, sim_view_tag>
    : public sc_channel,
      public msg_reader_protocol<msg_channel<Message, sim_view_tag>, Message>,
      public msg_writer_protocol<msg_channel<Message, sim_view_tag>, Message>,
      public trace_marker_if
{
public:
    using message_type = Message;
    typedef msg_writer_protocol<msg_channel<Message, sim_view_tag>, Message> base_writer_type;

public:
    sc_signal<bool>     SC_NAMED(rdy);
    sc_signal<bool>     SC_NAMED(vld);
    sc_signal<Message>  SC_NAMED(dat);

    msg_channel(const sc_module_name &nm) : sc_channel(nm) {}

    // Port-access mode for portless channels. Because msg_channel inherits both
    // reader and writer protocols, each defines enable_port_access(); these
    // disambiguated wrappers select the side. Enable a side to drive that side's
    // pins directly (writer: vld/dat; reader: rdy) instead of using push()/pop().
    // Must be called before the corresponding reset_push()/reset_pop().
    void enable_port_access_writer() {
        msg_writer_protocol<msg_channel<Message, sim_view_tag>, Message>::enable_port_access();
    }
    void enable_port_access_reader() {
        msg_reader_protocol<msg_channel<Message, sim_view_tag>, Message>::enable_port_access();
    }

    // trace_marker_if implementation
    virtual void set_trace(sc_trace_file *tf) override {
        sc_trace(tf, vld, std::string(name()) + ".vld");
        sc_trace(tf, rdy, std::string(name()) + ".rdy");
        sc_trace(tf, dat, std::string(name()) + ".dat");
    }

    virtual void reset_push() override {
        this->base_writer_type::reset_push();
        this->base_writer_type::driven_signal = static_cast<sc_interface*>(&vld);
    }

    // back_annotation_if is inherited from msg_writer_protocol - no delegation needed
    // set_rand_force_if is inherited from msg_reader_protocol - no delegation needed
};

// ============================================================================
// TLM View Specializations (Simulation-Only)
// ============================================================================

#ifndef __SYNTHESIS__

// ========================================================================
// msg_channel<Message, tlm_view_tag> - TLM Channel Implementation
// ========================================================================

template <typename Message>
class msg_channel<Message, tlm_view_tag>
    : public sc_module,
      public msg_in_if<Message>,
      public msg_out_if<Message>,
      public tlm_fifo_depth_if
{
public:
    using message_type = Message;

public:
    tlm::tlm_fifo<Message> fifo;

private:
    unsigned int depth_;

public:
    // Constructor with configurable depth (default: 16)
    msg_channel(const sc_module_name& nm, unsigned int depth = 16)
        : sc_module(nm),
          fifo(sc_gen_unique_name("fifo"), depth),
          depth_(depth) {}

    // ====================================================================
    // msg_in_if<Message> Implementation (Reader Side)
    // ====================================================================

    void reset_pop() override {
        // Drain any items left over from a prior run so the FIFO starts empty.
        Message m;
        while (fifo.nb_get(m)) {}
    }

    Message pop() override {
        return fifo.get();  // Blocking read
    }

    bool pop_nb(Message& m) override {
        return fifo.nb_get(m);  // Non-blocking read
    }

    bool peek_nb(Message& m) override {
        return fifo.nb_peek(m);
    }

    // ====================================================================
    // msg_out_if<Message> Implementation (Writer Side)
    // ====================================================================

    void reset_push() override {
        // TLM FIFOs don't need reset
    }

    void push(const Message& m) override {
        fifo.put(m);  // Blocking write
    }

    bool push_nb(const Message& m) override {
        return fifo.nb_put(m);
    }

    // ====================================================================
    // tlm_fifo_depth_if Implementation
    // ====================================================================

    void set_fifo_depth(unsigned int depth) override {
        depth_ = depth;
        // Note: Actual TLM FIFO depth is set at construction.
        // This method is for annotation file support.
        // Depth changes require reconstruction.
    }

    unsigned int get_fifo_depth() const override {
        return depth_;
    }

    std::string get_fifo_path() const override {
        return std::string(name());
    }
};

// ========================================================================
// msg_in<Message, tlm_view_tag> - TLM Input Port
// ========================================================================

template <typename Message>
class msg_in<Message, tlm_view_tag> : public sc_module {
public:
    using message_type = Message;

public:
    sc_port<tlm::tlm_get_if<Message>> port;

    msg_in(const sc_module_name& n = sc_gen_unique_name("msg_in"))
        : sc_module(n) {}

    // msg_in_if-compatible methods for user code
    Message pop() { return port->get(); }

    bool pop_nb(Message& m) { return port->nb_get(m); }

    bool peek_nb(Message& m) { return port->nb_peek(m); }

    // Drain any items left over from a prior run so the FIFO starts empty.
    void reset_pop() {
        Message m;
        while (port->nb_get(m)) {}
    }

    // Binding to TLM channel
    void bind(msg_channel<Message, tlm_view_tag>& chan) {
        port(chan.fifo);
    }

    // Binding to another TLM port
    template <typename C>
    void bind(C& rhs) {
        port(rhs.port);
    }

    template <typename C>
    void operator()(C& rhs) { bind(rhs); }
};

// ========================================================================
// msg_out<Message, tlm_view_tag> - TLM Output Port
// ========================================================================

template <typename Message>
class msg_out<Message, tlm_view_tag> : public sc_module {
public:
    using message_type = Message;

public:
    sc_port<tlm::tlm_put_if<Message>> port;

    msg_out(const sc_module_name& n = sc_gen_unique_name("msg_out"))
        : sc_module(n) {}

    // msg_out_if-compatible methods for user code
    void push(const Message& m) { port->put(m); }

    bool push_nb(const Message& m) { return port->nb_put(m); }

    void reset_push() { /* No reset needed for TLM */ }

    // Binding to TLM channel
    void bind(msg_channel<Message, tlm_view_tag>& chan) {
        port(chan.fifo);
    }

    // Binding to another TLM port
    template <typename C>
    void bind(C& rhs) {
        port(rhs.port);
    }

    template <typename C>
    void operator()(C& rhs) { bind(rhs); }
};

#endif // __SYNTHESIS__

} } // namespace sc_hls::msg_lib
