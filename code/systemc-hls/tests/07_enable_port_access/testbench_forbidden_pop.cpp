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
 * Sub-test: pop() forbidden after enable_port_access() on a msg_in reader.
 *
 * PASS if an SC_HLS error mentioning "forbidden" is reported when the process
 * calls pop() on a port that has port access enabled.
 */

#include "common.h"
#include <memory>

SC_MODULE(reader_dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(in);

    SC_CTOR(reader_dut) {
        in.enable_port_access();
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        // Port access is enabled, so reset_pop() must NOT be called (it would
        // itself error). Under port access, calling pop() is forbidden.
        wait();
        sc_uint<16> v = in.pop(); // FORBIDDEN in port-access mode
        (void)v;
        sc_stop();
    }
};

SC_MODULE(top) {
    reader_dut SC_NAMED(d);
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(in_chan);

    SC_CTOR(top) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        d.clk(clk);
        d.rst_bar(rst_bar);
        d.in(in_chan);
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
        std::cout << "PASS: pop() forbidden under port access" << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected a 'forbidden' SC_HLS error for pop()" << std::endl;
    return 1;
}
