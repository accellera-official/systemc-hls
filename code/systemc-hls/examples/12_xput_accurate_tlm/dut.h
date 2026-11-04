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
 * TLM Fast Simulation Example - DUT
 *
 * This example demonstrates the TLM (Transaction Level Modeling) view for
 * fast functional simulation using the sc_hls library.
 *
 * Key features:
 *   - msg_in<T, ViewTag>:  parameterized msg input port
 *   - msg_out<T, ViewTag>: parameterized msg output port
 *   - When ViewTag = tlm_view_tag the channels use tlm::tlm_fifo internally
 *     (fast functional simulation, not cycle-accurate)
 *   - When ViewTag = sim_view_tag (default) the channels use signals
 *
 * Templating the DUT on `ViewTag` lets the same source serve both fast
 * functional simulation and cycle-accurate validation.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

template <typename ViewTag = default_view_tag>
SC_MODULE(dut) {
public:
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>, ViewTag> SC_NAMED(in1);
    msg_in<sc_uint<16>, ViewTag> SC_NAMED(in2);
    msg_in<sc_uint<16>, ViewTag> SC_NAMED(in3);
    msg_out<sc_uint<32>, ViewTag> SC_NAMED(out1);
    msg_out<sc_uint<32>, ViewTag> SC_NAMED(out2);

    SC_CTOR(dut) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        in2.reset_pop();
        in3.reset_pop();
        out1.reset_push();
        out2.reset_push();
        wait();
    }

    void proc() {
        reset();
        while (1) {
            auto i1 = in1.pop();
            auto i2 = in2.pop();
            auto i3 = in3.pop();
            auto v1 = i1 * i2;
            auto v2 = v1 * i3;
            out1.push(v1);
            out2.push(v2);
        }
    }
};