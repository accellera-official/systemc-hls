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
 * Top Module for Multi-Process Access Test
 *
 * Instantiates TestModule and provides:
 * - Clock and reset generation (5-cycle reset, so multi-cycle reset tests
 *   will call Reset 5 times from the same process)
 * - A msg_channel to bind to the TestModule's port
 * - A helper process on the opposite side of the channel (writer for msg_in_if
 *   tests, reader for msg_out_if tests) so the channel is fully connected
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

    TestModule SC_NAMED(test_module);
    msg_channel<sc_uint<16>> SC_NAMED(chan);

    SC_CTOR(Top) : clk("clk", 10, SC_NS) {
        test_module.clk(clk);
        test_module.rst_bar(rst_bar);

        // Bind the TestModule's port to the channel
#if defined(TEST_OUT_PUSH) || defined(TEST_OUT_PUSHNB) || defined(TEST_MULTI_RESET_OUT)
        test_module.out(chan);
#elif defined(TEST_IN_POP) || defined(TEST_IN_POPNB) || defined(TEST_IN_PEEKNB) ||                 \
    defined(TEST_MULTI_RESET_IN)
        test_module.in(chan);
#endif

        // Reset process: holds rst_bar=0 for 5 clock cycles, then deasserts.
        // This means any process with async_reset_signal_is(rst_bar, false)
        // will have its reset body executed 5 times (multi-cycle reset).
        SC_CTHREAD(reset_proc, clk);

        // Helper process on the opposite side of the channel.
        // For msg_out_if tests: a reader to consume data pushed by TestModule.
        // For msg_in_if tests: a writer to drive data into the channel.
#if defined(TEST_OUT_PUSH) || defined(TEST_OUT_PUSHNB) || defined(TEST_MULTI_RESET_OUT)
        SC_CTHREAD(helper_reader, clk);
        async_reset_signal_is(rst_bar, false);
#elif defined(TEST_IN_POP) || defined(TEST_IN_POPNB) || defined(TEST_IN_PEEKNB) ||                 \
    defined(TEST_MULTI_RESET_IN)
        SC_CTHREAD(helper_writer, clk);
        async_reset_signal_is(rst_bar, false);
#endif
    }

    // Reset process: deasserts reset after 5 clock cycles.
    // Not sensitive to rst_bar itself - just drives it.
    void reset_proc() {
        rst_bar.write(false);
        wait(5);
        rst_bar.write(true);
        wait();
    }

#if defined(TEST_OUT_PUSH) || defined(TEST_OUT_PUSHNB) || defined(TEST_MULTI_RESET_OUT)
    // Helper reader: consumes data from the channel so push() doesn't block.
    void helper_reader() {
        chan.reset_pop();
        wait();
        while (true) {
            chan.pop();
        }
    }
#endif

#if defined(TEST_IN_POP) || defined(TEST_IN_POPNB) || defined(TEST_IN_PEEKNB) ||                   \
    defined(TEST_MULTI_RESET_IN)
    // Helper writer: drives data into the channel so pop() has data available.
    void helper_writer() {
        chan.reset_push();
        wait();
        int i = 0;
        while (true) {
            chan.push(i++);
        }
    }
#endif
};

#endif // TOP_H