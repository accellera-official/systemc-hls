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
 * enable_port_access Example - DUT (manual msg passthrough)
 *
 * Demonstrates msg_in::enable_port_access() / msg_out::enable_port_access():
 * a customer process that drives the msg pins (vld/rdy/dat) directly instead
 * of calling push()/pop().
 *
 * Both ports call enable_port_access() in the constructor (elaboration phase;
 * it is an error to call it later). In this mode:
 *   - push() / pop() / *_nb() are forbidden (report an SC_HLS error),
 *   - the reset functions reset_pop()/reset_push() are ALSO forbidden (report an
 *     SC_HLS error) — the user process owns the pins, including idle values,
 *
 * A single process `proc` maintains both handshakes as a registered 1-deep
 * passthrough:
 *   - input side (DUT is consumer):  assert in.rdy when it has room; a transfer
 *     occurs on a cycle where in.vld && in.rdy.
 *   - output side (DUT is producer): assert out.vld while holding a value; a
 *     transfer occurs on a cycle where out.vld && out.rdy.
 *
 * rdy/vld/dat are REGISTERED (updated only at the clock edge) and tracked in
 * rdy_reg/vld_reg so they stay consistent with the idle values driven in
 * reset().
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(in);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(dut) {
        // Opt in to direct pin access on both ports (elaboration phase only).
        in.enable_port_access();
        out.enable_port_access();

        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in.rdy.write(false);
        out.vld.write(false);
        wait();
    }

    void proc() {
        reset();

        bool have = false;
        sc_uint<16> hold = 0;
        bool rdy_reg = false;   // last value driven to in.rdy
        bool vld_reg = false;   // last value driven to out.vld

        while (1) {
            // Complete handshakes based on what we actually drove last cycle.
            if (vld_reg && out.rdy.read()) {
                have = false;               // output beat was consumed
            }
            if (rdy_reg && in.vld.read()) {
                hold = in.dat.read();       // input beat accepted
                have = true;
            }

            // Recompute registered outputs for the next edge.
            rdy_reg = !have;                // ready when we have room
            vld_reg = have;                 // valid while holding a value
            in.rdy.write(rdy_reg);
            out.vld.write(vld_reg);
            out.dat.write(hold);

            wait();
        }
    }
};
