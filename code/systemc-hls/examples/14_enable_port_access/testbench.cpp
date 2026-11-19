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
 * enable_port_access Testbench (manual msg passthrough)
 *
 * A normal (auto-scheduled) driver pushes 0..9 into the DUT's input, and a
 * normal monitor pops from the DUT's output and checks 0..9 in order. Only the
 * DUT uses enable_port_access() and drives its pins by hand; the driver and
 * monitor use the ordinary push()/pop() API, so this isolates the manual side.
 */

#include "dut.h"
#include "timeout_thread.h"
#include <memory>

SC_MODULE(driver) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(driver) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        out.reset_push();
        wait();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < 10; i++) {
            SC_HLS_LOGP("sending i=" << i);
            out.push(i);
        }
        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }
};

SC_MODULE(monitor) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(in);

    bool test_passed = 0;

    SC_CTOR(monitor) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        in.reset_pop();
        wait();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < 10; i++) {
            auto act = in.pop();
            SC_HLS_LOGP("got : " << act);
            if (act != i) {
                SC_REPORT_ERROR(name(), ("mismatch: ref=" + std::to_string(i) +
                                 ", act=" + std::to_string(act)) .c_str());
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

    msg_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_channel<sc_uint<16>> SC_NAMED(out_chan);

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.in(in_chan);
        dut_.out(out_chan);

        drv_.clk(clk);
        drv_.rst_bar(rst_bar);
        drv_.out(in_chan);

        mon_.clk(clk);
        mon_.rst_bar(rst_bar);
        mon_.in(out_chan);

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

    sc_start();
    sc_close_vcd_trace_file(tf);

    if ((!tb_top->mon_.test_passed) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}
