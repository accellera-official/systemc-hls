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
 * Latency Annotation Example
 *
 * This example demonstrates HLS latency back-annotation on a divergence /
 * reconvergence pipeline. It shows how to track data flow through divergent
 * and reconvergent paths to analyze pipeline behavior and latency.
 *
 * Architecture:
 *   din -> divergence -> [block0, block1] -> reconvergence -> dout
 *
 * The divergence module splits data to two parallel paths (block0 and block1),
 * which then reconverge to produce the final output. The two parallel paths
 * are individually annotated with a 3-cycle pipeline latency at simulation
 * start; see testbench.cpp for the four annotation methods.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

// Divergence module: splits input to two outputs.
SC_MODULE(divergence) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(din);
    msg_out<sc_uint<16>> SC_NAMED(dout0);
    msg_out<sc_uint<16>> SC_NAMED(dout1);

    SC_CTOR(divergence) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        din.reset_pop();
        dout0.reset_push();
        dout1.reset_push();
        wait();
    }

    void run() {
        reset();
        while (1) {
            auto tmp = din.pop();
            dout0.push(tmp);
            dout1.push(tmp);
        }
    }
};

// Block0: processes data from path 0.
SC_MODULE(block0) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(din);
    msg_out<sc_uint<16>> SC_NAMED(dout);

    SC_CTOR(block0) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        din.reset_pop();
        dout.reset_push();
        wait();
    }

    void run() {
        reset();
        while (1) {
            auto tmp = din.pop();
            dout.push(tmp);
        }
    }
};

// Block1: processes data from path 1.
SC_MODULE(block1) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(din);
    msg_out<sc_uint<16>> SC_NAMED(dout);

    SC_CTOR(block1) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        din.reset_pop();
        dout.reset_push();
        wait();
    }

    void run() {
        reset();
        while (1) {
            auto tmp = din.pop();
            dout.push(tmp);
        }
    }
};

// Reconvergence module: combines two inputs to one output.
SC_MODULE(reconvergence) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(din0);
    msg_in<sc_uint<16>> SC_NAMED(din1);
    msg_out<sc_uint<16>> SC_NAMED(dout);

    SC_CTOR(reconvergence) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        din0.reset_pop();
        din1.reset_pop();
        dout.reset_push();
        wait();
    }

    void run() {
        reset();
        while (1) {
            auto tmp0 = din0.pop();
            auto tmp1 = din1.pop();
            dout.push(tmp0 + tmp1);
        }
    }
};

// dut_top: top-level DUT module that wires the divergence/reconvergence pipeline.
SC_MODULE(dut_top) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(din);
    msg_out<sc_uint<16>> SC_NAMED(dout);

    // Sub-module instances
    divergence SC_NAMED(divergence_inst);
    block0 SC_NAMED(block0_inst);
    block1 SC_NAMED(block1_inst);
    reconvergence SC_NAMED(reconvergence_inst);

    // Internal channels
    msg_channel<sc_uint<16>> SC_NAMED(dout0);
    msg_channel<sc_uint<16>> SC_NAMED(dout1);
    msg_channel<sc_uint<16>> SC_NAMED(bout0);
    msg_channel<sc_uint<16>> SC_NAMED(bout1);

    SC_CTOR(dut_top) {
        divergence_inst.clk(clk);
        divergence_inst.rst_bar(rst_bar);
        divergence_inst.din(din);
        divergence_inst.dout0(dout0);
        divergence_inst.dout1(dout1);

        block0_inst.clk(clk);
        block0_inst.rst_bar(rst_bar);
        block0_inst.din(dout0);
        block0_inst.dout(bout0);

        block1_inst.clk(clk);
        block1_inst.rst_bar(rst_bar);
        block1_inst.din(dout1);
        block1_inst.dout(bout1);

        reconvergence_inst.clk(clk);
        reconvergence_inst.rst_bar(rst_bar);
        reconvergence_inst.din0(bout0);
        reconvergence_inst.din1(bout1);
        reconvergence_inst.dout(dout);
    }
};