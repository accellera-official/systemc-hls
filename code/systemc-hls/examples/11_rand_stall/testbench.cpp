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
 * Random Stall Injection Testbench
 *
 * Demonstrates how to enable random stalls on msg channels for verification.
 * Random stalls simulate back-pressure from downstream consumers, helping to
 * verify that designs correctly handle flow control.
 *
 * Usage:
 *   ./sim_sc        no random stalls (default)
 *   ./sim_sc 1      random stalls injected
 *   SC_HLS_RAND_SEED=42 ./sim_sc 1     reproducible stall sequence
 *
 * Random stall injection is always compiled in — no compile flag needed.
 */

#include "sc_hls_rand_stall.h"
#include "dut.h"
#include "timeout_thread.h"
#include <memory>

SC_MODULE(testbench) {
public:
    dut SC_NAMED(dut_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<sc_uint<32>> SC_NAMED(in1);
    msg_channel<sc_uint<32>> SC_NAMED(in2);
    msg_channel<sc_uint<32>> SC_NAMED(in3);
    msg_channel<sc_uint<32>> SC_NAMED(out1);
    msg_channel<sc_uint<32>> SC_NAMED(out2);

    bool test_passed = 0;

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.in1(in1);
        dut_.in2(in2);
        dut_.in3(in3);
        dut_.out1(out1);
        dut_.out2(out2);

        SC_THREAD(reset);
        sensitive << clk.posedge_event();

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        rst_bar.write(false);
        wait(5);
        rst_bar.write(true);
        wait();
    }

    void proc_driver() {
        in1.reset_push();
        in2.reset_push();
        in3.reset_push();
        wait();

        SC_HLS_LOGP("started");

        for (int i = 0; i < 10; i++) {
            SC_HLS_LOGP("sending i=" << i);
            in1.push(i);
            if (i & 1) {
                in3.push(0x1000);
                in2.push(0x100);
            } else {
                in2.push(0x0);
                in3.push(0x0);
            }
        }

        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }

    const std::vector<std::pair<unsigned,unsigned>> ref_values = {
        {   0 ,    0 },
        { 257 , 4097 },
        {   2 ,    2 },
        { 259 , 4099 },
        {   4 ,    4 },
        { 261 , 4101 },
        {   6 ,    6 },
        { 263 , 4103 },
        {   8 ,    8 },
        { 265 , 4105 },
    };

    void proc_monitor() {
        out1.reset_pop();
        out2.reset_pop();
        wait();
        SC_HLS_LOGP("started");

        for (int i = 0; i < 10; i++) {
            sc_uint<32> act1 = out1.pop();
            sc_uint<32> act2 = out2.pop();
            SC_HLS_LOGP("got : act1=" << act1 << " act2=" << act2);

            if (not(act1 == ref_values.at(i).first)) {
                SC_REPORT_ERROR(name(), ("mismatch on act1, "
                    "ref=" + std::to_string(ref_values.at(i).first) + ", "
                    "act=" + std::to_string(act1)).c_str());
            }
            if (not(act2 == ref_values.at(i).second)) {
                SC_REPORT_ERROR(name(), ("mismatch on act2, "
                    "ref=" + std::to_string(ref_values.at(i).second) + ", "
                    "act=" + std::to_string(act2)).c_str());
            }
        }

        SC_HLS_LOGP("all expected values received");
        test_passed = 1;
        wait(10);
        sc_stop();
    }
};

int sc_main(int argc, char** argv) {
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);

    // Parse command-line argument:
    //   ./sim_sc 0   no random stalls (default)
    //   ./sim_sc 1   random stalls injected on all msg channels
    int do_stall = 0;
    if (argc >= 2) {
        do_stall = std::atoi(argv[1]);
        if (do_stall < 0 || do_stall > 1) {
            std::cerr << "Usage: " << argv[0] << " [0|1]\n"
                      << "  0 = No random stalls (default)\n"
                      << "  1 = Random stalls injected\n";
            return 1;
        }
    }

    auto tb_top = std::make_shared<testbench>("tb_top");

    if (do_stall) {
        // Enable random stalls on all channels in the hierarchy.
        // verbose=true prints the stall probability for each channel.
        std::cout << "\n=== Enabling Random Stalls ===\n";
        sc_hls::msg_lib::rand_stall::set(*tb_top, true);
        std::cout << "==============================\n\n";
    } else {
        std::cout << "\n=== Random Stalls disabled ===\n";
    }

    // Timeout guard (2000 ns to allow extra cycles when stalls are active).
    add_timeout_thread(2000, SC_NS, 100, SC_NS);

    sc_trace_file* tf = sc_create_vcd_trace_file("waves");
    trace_hierarchy(tb_top.get(), tf);

    sc_start();
    sc_close_vcd_trace_file(tf);

    if ((!tb_top->test_passed) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED\n";
        return 1;
    }

    std::cout << "Simulation PASSED\n";
    return 0;
}