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

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

// Simple DUT with multiple inputs and outputs to demonstrate stall injection.
// The DUT reads from three inputs and writes to two outputs.
// With random stalls enabled, the ready signals on the input channels will
// be randomly deasserted, causing the DUT to stall and wait for data.

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<32>> SC_NAMED(in1);
    msg_in<sc_uint<32>> SC_NAMED(in2);
    msg_in<sc_uint<32>> SC_NAMED(in3);
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
        in3.reset_pop();
        out1.reset_push();
        out2.reset_push();
        wait();
    }

    void main() {
        reset();
        while (true) {
            sc_uint<32> i1 = in1.pop();
            sc_uint<32> i2 = in2.pop();
            sc_uint<32> i3 = in3.pop();
            out1.push(i1 + i2);
            out2.push(i1 + i3);
        }
    }
};