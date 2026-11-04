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
 * DUT for Multiple Writers Test
 *
 * This file contains a submodule with an msg_out_if port.
 * The parent module will incorrectly try to write to the same channel
 * that this submodule's port is bound to.
 */

#ifndef DUT_H
#define DUT_H

#include <systemc-hls>
#include <systemc.h>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

// Submodule that has an output port
SC_MODULE(Producer) {
    sc_in<bool> clk;
    sc_in<bool> rst_bar;
    msg_out<sc_uint<16>> out; // Output port

    SC_CTOR(Producer) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        out.reset_push();
        wait();

        // Write some data
        for (int i = 1; i <= 5; i++) {
            out.push(i * 10);
            std::cout << "Producer: Pushed " << (i * 10) << std::endl;
        }
    }
};

#endif // DUT_H