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
 * Nonblocking Example - pop_nb() and push_nb() Methods
 *
 * This example demonstrates the use of nonblocking pop and push operations
 * in the msg protocol. Unlike blocking pop()/push() which wait until data
 * is available/accepted, pop_nb()/push_nb() return immediately with a bool
 * indicating success or failure.
 *
 * Key features:
 * - pop_nb(m): Returns bool - true if data was available (written into m)
 * - push_nb(m): Returns bool - true if data was accepted, false if not ready
 * - Allows polling multiple ports in a single cycle
 * - Useful for implementing priority arbitration or opportunistic transfers
 *
 * This design polls two input ports and forwards data from whichever port
 * has data available, demonstrating how nonblocking operations enable
 * flexible multi-port handling within a single thread.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<32>> SC_NAMED(in1);
    msg_in<sc_uint<32>> SC_NAMED(in2);
    msg_out<sc_uint<32>> SC_NAMED(out1);
    msg_out<sc_uint<32>> SC_NAMED(out2);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        in2.reset_pop();
        out1.reset_push();
        out2.reset_push();
        wait();
    }

    void main() {
        reset();
        while (1) {
            typename decltype(in1)::message_type v1;
            typename decltype(in1)::message_type v2;
            bool got1 = false;
            bool got2 = false;
            do {
                got1 = in1.pop_nb(v1);
                got2 = in2.pop_nb(v2);
                if (got1 || got2) {
                    break;
                }
                wait();
            } while (true);

            if (got1)
                out1.push(v1);
            if (got2)
                out2.push(v2);
        }
    }
};
