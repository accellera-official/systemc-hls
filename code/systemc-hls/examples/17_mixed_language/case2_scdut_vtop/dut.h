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
 * Case 2 DUT: SystemC passthrough using the normal sc_hls API.
 *
 * Nothing in this module knows it will run inside a Verilog top: it uses
 * msg_in / msg_out, push()/pop(), and the usual reset() convention.
 */

#pragma once

#include <systemc-hls>
#include <cstdint>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_n);

    msg_in<uint32_t>  SC_NAMED(in1);
    msg_out<uint32_t> SC_NAMED(out1);

    SC_CTOR(dut) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_n, false);
    }

    void reset() {
        in1.reset_pop();
        out1.reset_push();
        wait();
    }

    void proc() {
        reset();
        while (1) {
            auto v = in1.pop();
            out1.push(v);
        }
    }
};
