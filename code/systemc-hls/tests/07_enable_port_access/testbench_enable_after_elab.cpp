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
 * Sub-test: enable_port_access() forbidden after elaboration.
 *
 * enable_port_access() may only be called during elaboration (in the module
 * constructor). Here it is called from inside a process — i.e. after sc_start()
 * has advanced past elaboration into SC_RUNNING — which must report an SC_HLS
 * "must be called during elaboration" error.
 *
 * PASS if that error is captured.
 */

#include "common.h"
#include <memory>

SC_MODULE(late_enable_dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(late_enable_dut) {
        // NOTE: enable_port_access() intentionally NOT called here (in the
        // constructor). It is called too late, from proc() during simulation.
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        out.enable_port_access();   // TOO LATE — not elaboration phase
        wait();
        sc_stop();
    }
};

SC_MODULE(top) {
    late_enable_dut SC_NAMED(d);
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(out_chan);

    SC_CTOR(top) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        d.clk(clk);
        d.rst_bar(rst_bar);
        d.out(out_chan);
        SC_CTHREAD(reset, clk);
    }

    void reset() {
        rst_bar.write(0);
        wait(3);
        rst_bar.write(1);
        wait();
    }
};

int sc_main(int, char**) {
    install_capturing_handler();
    auto t = std::make_shared<top>("top");
    sc_start(20, SC_NS);

    if (saw_sc_hls_error_containing("must be called during elaboration")) {
        std::cout << "PASS: enable_port_access() after elaboration reported" << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected an elaboration-phase SC_HLS error" << std::endl;
    return 1;
}
