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
 * Direct Input + Sync Testbench
 *
 * This testbench demonstrates:
 *   - Using sync_channel for explicit synchronization
 *   - Mixing msg channels with direct sc_signal inputs
 *   - Coordinating data transfer with configuration updates
 *   - Canonical testbench skeleton (testbench class, tb_top, waves.vcd)
 */

#include "dut.h"
#include "timeout_thread.h"
#include <array>
#include <memory>

SC_MODULE(testbench) {
public:
    dut SC_NAMED(dut_);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_channel<uint32_t> SC_NAMED(out1);
    sc_vector<msg_channel<uint32_t>> SC_NAMED(sample_in);
    sync_channel<> SC_NAMED(sync_chan);
    sc_vector<sc_signal<uint32_t>> SC_NAMED(direct_inputs);

    // Number of outputs produced by the DUT for the current stimulus. This is
    // data-dependent (driven by direct_inputs and num_samples); the value here
    // is the count observed for this fixed stimulus.
    static constexpr int NUM_OUTPUTS = 225;

    bool test_passed = 0;

    SC_CTOR(testbench) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        sample_in.init(dut::num_samples);
        direct_inputs.init(dut::num_direct_inputs);

        dut_.clk(clk);
        dut_.rst_bar(rst_bar);
        dut_.out1(out1);
        dut_.sync_port(sync_chan);

        for (int i = 0; i < dut::num_samples; i++) {
            dut_.sample_in[i](sample_in[i]);
        }

        for (int y = 0; y < dut::num_direct_inputs; y++) {
            dut_.direct_inputs[y](direct_inputs[y]);
        }

        SC_CTHREAD(reset, clk);

        SC_THREAD(proc_driver);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc_monitor);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    void proc_driver() {
        sync_chan.reset_push();

        for (int i = 0; i < dut::num_samples; i++) {
            sample_in[i].reset_push();
        }

        for (int y = 0; y < dut::num_direct_inputs; y++) {
            direct_inputs[y] = y;
        }

        direct_inputs[0] = 5;
        direct_inputs[1] = 5;
        direct_inputs[7] = 4;

        wait();
        SC_HLS_LOGP("started");

        for (int i = 0; i < 10; i++) {
            do {
                wait();
            } while (sync_chan.rdy == 0);

            // update dir inputs here
            direct_inputs[2] = 1 + direct_inputs[2];
            direct_inputs[3] = direct_inputs[4];
            direct_inputs[4] = direct_inputs[3];
            direct_inputs[7] = 4 + (i & 3);
            sync_chan.vld = 1;

            wait();
            sync_chan.vld = 0;

            for (uint32_t x = 0; x < direct_inputs[0]; x++) {
                for (uint32_t y = 0; y < direct_inputs[1]; y++) {
                    for (uint32_t s = 0; s < dut::num_samples; s++) {
                        SC_HLS_LOGP("sending i=" << ((x * y) + i + s));
                        sample_in[s].push((x * y) + i + s);
                    }
                }
            }
        }

        SC_HLS_LOGP("sent in all inputs");
        sc_event eee;
        wait(eee);
    }

    std::array<uint32_t,NUM_OUTPUTS> ref_values =  {
                          100, 100, 100, 100, 100, 100, 200, 300, 400, 500, 100,
                         300, 500, 700, 900, 100, 400, 700, 1000, 1300, 100, 500,
                         900, 1300, 1700, 220, 220, 220, 220, 220, 220, 330, 440,
                         550, 660, 220, 440, 660, 880, 1100, 220, 550, 880, 1210,
                         1540, 220, 660, 1100, 1540, 1980, 340, 340, 340, 340,
                         340, 340, 460, 580, 700, 820, 340, 580, 820, 1060, 1300,
                         340, 700, 1060, 1420, 1780, 340, 820, 1300, 1780, 2260,
                         500, 500, 500, 500, 500, 500, 630, 760, 890, 1020, 500,
                         760, 1020, 1280, 1540, 500, 890, 1280, 1670, 2060, 500,
                         1020, 1540, 2060, 2580, 660, 660, 660, 660, 660, 660,
                         800, 940, 1080, 1220, 660, 940, 1220, 1500, 1780, 660,
                         1080, 1500, 1920, 2340, 660, 1220, 1780, 2340, 2900,
                         860, 860, 860, 860, 860, 860, 1010, 1160, 1310, 1460,
                         860, 1160, 1460, 1760, 2060, 860, 1310, 1760, 2210,
                         2660, 860, 1460, 2060, 2660, 3260, 1060, 1060, 1060,
                         1060, 1060, 1060, 1220, 1380, 1540, 1700, 1060, 1380,
                         1700, 2020, 2340, 1060, 1540, 2020, 2500, 2980, 1060,
                         1700, 2340, 2980, 3620, 1300, 1300, 1300, 1300, 1300,
                         1300, 1470, 1640, 1810, 1980, 1300, 1640, 1980, 2320,
                         2660, 1300, 1810, 2320, 2830, 3340, 1300, 1980, 2660,
                         3340, 4020, 1540, 1540, 1540, 1540, 1540, 1540, 1720,
                         1900, 2080, 2260, 1540, 1900, 2260, 2620, 2980, 1540,
                         2080, 2620, 3160, 3700, 1540, 2260, 2980, 3700, 4420 };

    void proc_monitor() {
        out1.reset_pop();
        wait();
        SC_HLS_LOGP("started");

        for (int i = 0; i < NUM_OUTPUTS; i++) {
            uint32_t act = out1.pop();
            SC_HLS_LOGP("got : " << act);
            if (act != ref_values.at(i)) {
                SC_REPORT_ERROR(name(),
                    ("mismatch: at i="+std::to_string(i)+", ref="+std::to_string(ref_values.at(i))+
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

    if (sc_report_handler::get_count(SC_ERROR) > 0) {
        std::cout << "Simulation FAILED" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}
