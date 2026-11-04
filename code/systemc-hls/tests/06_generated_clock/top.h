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
 * top.h — drives the DUT's clock manually instead of using sc_clock.
 *
 * clkgen toggles clk_sig by hand:
 *   wait(1, SC_NS); clk_sig = 0; wait(1, SC_NS); clk_sig = 1;
 *
 * sc_hls's clock_manager only discovers clocks via dynamic_cast<sc_clock*>
 * (see sc_hls_manager.h: clock_manager::find_clocks()), so a hand-toggled
 * sc_signal<bool> is never registered even though dut.proc is correctly
 * sensitive to its posedge_event(). The expected result is a "No clocks
 * found" SC_HLS error at first port access.
 */

#pragma once

#include "dut.h"

SC_MODULE(Top) {
    dut SC_NAMED(d);
    sc_signal<bool> SC_NAMED(clk_sig);
    sc_signal<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_channel<sc_uint<16>> SC_NAMED(out_chan);

    SC_CTOR(Top) {
        d.clk(clk_sig);
        d.rst_bar(rst_bar);
        d.in(in_chan);
        d.out(out_chan);

        SC_THREAD(clkgen);

        SC_THREAD(driver);
        sensitive << clk_sig.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(reset_proc);
        sensitive << clk_sig.posedge_event();
    }

    void clkgen() {
        while (1) {
            wait(1, SC_NS);
            clk_sig = 0;
            wait(1, SC_NS);
            clk_sig = 1;
        }
    }

    void driver() {
        in_chan.reset_push();
        wait();
        in_chan.push(100);
        wait();
    }

    void reset_proc() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};
