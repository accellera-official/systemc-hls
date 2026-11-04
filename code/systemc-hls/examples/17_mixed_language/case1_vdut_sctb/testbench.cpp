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
 * Mixed-language Example, case 1 - Verilog DUT / SystemC testbench
 *
 * The DUT is RTL (vpass.v), Verilated and wrapped in a pins-only sc_module
 * (vpass_wrap.h). The testbench is ordinary sc_hls code: a driver pushes
 * N values through msg_out, a monitor pops them through msg_in and checks
 * them in order. The clock is an sc_clock on the SystemC side.
 *
 * The two msg_channels are the boundary: their rdy/vld/dat signals are bound
 * to the wrapper pins. The testbench side of each channel is a normal,
 * reset endpoint; the DUT side is the RTL.
 */

#include <systemc-hls>
#include "vpass_wrap.h"
#include "timeout_thread.h"
#include <memory>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

static constexpr unsigned N = 10;

static uint32_t ref_value(unsigned i) { return i * 7 + 3; }

SC_MODULE(driver) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_out<uint32_t> SC_NAMED(out);

    SC_CTOR(driver) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        out.reset_push();
        wait();
    }

    void proc() {
        reset();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < N; i++) {
            SC_HLS_LOGP("sending " << ref_value(i));
            out.push(ref_value(i));
        }
        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }
};

SC_MODULE(monitor) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<uint32_t> SC_NAMED(in);

    unsigned received = 0;

    SC_CTOR(monitor) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in.reset_pop();
        wait();
    }

    void proc() {
        reset();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < N; i++) {
            auto act = in.pop();
            SC_HLS_LOGP("got : " << act);
            if (act != ref_value(i)) {
                SC_REPORT_ERROR(name(), ("mismatch: ref=" + std::to_string(ref_value(i)) +
                                 ", act=" + std::to_string(act)).c_str());
            }
            received++;
        }
        SC_HLS_LOGP("all expected values received");
        wait(10);
        sc_stop();
    }
};

SC_MODULE(testbench) {
public:
    vpass_wrap SC_NAMED(dut_);
    driver SC_NAMED(drv_);
    monitor SC_NAMED(mon_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<uint32_t> SC_NAMED(in1);
    msg_channel<uint32_t> SC_NAMED(out1);

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        // RTL DUT: channel signals bind directly to the wrapper pins.
        dut_.clk(clk);
        dut_.rst_n(rst_bar);
        dut_.in1_rdy(in1.rdy);
        dut_.in1_vld(in1.vld);
        dut_.in1_dat(in1.dat);
        dut_.out1_rdy(out1.rdy);
        dut_.out1_vld(out1.vld);
        dut_.out1_dat(out1.dat);

        drv_.clk(clk);
        drv_.rst_bar(rst_bar);
        drv_.out(in1);

        mon_.clk(clk);
        mon_.rst_bar(rst_bar);
        mon_.in(out1);

        SC_CTHREAD(reset, clk);
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
    sc_trace(tf, tb_top->clk, "clk");
    sc_trace(tf, tb_top->rst_bar, "rst_bar");

    sc_start();
    sc_close_vcd_trace_file(tf);

    if ((tb_top->mon_.received != N) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED (received " << tb_top->mon_.received
                  << "/" << N << ")" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}
