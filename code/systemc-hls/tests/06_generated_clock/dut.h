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
 * dut.h — plain msg_in/msg_out pass-through DUT.
 *
 * Correctly resets both ports; the intended failure in this test is not in
 * the DUT itself but in how Top drives its clock (see top.h).
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(in);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(dut) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        in.reset_pop();
        out.reset_push();
        wait();
        auto val = in.pop();
        out.push(val);
    }
};
