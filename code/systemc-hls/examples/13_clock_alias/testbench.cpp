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
 * Clock Alias Testbench
 *
 * Demonstrates sc_hls::add_clock_alias(): running a design whose clock is a
 * hand-toggled sc_signal<bool> (NOT an sc_clock) cycle-accurately.
 *
 * The sc_hls clock manager discovers clocks only via dynamic_cast<sc_clock*>
 * while walking the hierarchy, and the pre/post scheduler needs a clock's
 * period() to place pre() one epsilon before, and post() one epsilon after,
 * each active edge. A plain sc_signal<bool> toggled by a thread is therefore
 * invisible to the library -- a design driven only by such a signal fails with
 * "No clocks found".
 *
 * add_clock_alias(companion_sc_clock, toggled_signal) bridges the gap: create a
 * companion sc_clock whose period and phase match the toggled signal, and alias
 * the signal's posedge to it. Processes statically sensitive to the toggled
 * signal then resolve into the companion clock's timing domain.
 *
 * Cycle-accuracy caveat: the scheduler free-runs on the companion sc_clock's
 * period/phase -- it is NOT synchronized to the toggled signal's edges.
 * Correctness requires the companion clock to match the signal (period AND
 * phase). This is not validated by the library; the VCD cross-check in the
 * README confirms alignment.
 */

#include "dut.h"
#include "timeout_thread.h"
#include <memory>

SC_MODULE(testbench) {
public:
    static constexpr unsigned N = 10;

    dut SC_NAMED(dut_);

    // The design's clock is a plain signal, toggled by hand in clkgen().
    sc_signal<bool> SC_NAMED(clk);
    sc_signal<bool> SC_NAMED(rst_bar);

    // Companion clock: period 2 ns, 50% duty, start 0, posedge first.
    // Matches clkgen()'s waveform below (posedge at t = 0, 2, 4, ... ns).
    sc_clock clk_alias;

    msg_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_channel<sc_uint<16>> SC_NAMED(out_chan);

    bool test_passed = 0;

    SC_CTOR(testbench)
        : clk_alias("clk_alias", 2, SC_NS, 0.5, 0, SC_NS, true)
    {
        // Tie the hand-toggled clk signal to the companion sc_clock.
        add_clock_alias(clk_alias, clk);

        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.in(in_chan);
        dut_.out(out_chan);

        SC_THREAD(clkgen);

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(reset);
        sensitive << clk.posedge_event();
    }

    // Hand-generated clock: posedge at t = 0, 2, 4, ... ns (matches clk_alias).
    void clkgen() {
        while (1) {
            clk = 1;
            wait(1, SC_NS);
            clk = 0;
            wait(1, SC_NS);
        }
    }

    void proc_driver() {
        in_chan.reset_push();
        wait();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < N; i++) {
            SC_HLS_LOGP("sending i=" << i);
            in_chan.push(i);
        }
        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }

    void proc_monitor() {
        out_chan.reset_pop();
        wait();
        SC_HLS_LOGP("started");
        for (unsigned i = 0; i < N; i++) {
            auto act = out_chan.pop();
            SC_HLS_LOGP("got : " << act);
            if (act != i) {
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
    sc_trace(tf, tb_top->clk, "clk");
    sc_trace(tf, tb_top->clk_alias, "clk_alias");

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
