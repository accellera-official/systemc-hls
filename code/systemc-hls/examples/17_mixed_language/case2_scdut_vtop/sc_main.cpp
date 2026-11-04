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
 * Mixed-language Example, case 2 - SystemC DUT / Verilog top
 *
 * The Verilog top (vtop.v) generates the clock and reset and checks the
 * SystemC block's output. Verilator cannot instantiate an sc_module inside
 * Verilog, so this harness instantiates the Verilated top and the SystemC
 * wrapper side by side and connects them with plain signals. The Verilog
 * still owns clk and rst_n.
 *
 * PASS requires: the Verilog consumer saw all N items in order (done && ok),
 * no SC_ERROR / SC_FATAL (this includes the wrapper's clock-mismatch check).
 */

#include "dut_wrap.h"
#include "Vvtop.h"
#include "timeout_thread.h"

// Stops the simulation when the Verilog top reports done.
SC_MODULE(stopper) {
    sc_in<bool> SC_NAMED(done);
    SC_CTOR(stopper) {
        SC_METHOD(proc);
        sensitive << done.pos();
        dont_initialize();
    }
    void proc() { sc_stop(); }
};

int sc_main(int argc, char** argv) {
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);

    sc_signal<bool>     SC_NAMED(clk);
    sc_signal<bool>     SC_NAMED(rst_n);
    sc_signal<bool>     SC_NAMED(in1_rdy);
    sc_signal<bool>     SC_NAMED(in1_vld);
    sc_signal<uint32_t> SC_NAMED(in1_dat);
    sc_signal<bool>     SC_NAMED(out1_rdy);
    sc_signal<bool>     SC_NAMED(out1_vld);
    sc_signal<uint32_t> SC_NAMED(out1_dat);
    sc_signal<bool>     SC_NAMED(done);
    sc_signal<bool>     SC_NAMED(ok);

    // Verilog top: clock period 2ns, first posedge at 0ns.
    Vvtop vtop("vtop");
    vtop.clk(clk);
    vtop.rst_n(rst_n);
    vtop.in1_rdy(in1_rdy);
    vtop.in1_vld(in1_vld);
    vtop.in1_dat(in1_dat);
    vtop.out1_rdy(out1_rdy);
    vtop.out1_vld(out1_vld);
    vtop.out1_dat(out1_dat);
    vtop.done(done);
    vtop.ok(ok);

    // SystemC block; companion clock parameters match vtop's clock.
    dut_wrap wrap("wrap", sc_time(2, SC_NS), SC_ZERO_TIME);
    wrap.clk(clk);
    wrap.rst_n(rst_n);
    wrap.in1_rdy(in1_rdy);
    wrap.in1_vld(in1_vld);
    wrap.in1_dat(in1_dat);
    wrap.out1_rdy(out1_rdy);
    wrap.out1_vld(out1_vld);
    wrap.out1_dat(out1_dat);

    stopper stop("stop");
    stop.done(done);

    // Timeout guard against infinite hangs (500 ns timeout, 100 ns ping interval).
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    // R6: there is no msg_channel at this boundary, so trace_hierarchy()
    // finds nothing to trace; the boundary pins are traced explicitly.
    sc_trace_file* tf = sc_create_vcd_trace_file("waves");
    sc_trace(tf, clk, "clk");
    sc_trace(tf, wrap.clk_companion, "clk_companion");
    sc_trace(tf, rst_n, "rst_n");
    sc_trace(tf, in1_rdy, "in1_rdy");
    sc_trace(tf, in1_vld, "in1_vld");
    sc_trace(tf, in1_dat, "in1_dat");
    sc_trace(tf, out1_rdy, "out1_rdy");
    sc_trace(tf, out1_vld, "out1_vld");
    sc_trace(tf, out1_dat, "out1_dat");

    sc_start();
    sc_close_vcd_trace_file(tf);

    if (!done.read() || !ok.read() ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED (done=" << done.read()
                  << " ok=" << ok.read() << ")" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}
