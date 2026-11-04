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
 * Sub-test: reset_push() forbidden when enable_port_access() is enabled.
 *
 * With port access enabled (in the constructor), calling reset_push() must
 * report an SC_HLS "forbidden" error — the user owns the pins and must not call
 * the reset functions.
 *
 * PASS if that "forbidden" error is captured.
 */

#include "common.h"
#include <memory>

SC_MODULE(reset_dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(reset_dut) {
        out.enable_port_access();   // elaboration phase: OK
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        out.reset_push();           // FORBIDDEN under port access
        wait();
        sc_stop();
    }
};

SC_MODULE(top) {
    reset_dut SC_NAMED(d);
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

    if (saw_sc_hls_error_containing("forbidden")) {
        std::cout << "PASS: reset_push() forbidden under port access" << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected a 'forbidden' SC_HLS error for reset_push()" << std::endl;
    return 1;
}
