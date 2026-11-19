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
 * enable_port_access Example - portless channel (DUT)
 *
 * Demonstrates enable_port_access_reader() / enable_port_access_writer() on a
 * portless msg_channel. Because a channel inherits BOTH the reader and writer
 * protocols, the two disambiguated calls select which side is manual.
 *
 * One module, two processes, communicating through an internal (portless)
 * msg_channel `chan` whose handshake is driven by hand on both sides:
 *
 *     in (msg_in, normal) --> proc1 --> chan (manual) --> proc2 --> out (msg_out, normal)
 *
 *   - proc1 owns the channel WRITER side: it pops from `in` with the normal
 *     API, then drives chan.vld/chan.dat and reads chan.rdy directly.
 *   - proc2 owns the channel READER side: it drives chan.rdy and reads
 *     chan.vld/chan.dat directly, then pushes to `out` with the normal API.
 *
 * Only the portless channel is manual; the module's `in`/`out` ports use the
 * ordinary pop()/push() API. This isolates the channel-side reader/writer
 * split. Handshake pins are registered (updated at the clock edge); the VCD
 * cross-check in the README confirms cycle accuracy.
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

    // Internal portless channel with BOTH sides in port-access mode.
    msg_channel<sc_uint<16>> SC_NAMED(chan);

    SC_CTOR(dut) {
        // Enable manual pin access on each side of the portless channel
        // (elaboration phase only).
        chan.enable_port_access_writer();
        chan.enable_port_access_reader();

        SC_THREAD(proc1);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(proc2);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    // Reset for proc1: normal reader reset on `in`; manual writer idle on `chan`
    // (reset_push() is forbidden under port access).
    void reset1() {
        in.reset_pop();               // normal reader reset (auto-scheduled)
        chan.vld.write(false);        // manual channel writer idle
        wait();
    }

    // Stage 1: normal pop() from `in`, manual msg write to the channel.
    void proc1() {
        reset1();
        bool have = false;
        sc_uint<16> hold = 0;
        bool vld_reg = false;         // last value driven to chan.vld

        while (1) {
            // Complete the previous write handshake.
            if (vld_reg && chan.rdy.read()) {
                have = false;
            }
            // Fetch the next value when the holding register is free.
            if (!have) {
                if (in.pop_nb(hold)) {
                    have = true;
                }
            }
            // Drive registered writer pins for the next edge.
            vld_reg = have;
            chan.vld.write(vld_reg);
            chan.dat.write(hold);
            wait();
        }
    }

    // Reset for proc2: manual reader idle on `chan` (reset_pop() is forbidden
    // under port access); normal writer reset on `out`.
    void reset2() {
        chan.rdy.write(false);        // manual channel reader idle
        out.reset_push();             // normal writer reset (auto-scheduled)
        wait();
    }

    // Stage 2: manual msg read from the channel, normal push() to `out`.
    void proc2() {
        reset2();
        bool have = false;
        sc_uint<16> hold = 0;
        bool rdy_reg = false;         // last value driven to chan.rdy

        while (1) {
            // Accept a beat from the channel if we offered ready and it was valid.
            if (rdy_reg && chan.vld.read()) {
                hold = chan.dat.read();
                have = true;
            }
            // Forward a held value downstream (normal blocking push semantics
            // via non-blocking to keep this process's timing explicit).
            if (have) {
                if (out.push_nb(hold)) {
                    have = false;
                }
            }
            // Ready to accept next beat only when the holding register is free.
            rdy_reg = !have;
            chan.rdy.write(rdy_reg);
            wait();
        }
    }
};
