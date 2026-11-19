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
 * Throughput-Accurate Message Passing Example - testbench / driver / monitor
 *
 * This file declares the structured testbench used by 01_xput_accurate:
 *
 *   testbench
 *     ├── dut_  (the design under test)
 *     ├── drv_  (a separate driver module that pushes stimulus)
 *     └── mon_  (a separate monitor module that pops + checks)
 *
 * Most other examples in this directory inline stim/resp threads directly in
 * `testbench`. This example keeps `driver` and `monitor` as separate modules
 * to demonstrate the more structured pattern. (Example 12 also uses this
 * pattern, made cleaner by templating on `ViewTag`.)
 */

#pragma once

#include "dut.h"

SC_MODULE(driver) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_out<sc_uint<16>> SC_NAMED(out1);
    msg_out<sc_uint<16>> SC_NAMED(out2);
    msg_out<sc_uint<16>> SC_NAMED(out3);

    SC_CTOR(driver) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        out1.reset_push();
        out2.reset_push();
        out3.reset_push();
        wait();
        SC_HLS_LOGP("started");
        int cnt = 1;
        for (int i = 0; i < 10; i++) {
            SC_HLS_LOGP("sending i=" << i);
            out1.push(cnt++);
            if (i & 1) {
                out3.push(cnt++);
                out2.push(cnt++);
            } else {
                out2.push(cnt++);
                out3.push(cnt++);
            }
        }
        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }
};

SC_MODULE(monitor) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<32>> SC_NAMED(in1);
    msg_in<sc_uint<32>> SC_NAMED(in2);

    bool test_passed = 0;

    SC_CTOR(monitor) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        in1.reset_pop();
        in2.reset_pop();
        wait();
        SC_HLS_LOGP("started");

        const std::vector<std::pair<unsigned, unsigned>> ref_values = {
            {2, 6},      {24, 120},   {56, 504},    {120, 1320},  {182, 2730},
            {288, 4896}, {380, 7980}, {528, 12144}, {650, 17550}, {840, 24360}};

        for (std::size_t i = 0; i < 10; i++) {
            auto act1 = in1.pop();
            auto act2 = in2.pop();
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

SC_MODULE(testbench) {
public:
    dut SC_NAMED(dut_);
    driver SC_NAMED(drv_);
    monitor SC_NAMED(mon_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<sc_uint<16>> SC_NAMED(in1);
    msg_channel<sc_uint<16>> SC_NAMED(in2);
    msg_channel<sc_uint<16>> SC_NAMED(in3);
    msg_channel<sc_uint<32>> SC_NAMED(out1);
    msg_channel<sc_uint<32>> SC_NAMED(out2);

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        // DUT
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.in1(in1);
        dut_.in2(in2);
        dut_.in3(in3);
        dut_.out1(out1);
        dut_.out2(out2);

        // Driver: pushes onto in1/in2/in3
        drv_.clk(clk);
        drv_.rst_bar(rst_bar);
        drv_.out1(in1);
        drv_.out2(in2);
        drv_.out3(in3);

        // Monitor: pops from out1/out2 and checks
        mon_.clk(clk);
        mon_.rst_bar(rst_bar);
        mon_.in1(out1);
        mon_.in2(out2);

        SC_CTHREAD(reset, clk);
    }

    void reset() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};
