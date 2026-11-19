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
 * dat_vld Example - Message Passing without Back-Pressure (msg_dv)
 *
 * This example demonstrates the use of msg_dv_in and msg_dv_out ports, which implement
 * the msg_dv protocol (valid signal only, no ready/back-pressure).
 *
 * The msg_dv protocol is suitable for modeling producer-driven streams where the
 * consumer cannot apply back-pressure to stall the source. This is common in
 * scenarios like sensor outputs or streaming data sources.
 *
 * Key differences from msg_in/msg_out (dat/vld/rdy):
 * - No ready signal from receiver to sender
 * - Sender cannot be stalled by receiver
 * - Simpler protocol, but less flexible flow control
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;


SC_MODULE(dut) {
    using T = unsigned;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_dv_in<T> SC_NAMED(in1);
    msg_dv_out<T> SC_NAMED(out1);

    sc_signal<sc_uint<4>>   SC_NAMED(aux_ctrl);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        out1.reset_push();
        in1.reset_pop();
        aux_ctrl = 0;
        wait();
    }

    void main() {
        reset();
        while (1) {
            // pipeline this ii=1
            for (unsigned i=0; i<30; i++) {
                auto x = in1.pop();
                out1.push(x);
            }

            // pipeline this ii=4, can miss some
            for (unsigned i=30; i<40; i++) {
                aux_ctrl = 1;
                auto x = in1.pop();
                aux_ctrl = 2;
                wait();
                aux_ctrl = 3;
                wait();
                aux_ctrl = 4;
                wait();
                aux_ctrl = 5;
                wait();
                aux_ctrl = 6;
                out1.push(x);
                aux_ctrl = 7;
            }
        }
    }
};