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
 * Ping-Pong Memory Testbench
 *
 * This testbench demonstrates:
 *   - Using sync_channel for memory synchronization between two DUT threads
 *   - Ping-pong buffering pattern with double-buffered memory
 *   - VCD waveform tracing including the internal sync channel
 *   - Canonical testbench skeleton (testbench class, tb_top, waves.vcd)
 */

#include "dut.h"
#include "timeout_thread.h"
#include <memory>

SC_MODULE(testbench) {
    using T = sc_uint<16>;

public:
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<T> SC_NAMED(out1);
    msg_channel<T> SC_NAMED(in1);

    dut SC_NAMED(dut_);

    bool test_passed = 0;

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.out1(out1);
        dut_.in1(in1);

        SC_CTHREAD(reset, clk);

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    void proc_driver() {
        in1.reset_push();
        wait();
        SC_HLS_LOGP("started");

        for (int i = 0; i < 40; i++) {
            SC_HLS_LOGP("sending i=" << i);
            in1.push(i);
        }

        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }

    void proc_monitor() {
        out1.reset_pop();
        wait();
        SC_HLS_LOGP("started");

        for (unsigned i = 0; i < 40; i++) {
            auto act = out1.pop();
            SC_HLS_LOGP("got : " << act);
            if (!(act == i)) {
                SC_REPORT_ERROR(name(), ("mismatch: ref="+std::to_string(i)+
                    ", act="+std::to_string(act)).c_str());
            }
        }
        SC_HLS_LOGP("all expected values received");
        test_passed = 1;
        wait(10);
        sc_stop();
    }

    void reset() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};

int sc_main(int argc, char** argv) {
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);

    auto tb_top = std::make_shared<testbench>("tb_top");

    // Timeout guard against infinite hangs (500 ns timeout, 100 ns ping interval).
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    sc_trace_file* tf = sc_create_vcd_trace_file("waves");
    trace_hierarchy(tb_top.get(), tf);

    sc_start();
    sc_close_vcd_trace_file(tf);

    if ((!tb_top->test_passed) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}