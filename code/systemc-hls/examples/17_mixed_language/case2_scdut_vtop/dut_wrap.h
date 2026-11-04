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
 * Case 2 wrapper: pins-only boundary around a SystemC DUT, for instantiation
 * inside a Verilog design that owns the clock and reset.
 *
 * This is the wrapper an HLS / co-simulation flow would generate around a
 * SystemC block:
 *   - R1: the boundary is pins only, named per §7.4.2 (<port>_rdy/_vld/_dat)
 *     plus clk / rst_n. The DUT's msg_in / msg_out pins bind port-to-port to
 *     the wrapper ports. There is no msg_channel: the "channel" is the
 *     Verilog wiring.
 *   - R2: the sc_hls scheduler needs an sc_clock (it discovers clocks by
 *     dynamic_cast<sc_clock*> and places pre()/post() from the clock's
 *     period). The Verilog clock is just a bool signal, so the wrapper holds a
 *     companion sc_clock and aliases the clk pin to it (as in examples/13).
 *     The scheduler free-runs on the companion's period/phase, so they are
 *     wrapper parameters and MUST match the Verilog clock. clk_check()
 *     enforces this at every Verilog posedge.
 *     The scheduler also places pre()/post() at k*period +/- epsilon from
 *     t=0, ignoring the sc_clock start time (sc_hls_manager.h run_for_clock),
 *     so the first posedge must be a multiple of the period. A clock with
 *     another phase is rejected at elaboration rather than mis-scheduled.
 *   - R3: reset comes straight from the Verilog rst_n.
 *   - R4 (Verilog side): DUT inputs must not change in (edge-0.02ns, edge];
 *     driving them from posedge-clocked registers satisfies this.
 */

#pragma once

#include "dut.h"

SC_MODULE(dut_wrap) {
    sc_in<bool>      SC_NAMED(clk);
    sc_in<bool>      SC_NAMED(rst_n);

    sc_out<bool>     SC_NAMED(in1_rdy);
    sc_in<bool>      SC_NAMED(in1_vld);
    sc_in<uint32_t>  SC_NAMED(in1_dat);

    sc_in<bool>      SC_NAMED(out1_rdy);
    sc_out<bool>     SC_NAMED(out1_vld);
    sc_out<uint32_t> SC_NAMED(out1_dat);

    dut SC_NAMED(dut_);

    // Companion clock: must match the Verilog clock (period and first posedge).
    sc_clock clk_companion;

    dut_wrap(const sc_module_name& nm, const sc_time& period, const sc_time& first_posedge)
        : sc_module(nm)
        , clk_companion("clk_companion", period, 0.5, first_posedge, true)
        , period_(period)
        , first_posedge_(first_posedge)
    {
        dut_.clk(clk);
        dut_.rst_n(rst_n);

        dut_.in1.rdy(in1_rdy);
        dut_.in1.vld(in1_vld);
        dut_.in1.dat(in1_dat);

        dut_.out1.rdy(out1_rdy);
        dut_.out1.vld(out1_vld);
        dut_.out1.dat(out1_dat);

        SC_METHOD(clk_check);
        sensitive << clk.pos();
        dont_initialize();
    }

    // The clk pin is only bound to the Verilog signal once elaboration is
    // done, so the alias is registered here (the sc_hls manager starts later,
    // at the first reset_pop()/reset_push()).
    void end_of_elaboration() override {
        if (first_posedge_ % period_ != SC_ZERO_TIME) {
            SC_REPORT_ERROR("SC_HLS-WRAP",
                (std::string("clock phase not supported: first posedge ") +
                 first_posedge_.to_string() + " is not a multiple of the period " +
                 period_.to_string() + " (the sc_hls scheduler runs at k*period)").c_str());
        }
        add_clock_alias(clk_companion, *clk.operator->());
    }

    // Every Verilog posedge must coincide with a companion-clock posedge.
    void clk_check() {
        const sc_time now = sc_time_stamp();
        if (now < first_posedge_ || (now - first_posedge_) % period_ != SC_ZERO_TIME) {
            SC_REPORT_ERROR("SC_HLS-WRAP",
                (std::string("clock mismatch: Verilog clk posedge at ") + now.to_string() +
                 " is not a posedge of " + clk_companion.name() +
                 " (period " + period_.to_string() +
                 ", first posedge " + first_posedge_.to_string() + ")").c_str());
        }
    }

private:
    sc_time period_;
    sc_time first_posedge_;
};
