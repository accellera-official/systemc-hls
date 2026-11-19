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
sc_hls_fifo_module.h

FIFO Module Implementation

This file contains FIFO implementations for message passing with flow control (msg):
- msg_fifo_module<Message, Depth, ViewTag>: General FIFO with configurable depth
- msg_fifo_module<Message, 1, ViewTag>: Depth-1 specialization for II=1 optimization

The FIFO provides buffering between producer and consumer, allowing them
to operate at different rates while maintaining throughput.

The FIFO exposes its internal-facing endpoints as msg_in<Message> enq and
msg_out<Message> deq. These are driven internally by the FIFO: the general
(Depth>1) implementation drives their vld/rdy/dat pins via its own SC_METHODs,
and the Depth-1 specialization drives them by calling enq.pop()/deq.push()
internally. They are meant to be bound to channels (see examples/04_fifo), not
operated on directly by user code. For a wrapper that presents the FIFO as a
pair of ready-to-use channels, see msg_fifo_channel in sc_hls_fifo_channel.h.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"
#include "sc_hls_msg.h"
#include "sc_hls_interfaces.h"
#include "internal/sc_hls_synth_util.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// msg_fifo_element - Internal storage element for FIFO
// ============================================================================

template <typename Message> class msg_fifo_element {
public:
    sc_signal<Message> dat;
    const Message init_val;

    msg_fifo_element() : dat(sc_gen_unique_name("dat")) {}

    msg_fifo_element(sc_module_name n) : dat(SC_HLS_SYNTH_NAME(n, "dat")) {}

    void reset_state() {
        dat.write(init_val);
    }
};

// ============================================================================
// msg_fifo_module - General FIFO Implementation
// ============================================================================

/// msg_fifo_module - Buffered msg channel with configurable depth
/// @tparam Message  Data type
/// @tparam Depth    FIFO depth (number of entries)
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, unsigned int Depth, typename ViewTag = default_view_tag>
class msg_fifo_module : public sc_module {
    SC_HAS_PROCESS(msg_fifo_module);

public:
    // Interface - ViewTag is passed through to internal ports
    sc_in_clk clk;
    sc_in<bool> rst;
    msg_in<Message, ViewTag> enq;
    msg_out<Message, ViewTag> deq;

    msg_fifo_module(sc_module_name n = sc_gen_unique_name("msg_fifo"))
        : sc_module(n),
          clk("clk"),
          rst("rst"),
          enq(SC_HLS_SYNTH_NAME(n, "enq")),
          deq(SC_HLS_SYNTH_NAME(n, "deq")) {
        Init();
    }

protected:
    // Calculate address width needed for Depth entries
    static constexpr unsigned bits_needed(unsigned x) {
        return x == 0 ? 0 : 1 + bits_needed(x >> 1);
    }
    static constexpr int AddrWidth = bits_needed(Depth);
    using BuffIdx = sc_uint<AddrWidth>;
    using Bit = bool;

    // Internal wires
    sc_signal<Bit> full_next;
    sc_signal<BuffIdx> head_next;
    sc_signal<BuffIdx> tail_next;

    // Internal state
    sc_signal<Bit> full;
    sc_signal<BuffIdx> head;
    sc_signal<BuffIdx> tail;
    msg_fifo_element<Message> buffer[Depth];

    // Helper functions
    void Init() {
        SC_METHOD(EnqRdy);
        sensitive << full;

        SC_METHOD(DeqVal);
        sensitive << full << head << tail;

        SC_METHOD(DeqMsg);
        sensitive << deq.rdy << full << head << tail;

        SC_METHOD(HeadNext);
        sensitive << enq.vld << full << head;

        SC_METHOD(TailNext);
        sensitive << deq.rdy << full << head << tail;

        SC_METHOD(FullNext);
        sensitive << enq.vld << deq.rdy << full << head << tail;

        SC_THREAD(Seq);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        // Initialize tail so DeqMsg always has a valid value
        tail.write(0);
    }

    // Combinational logic

    // Enqueue ready
    void EnqRdy() {
        enq.rdy.write(!full.read());
    }

    // Dequeue valid
    void DeqVal() {
        bool empty = (!full.read() && (head.read() == tail.read()));
        deq.vld.write(!empty);
    }

    // Dequeue message
    void DeqMsg() {
        bool empty = (!full.read() && (head.read() == tail.read()));
        bool do_deq = !empty;
        if (do_deq) {
            deq.dat.write(buffer[tail.read()].dat.read());
        } else {
            deq.dat.write(buffer[0].init_val);
        }
    }

    // Head next calculations
    void HeadNext() {
        bool do_enq = (enq.vld.read() && !full.read());
        BuffIdx head_inc;
        if ((head.read() + 1) == Depth) {
            head_inc = 0;
        } else {
            head_inc = head.read() + 1;
        }

        if (do_enq) {
            head_next.write(head_inc);
        } else {
            head_next.write(head.read());
        }
    }

    // Tail next calculations
    void TailNext() {
        bool empty = (!full.read() && (head.read() == tail.read()));
        bool do_deq = (deq.rdy.read() && !empty);
        BuffIdx tail_inc;
        if ((tail.read() + 1) == Depth) {
            tail_inc = 0;
        } else {
            tail_inc = tail.read() + 1;
        }

        if (do_deq) {
            tail_next.write(tail_inc);
        } else {
            tail_next.write(tail.read());
        }
    }

    // Full next calculations
    void FullNext() {
        bool empty = (!full.read() && (head.read() == tail.read()));
        bool do_enq = (enq.vld.read() && !full.read());
        bool do_deq = (deq.rdy.read() && !empty);

        BuffIdx head_inc;
        if ((head.read() + 1) == Depth) {
            head_inc = 0;
        } else {
            head_inc = head.read() + 1;
        }

        if (do_enq && !do_deq && (head_inc == tail.read())) {
            full_next.write(1);
        } else if (do_deq && full.read()) {
            full_next.write(0);
        } else {
            full_next.write(full.read());
        }
    }

    // Sequential logic
    void Seq() {
        // Reset state
        full.write(0);
        head.write(0);
        tail.write(0);
        for (unsigned int i = 0; i < Depth; ++i) {
            buffer[i].reset_state();
        }

        wait();

        while (1) {
            // Head update
            head.write(head_next);

            // Tail update
            tail.write(tail_next);

            // Full update
            full.write(full_next);

            // Enqueue message
            if (enq.vld.read() && !full.read()) {
                buffer[head.read()].dat.write(enq.dat.read());
            }

            wait();
        }
    }
};

// ============================================================================
// msg_fifo_module Depth-1 Specialization - II=1 Optimization
// ============================================================================

/// msg_fifo_module<Message, 1, ViewTag> - Depth-1 specialization for II=1
/// This specialization provides an optimized implementation for single-entry
/// FIFOs, which simply pass data through with one cycle of latency.
template <typename Message, typename ViewTag>
class msg_fifo_module<Message, 1, ViewTag> : public sc_module {
    SC_HAS_PROCESS(msg_fifo_module);

public:
    sc_in_clk clk;
    sc_in<bool> rst;
    msg_in<Message, ViewTag> enq;
    msg_out<Message, ViewTag> deq;

    msg_fifo_module()
        : sc_module(sc_module_name(sc_gen_unique_name("msg_fifo"))),
          clk("clk"),
          rst("rst"),
          enq(sc_gen_unique_name("enq")),
          deq(sc_gen_unique_name("deq")) {
        Init();
    }

    msg_fifo_module(sc_module_name n)
        : sc_module(n),
          clk("clk"),
          rst("rst"),
          enq(SC_HLS_SYNTH_NAME(n, "enq")),
          deq(SC_HLS_SYNTH_NAME(n, "deq")) {
        Init();
    }

protected:
    void Init() {
        SC_THREAD(Seq);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);
    }

    void Seq() {
        enq.reset_pop();
        deq.reset_push();
        wait();

        while (1) {
            deq.push(enq.pop());
        }
    }
};

} } // namespace sc_hls::msg_lib