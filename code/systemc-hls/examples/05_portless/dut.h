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
 * Portless Channel Example - Internal msg_channel
 *
 * This example demonstrates the use of portless (internal) channels for
 * communication between threads within a single module. Unlike ports that
 * connect across module boundaries, portless channels are declared as
 * member variables and used for intra-module communication.
 *
 * Key features:
 * - msg_channel<T>: Internal channel (not a port)
 * - Two threads within the same module communicate via the internal channel
 * - No port binding required - channel is directly accessible
 * - Useful for pipeline stages within a single module
 *
 * Architecture:
 *   in1 → [thread1] → chan1 → [thread2] → out1
 *
 * This pattern is common in pipelined designs where multiple processing
 * stages exist within a single module boundary.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<uint32_t> SC_NAMED(in1);
    msg_out<uint32_t> SC_NAMED(out1);

    // internal portless channel:
    msg_channel<uint32_t> SC_NAMED(chan1);

    SC_CTOR(dut) {
        SC_THREAD(main1);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(main2);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset_main1() {
        in1.reset_pop();
        chan1.reset_push();
        wait();
    }

    void main1() {
        reset_main1();
        SC_HLS_LOGP("out of reset");
        while (1) {
            auto v = in1.pop();
            chan1.push(v);
        }
    }

    void reset_main2() {
        chan1.reset_pop();
        out1.reset_push();
        wait();
    }

    void main2() {
        reset_main2();
        SC_HLS_LOGP("out of reset");
        while (1) {
            auto v = chan1.pop();
            out1.push(v);
        }
    }
};