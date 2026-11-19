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
 * direct_input_sync Example - Mixed Protocol Usage
 *
 * This example demonstrates the use of multiple sc_hls msg_lib protocols in a
 * single design:
 * - msg_in/msg_out: Message ports (dat/vld/rdy) for flow-controlled data transfer
 * - sync_in: Rendezvous synchronization port for explicit cycle-accurate sync
 * - sc_in: Direct SystemC inputs (not part of sc_hls, but commonly mixed)
 *
 * The design shows how:
 * 1. sync_in provides a synchronization barrier before processing begins
 * 2. msg_in ports transfer data with back-pressure support
 * 3. Direct sc_in ports provide configuration/control values
 * 4. msg_out port sends results with flow control
 *
 * This pattern is useful for designs that need explicit synchronization points
 * combined with data streaming and configuration inputs.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;


SC_MODULE(dut)
{
    static constexpr int num_direct_inputs = 8;
    static constexpr int num_samples = 3;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_out<uint32_t> SC_NAMED(out1);
    sc_vector<msg_in<uint32_t>> SC_NAMED(sample_in);
    sync_in<> SC_NAMED(sync_port);
    sc_vector<sc_in<uint32_t>> SC_NAMED(direct_inputs);

    SC_CTOR(dut) {
        sample_in.init(num_samples);
        direct_inputs.init(num_direct_inputs);

        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        out1.reset_push();
        sync_port.reset_pop();
        for (int i = 0; i < num_samples; i++) {
            sample_in[i].reset_pop();
        }
        wait(); // reset state
    }

    void main() {
        reset();
        while (1) {
            sync_port.sync();

            // HLS PIPELINE II=1
            for (uint32_t x = 0; x < direct_inputs[0]; x++) {
                for (uint32_t y = 0; y < direct_inputs[1]; y++) {
                    uint32_t sum = 0;
                    for (uint32_t s = 0; s < num_samples; s++) // HLS UNROLL
                    {
                        sum += sample_in[s].pop() * direct_inputs[2 + s];
                    }
                    uint32_t rslt = 0;
                    rslt = sum * 10;
                    if (rslt > direct_inputs[7]) {
                        out1.push(rslt);
                    }
                }
            }
        }
    }
};