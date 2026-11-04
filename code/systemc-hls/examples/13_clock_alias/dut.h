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
 * Clock Alias Example - DUT
 *
 * A plain msg (dat/vld/rdy) pass-through: whatever is pushed into `in`
 * is forwarded to `out`, one value per handshake.
 *
 * The DUT is an ordinary cycle-accurate sc_hls design. Its process is
 * statically sensitive to clk.pos(). It is unaware that, in this example, its
 * clock is a hand-toggled sc_signal<bool> rather than an sc_clock -- that
 * detail is handled entirely in the testbench via add_clock_alias().
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

    void reset() {
        in.reset_pop();
        out.reset_push();
        wait();
    }

    void proc() {
        reset();
        while (1) {
            auto v = in.pop();
            out.push(v);
        }
    }
};
