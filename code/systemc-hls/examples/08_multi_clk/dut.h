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
 * Multi-Clock Domain Example
 *
 * This example demonstrates handling multiple clock domains in SystemC-HLS lib.
 * The DUT operates on a different clock (clk5) than the testbench (clk1),
 * showing how channels handle clock domain crossing automatically.
 *
 * Key features:
 * - DUT runs on clk5 (5ns period)
 * - Testbench runs on clk1 (1ns period)
 * - Channels handle synchronization automatically
 * - Demonstrates proper multi-clock design patterns
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk5);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_out<uint32_t> SC_NAMED(out1);
    msg_in<uint32_t> SC_NAMED(in1);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk5.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        out1.reset_push();
        in1.reset_pop();
        wait();
    }

    void main() {
        reset();
        while (1) {
            auto v = in1.pop();
            SC_HLS_LOGP("got v="<<v);
            out1.push(v + 0x100);
        }
    }
};