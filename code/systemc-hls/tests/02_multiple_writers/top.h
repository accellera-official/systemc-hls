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
 * Top Module for Multiple Writers Test
 *
 * This module demonstrates the VIOLATION:
 * - A channel is bound to a submodule's msg_out_if port (Producer.out)
 * - The parent module ALSO has a process writing to the same channel
 *
 * This creates multiple writers to the same channel, which should be detected
 * and flagged as an error (SC_HLS-102).
 */

#ifndef TOP_H
#define TOP_H

#include <systemc-hls>
#include "dut.h"
#include <systemc.h>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(Top) {
    sc_clock clk;
    sc_signal<bool> rst_bar;

    Producer SC_NAMED(producer);
    msg_channel<sc_uint<16>> SC_NAMED(chan);

    SC_CTOR(Top) : clk("clk", 10, SC_NS) {
        // Bind submodule's output port to channel
        producer.clk(clk);
        producer.rst_bar(rst_bar);
        producer.out(chan); // Producer writes to chan

        // Parent module also has a process that writes to the same channel
        // This is the VIOLATION!
        SC_CTHREAD(parent_writer, clk);
        async_reset_signal_is(rst_bar, false);

        // Add a reader to consume data and show the conflict
        SC_CTHREAD(reader, clk);
        async_reset_signal_is(rst_bar, false);

        SC_CTHREAD(reset_proc, clk);
    }

    // VIOLATION: Parent process writing to same channel that submodule writes to
    void parent_writer() {
        chan.reset_push(); // This should trigger SC_HLS-102 error
        wait();

        for (int i = 1; i <= 3; i++) {
            chan.push(i * 100);
            std::cout << "Parent: Pushed " << (i * 100) << std::endl;
        }
    }

    // Reader to consume data and show the interleaving/conflict
    void reader() {
        chan.reset_pop();
        wait();

        for (int i = 0; i < 8; i++) // Read 8 values (5 from producer + 3 from parent)
        {
            sc_uint<16> val = chan.pop();
            std::cout << "Reader: Received " << val << " at " << sc_time_stamp() << std::endl;
        }

        std::cout
            << "\n=== Test completed - Multiple writers were able to write to same channel ==="
            << std::endl;
        sc_stop();
    }

    void reset_proc() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};

#endif // TOP_H