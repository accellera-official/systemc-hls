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
 * Test: 04_incorrect_annotation
 *
 * Verifies that annotating a msg_channel (instead of the msg_out port bound to it)
 * produces a SC_HLS-ANNOTATE error at end of simulation.
 *
 * Topology:
 *   block_inst.dout (msg_out) --> bout (msg_channel) --> reconvergence_inst.din (msg_in)
 *
 * The annotation is intentionally applied to 'bout' (the channel), not to
 * 'block_inst.dout' (the port that calls reset_push()). This should trigger
 * the validation error.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

// Simple producer module
SC_MODULE(block) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rstn);
    msg_out<sc_uint<16>> SC_NAMED(dout);

    SC_CTOR(block) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rstn, false);
    }

private:
    void run() {
        dout.reset_push();
        wait();
        while (1) {
            dout.push(42);
        }
    }
};

// Simple consumer module
SC_MODULE(consumer) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rstn);
    msg_in<sc_uint<16>> SC_NAMED(din);

    SC_CTOR(consumer) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rstn, false);
    }

private:
    void run() {
        din.reset_pop();
        wait();
        while (1) {
            din.pop();
        }
    }
};

// Top module: block_inst.dout is bound to the 'bout' channel
SC_MODULE(top) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rstn);

    SC_CTOR(top) {
        block_inst.clk(clk);
        block_inst.rstn(rstn);
        block_inst.dout(bout);

        consumer_inst.clk(clk);
        consumer_inst.rstn(rstn);
        consumer_inst.din.bind(bout);
    }

    block SC_NAMED(block_inst);
    consumer SC_NAMED(consumer_inst);
    msg_channel<sc_uint<16>> SC_NAMED(bout);
};