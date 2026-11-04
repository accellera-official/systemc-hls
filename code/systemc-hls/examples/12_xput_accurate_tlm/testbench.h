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
 * TLM Fast Simulation Example - testbench / driver / monitor
 *
 * Templated `testbench<ViewTag>` that wires together a `driver`, the DUT,
 * and a `monitor`. The `ViewTag` parameter selects between TLM-fifo channels
 * (fast functional) and signal-based channels (cycle-accurate).
 *
 * Like 01_xput_accurate, this example uses the structured testbench pattern
 * (separate `driver`/`monitor` modules instead of inline stim/resp threads).
 * Templating helper modules on `ViewTag` is much cleaner than inlining them
 * into a templated `testbench`, which is why this example keeps the pattern.
 */

#pragma once

#include "dut.h"

template <typename ViewTag = default_view_tag>
SC_MODULE(driver) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_out<sc_uint<16>, ViewTag> SC_NAMED(out1);
    msg_out<sc_uint<16>, ViewTag> SC_NAMED(out2);
    msg_out<sc_uint<16>, ViewTag> SC_NAMED(out3);

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
        // Send 40 values (TLM mode buffers them through tlm_fifos of depth 20).
        for (int i = 0; i < 40; i++) {
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

template <typename ViewTag = default_view_tag>
SC_MODULE(monitor) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<32>, ViewTag> SC_NAMED(in1);
    msg_in<sc_uint<32>, ViewTag> SC_NAMED(in2);

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

        // Expected values for 40 iterations.
        std::vector<std::pair<unsigned, unsigned>> expected = {
            {2, 6},          {24, 120},        {56, 504},        {120, 1320},
            {182, 2730},     {288, 4896},      {380, 7980},      {528, 12144},
            {650, 17550},    {840, 24360},     {992, 32736},     {1224, 42840},
            {1406, 54834},   {1680, 68880},    {1892, 85140},    {2208, 103776},
            {2450, 124950},  {2808, 148824},   {3080, 175560},   {3480, 205320},
            {3782, 238266},  {4224, 274560},   {4556, 314364},   {5040, 357840},
            {5402, 405150},  {5928, 456456},   {6320, 511920},   {6888, 571704},
            {7310, 635970},  {7920, 704880},   {8372, 778596},   {9024, 857280},
            {9506, 941094},  {10200, 1030200}, {10712, 1124760}, {11448, 1224936},
            {11990, 1330890}, {12768, 1442784}, {13340, 1560780}, {14160, 1685040}};

        for (std::size_t i = 0; i < 40; i++) {
            auto v1 = in1.pop();
            auto v2 = in2.pop();
            SC_HLS_LOGP("got : act1=" << v1 << " act2=" << v2);
            if (not(v1 == expected[i].first)) {
                SC_REPORT_ERROR(name(), ("mismatch on v1, ref="+std::to_string(expected[i].first)+
                                 ", act="+std::to_string(v1)) .c_str());
            }
            if (not(v2 == expected[i].second)) {
                SC_REPORT_ERROR(name(), ("mismatch on v2, ref="+std::to_string(expected[i].second)+
                                 ", act="+std::to_string(v2)) .c_str());
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
    dut<tlm_view_tag> SC_NAMED(dut_);
    driver<tlm_view_tag> SC_NAMED(drv_);
    monitor<tlm_view_tag> SC_NAMED(mon_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    // TLM channels with depth=20.
    msg_channel<sc_uint<16>, tlm_view_tag> in1{"in1", 20};
    msg_channel<sc_uint<16>, tlm_view_tag> in2{"in2", 20};
    msg_channel<sc_uint<16>, tlm_view_tag> in3{"in3", 20};
    msg_channel<sc_uint<32>, tlm_view_tag> out1{"out1", 20};
    msg_channel<sc_uint<32>, tlm_view_tag> out2{"out2", 20};

    SC_CTOR(testbench)
        : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true)
    {
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
