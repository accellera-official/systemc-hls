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
 * FIFO Example (msg_fifo_module)
 *
 * Demonstrates sc_hls::msg_lib::msg_fifo_module<T, N> used directly: a producer
 * module and a consumer module are wired to opposite sides of a FIFO through
 * two msg_channels. The FIFO buffers up to N elements, decoupling the two
 * modules while the vld/rdy back-pressure path keeps overflow / underflow safe.
 *
 * The FIFO's enq/deq are ports driven internally by the FIFO; they are bound
 * here to the channels fifo_in / fifo_out, and the producer/consumer modules
 * operate on those channels. (For a wrapper that presents the FIFO as a pair
 * of ready-to-use channels directly, see example 16_fifo_channel.)
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

// Producer: reads from its input port and pushes into the FIFO channel.
SC_MODULE(producer) {
    using T = sc_uint<32>;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<T> SC_NAMED(in1);
    msg_out<T> SC_NAMED(to_fifo);

    SC_CTOR(producer) {
        SC_THREAD(main1);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        to_fifo.reset_push();
        wait();
    }

    void main1() {
        reset();
        while (1) {
            to_fifo.push(in1.pop());
        }
    }
};

// Consumer: pops from the FIFO channel and writes to its output port.
SC_MODULE(consumer) {
    using T = sc_uint<32>;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<T> SC_NAMED(from_fifo);
    msg_out<T> SC_NAMED(out1);

    SC_CTOR(consumer) {
        SC_THREAD(main2);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        from_fifo.reset_pop();
        out1.reset_push();
        wait();
    }

    void main2() {
        reset();
        while (1) {
            out1.push(from_fifo.pop());
        }
    }
};

SC_MODULE(dut) {
    using T = sc_uint<32>;
    static constexpr int N = 8;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<T> SC_NAMED(in1);
    msg_out<T> SC_NAMED(out1);

    sc_hls::msg_lib::msg_fifo_module<T, N> SC_NAMED(fifo1);
    msg_channel<T> SC_NAMED(fifo_in);
    msg_channel<T> SC_NAMED(fifo_out);

    producer SC_NAMED(prod);
    consumer SC_NAMED(cons);

    SC_CTOR(dut) {
        fifo1.clk(clk);
        fifo1.rst(rst_bar);
        fifo1.enq(fifo_in);
        fifo1.deq(fifo_out);

        prod.clk(clk);
        prod.rst_bar(rst_bar);
        prod.in1(in1);
        prod.to_fifo(fifo_in);

        cons.clk(clk);
        cons.rst_bar(rst_bar);
        cons.from_fifo(fifo_out);
        cons.out1(out1);
    }
};
