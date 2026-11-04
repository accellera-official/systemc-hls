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
 * Transaction Logging DUT
 *
 * Simple squaring DUT used to demonstrate the transaction logging facility:
 *   - 1 input port (in1)  : sc_uint<16>
 *   - 1 output port (out1): sc_uint<32>
 *
 * For each value popped from in1, push (in1 * in1) to out1.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(in1);
    msg_out<sc_uint<32>> SC_NAMED(out1);

    SC_CTOR(dut) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        out1.reset_push();
        wait();
    }

    void proc() {
        reset();
        while (1) {
            for (int i = 0; i < 10; i++) {
                auto val = in1.pop();
                auto result = val * val; // Square the input
                out1.push(result);
            }
        }
    }
};