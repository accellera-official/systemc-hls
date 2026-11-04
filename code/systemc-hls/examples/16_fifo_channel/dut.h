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
 * FIFO Channel Example (msg_fifo_channel)
 *
 * Demonstrates sc_hls::msg_lib::msg_fifo_channel<T, N>: a FIFO wrapped as a
 * pair of ready-to-use channels (in / out) that let two processes communicate
 * directly through the buffered channel. One thread reads the module's input
 * and pushes into fifo_chan.in; the other pops from fifo_chan.out and writes
 * the module's output. Neither thread touches the FIFO's raw enq/deq ports —
 * the wrapper binds those internally.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    using T = sc_uint<32>;
    static constexpr int N = 8;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<T> SC_NAMED(in1);
    msg_out<T> SC_NAMED(out1);

    // FIFO wrapped as in/out channels; the two threads communicate through it.
    msg_fifo_channel<T, N> SC_NAMED(fifo_chan);

    SC_CTOR(dut) {
        fifo_chan.clk(clk);
        fifo_chan.rst(rst_bar);

        SC_THREAD(producer);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(consumer);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset_producer() {
        in1.reset_pop();
        fifo_chan.in.reset_push();
        wait();
    }

    void reset_consumer() {
        fifo_chan.out.reset_pop();
        out1.reset_push();
        wait();
    }

    void producer() {
        reset_producer();
        while (1) {
            fifo_chan.in.push(in1.pop());
        }
    }

    void consumer() {
        reset_consumer();
        while (1) {
            out1.push(fifo_chan.out.pop());
        }
    }
};
