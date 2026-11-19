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
 * Latency Annotation Testbench
 *
 * This testbench demonstrates back-annotation of HLS pipeline latency on a
 * divergence/reconvergence DUT.
 *
 *   din -> divergence -> [block0, block1] -> reconvergence -> dout
 *
 * Latency is applied to the msg_out ports block0_inst.dout and
 * block1_inst.dout (which call reset_push and own the schedule). Annotating
 * the bound channels (bout0, bout1) instead would be inert because the
 * Pre()/Post() scheduling is tied to the port that calls reset_push().
 *
 * Four annotation methods, selected via the command-line argument:
 *
 *   ./sim_sc 0   no annotations
 *   ./sim_sc 1   programmatic         (set_latency_capacity calls)
 *   ./sim_sc 2   text-file annotation (latencies.txt)
 *   ./sim_sc 3   JSON-file annotation (latencies.json)
 *
 * Default (no argument): method 0. The Makefile-driven `make run` therefore
 * exercises the no-annotation path; running ./sim_sc 1|2|3 by hand exercises
 * the others.
 *
 * Expected behavior with latency=3 on bout0 and bout1:
 *   - Data pushed at cycle N appears at the reconvergence inputs at cycle N+3
 *   - The reconvergence module sees both inputs arrive at the same cycle
 *   - Output appears 3 cycles after input (plus pipeline overhead)
 *
 * Back-annotation is always compiled in; no compile flag needed.
 */

#include "sc_hls_annotate.h"
#include "dut.h"
#include "timeout_thread.h"
#include <cstring>
#include <memory>

SC_MODULE(testbench) {
public:
    dut_top SC_NAMED(dut_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<sc_uint<16>> SC_NAMED(din);
    msg_channel<sc_uint<16>> SC_NAMED(dout);

    bool test_passed = 0;

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.din(din);
        dut_.dout(dout);

        SC_THREAD(reset);
        sensitive << clk.posedge_event();

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    // Drives the "din" port on the DUT.
    void proc_driver() {
        din.reset_push();
        wait();
        SC_HLS_LOGP("started");

        for (int i = 1; i < 20; i++) {
            SC_HLS_LOGP("sending i=" << i);
            din.push(i);
        }
        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }

    void proc_monitor() {
        dout.reset_pop();
        wait();
        SC_HLS_LOGP("started");
        for (int i = 1; i < 20; i++) {
            sc_uint<16> act = dout.pop();
            SC_HLS_LOGP("got : "  << act);
            if (!(act == (2*i))) {
                SC_REPORT_ERROR(name(), ("mismatch: ref="+std::to_string(i*2)+
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

    // Parse command-line argument to select the annotation method.
    //   ./sim_sc 0   no annotations (default)
    //   ./sim_sc 1   programmatic
    //   ./sim_sc 2   text file (latencies.txt)
    //   ./sim_sc 3   JSON file (latencies.json)
    int method = 0;
    if (argc >= 2) {
        method = std::atoi(argv[1]);
        if (method < 0 || method > 3) {
            std::cerr << "Usage: " << argv[0] << " [0|1|2|3]\n"
                      << "  0 = No annotations (default)\n"
                      << "  1 = Programmatic annotation\n"
                      << "  2 = Text file annotation (latencies.txt)\n"
                      << "  3 = JSON file annotation (latencies.json)\n";
            return 1;
        }
    }

    auto tb_top = std::make_shared<testbench>("tb_top");

    // Apply latency annotations on the two parallel paths to model HLS
    // pipeline stages. All three non-zero methods produce identical results.
    if (method == 0) {
        std::cout << "Using Method 0: No annotation\n";
    } else if (method == 1) {
        // latency=3, capacity=3 models an II=1 pipeline with 3-cycle latency.
        std::cout << "Using Method 1: Programmatic annotation\n";
        tb_top->dut_.block0_inst.dout.set_latency_capacity(3, 3);
        tb_top->dut_.block1_inst.dout.set_latency_capacity(3, 3);
    } else if (method == 2) {
        // Text file format: "<channel_path> <latency> [capacity]" per line.
        std::cout << "Using Method 2: Text file annotation (latencies.txt)\n";
        sc_hls::msg_lib::annotate::load_annotations("latencies.txt");
    } else if (method == 3) {
        // JSON format:
        //   { "channels": { "<path>": { "latency": N, "capacity": M } } }
        std::cout << "Using Method 3: JSON file annotation (latencies.json)\n";
        sc_hls::msg_lib::annotate::load_annotations_json("latencies.json");
    }

    // Timeout guard against infinite hangs.
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    sc_trace_file* tf = sc_create_vcd_trace_file("waves");
    trace_hierarchy(tb_top.get(), tf);

    sc_start();
    sc_close_vcd_trace_file(tf);

    if ((!tb_top->test_passed) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED with " << sc_report_handler::get_count(SC_ERROR)
                  << " errors" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}