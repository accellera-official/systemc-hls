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
 * Transaction Logging Testbench
 *
 * Demonstrates the transaction-logging facility of the sc_hls library and how
 * a user extends it with a custom output format.
 *
 * The log format is selected at COMPILE time via the SC_HLS_EMITTER macro
 * (see custom_emitters.h). With no macro, the library's built-in pipe-delimited
 * text emitter is used. The Makefile's run-jsonl / run-csv targets rebuild with
 * -DSC_HLS_EMITTER=json_emitter_tag / =csv_emitter_tag to select a user emitter.
 *
 * The output filename is provided by the LOG_FILENAME macro (set by the
 * Makefile); it defaults to a text filename for a plain `make run`.
 */

#include "custom_emitters.h" // pulls in <systemc-hls> + user emitter specializations
#include "dut.h"
#include "timeout_thread.h"
#include <memory>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

#ifndef LOG_FILENAME
#define LOG_FILENAME "example_log.txt"
#endif

SC_MODULE(testbench) {
public:
    dut SC_NAMED(dut_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_channel<sc_uint<32>> SC_NAMED(out_chan);

    bool test_passed = 0;

    SC_CTOR(testbench) : clk("clk", 10, SC_NS) {
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.in1(in_chan);
        dut_.out1(out_chan);

        SC_CTHREAD(reset, clk);

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    void proc_driver() {
        in_chan.reset_push();
        wait();
        SC_HLS_LOGP("started");

        for (int i = 0; i < 20; i++) {
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

        for (unsigned i = 0; i < 20; i++) {
            auto act = out_chan.pop();
            SC_HLS_LOGP("got : " << act);
            if (act != (i * i)) {
                SC_REPORT_ERROR(name(), ("mismatch: ref="+std::to_string(i*i)+
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

    // channel_logs must outlive sc_start(); declare it after the testbench so
    // it is destroyed first on scope exit.
    sc_hls::msg_lib::channel_logs logs;
    sc_hls::msg_lib::emitter_options opts;
    opts.log_direction = true;
    opts.unbuffered = true;

    // The active emitter (and thus the format) is fixed at compile time by
    // SC_HLS_EMITTER; here we just name the output file and register the hierarchy.
    logs.enable(LOG_FILENAME, opts);
    logs.log_hierarchy(*tb_top);

    // Timeout guard against infinite hangs.
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    sc_trace_file* tf = sc_create_vcd_trace_file("waves");
    trace_hierarchy(tb_top.get(), tf);

    sc_start();
    sc_close_vcd_trace_file(tf);

    std::cout << "\nLogging file created: " << LOG_FILENAME << std::endl;

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
