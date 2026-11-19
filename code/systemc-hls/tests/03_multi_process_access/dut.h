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
 * DUT for Multi-Process Access Test
 *
 * Tests two scenarios:
 *
 * 1. ERROR TESTS: Two processes in the same module, one owns the port
 *    (called Reset), the other tries to call an API function.
 *    Expected: SC_HLS-102 error.
 *
 * 2. SUCCESS TESTS: One process calls Reset multiple times (multi-cycle
 *    reset pattern - common in real designs). Expected: no error.
 *
 * Compile-time selection via defines:
 *   Error tests:   TEST_OUT_PUSH, TEST_OUT_PUSHNB
 *                  TEST_IN_POP, TEST_IN_POPNB, TEST_IN_PEEKNB
 *   Success tests: TEST_MULTI_RESET_OUT, TEST_MULTI_RESET_IN
 */

#ifndef DUT_H
#define DUT_H

#include <systemc-hls>
#include <systemc.h>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

// ============================================================================
// ERROR TESTS: msg_out_if - wrong process calls push() or push_nb()
// ============================================================================

#if defined(TEST_OUT_PUSH) || defined(TEST_OUT_PUSHNB)

SC_MODULE(TestModule) {
    sc_in<bool> clk;
    sc_in<bool> rst_bar;
    msg_out<sc_uint<16>> out;

    SC_CTOR(TestModule) {
        SC_CTHREAD(proc_owner, clk.pos());
        async_reset_signal_is(rst_bar, false);

        SC_CTHREAD(proc_violator, clk.pos());
        async_reset_signal_is(rst_bar, false);
    }

    // Process A: Calls reset_push() and becomes the owning process.
    void proc_owner() {
        out.reset_push(); // Registers this process as owner
        wait();
        // Normal operation - hold the port, do nothing
        while (true) {
            wait();
        }
    }

    // Process B: Tries to call an API function without being the owner.
    // This should trigger SC_HLS-102.
    void proc_violator() {
        wait(); // End of reset body - wait for reset to deassert
        // Wait a couple extra cycles to ensure proc_owner has registered
        wait(2);
#if defined(TEST_OUT_PUSH)
        out.push(42); // VIOLATION: wrong process calling push()
#elif defined(TEST_OUT_PUSHNB)
        out.push_nb(42); // VIOLATION: wrong process calling push_nb()
#endif
    }
};

    // ============================================================================
    // ERROR TESTS: msg_in_if - wrong process calls pop(), pop_nb(), or peek_nb()
    // ============================================================================

#elif defined(TEST_IN_POP) || defined(TEST_IN_POPNB) || defined(TEST_IN_PEEKNB)

SC_MODULE(TestModule) {
    sc_in<bool> clk;
    sc_in<bool> rst_bar;
    msg_in<sc_uint<16>> in;

    SC_CTOR(TestModule) {
        SC_CTHREAD(proc_owner, clk.pos());
        async_reset_signal_is(rst_bar, false);

        SC_CTHREAD(proc_violator, clk.pos());
        async_reset_signal_is(rst_bar, false);
    }

    // Process A: Calls reset_pop() and becomes the owning process.
    void proc_owner() {
        in.reset_pop(); // Registers this process as owner
        wait();
        // Normal operation - hold the port, do nothing
        while (true) {
            wait();
        }
    }

    // Process B: Tries to call an API function without being the owner.
    // This should trigger SC_HLS-102.
    void proc_violator() {
        wait(); // End of reset body - wait for reset to deassert
        // Wait a couple extra cycles to ensure proc_owner has registered
        wait(2);
#if defined(TEST_IN_POP)
        in.pop(); // VIOLATION: wrong process calling pop()
#elif defined(TEST_IN_POPNB)
        sc_uint<16> val;
        in.pop_nb(val); // VIOLATION: wrong process calling pop_nb()
#elif defined(TEST_IN_PEEKNB)
        sc_uint<16> val;
        in.peek_nb(val); // VIOLATION: wrong process calling peek_nb()
#endif
    }
};

    // ============================================================================
    // SUCCESS TEST: Multi-cycle reset for msg_out_if
    //
    // The process calls reset_push() every clock cycle while in reset (rst_bar=0).
    // With async_reset_signal_is, the reset body (before the first wait()) is
    // re-executed each clock cycle while rst_bar=0. This means reset_push() is
    // called multiple times from the same process - this must NOT error out.
    // After reset deasserts (rst_bar=1), the process calls push() normally.
    // ============================================================================

#elif defined(TEST_MULTI_RESET_OUT)

SC_MODULE(TestModule) {
    sc_in<bool> clk;
    sc_in<bool> rst_bar;
    msg_out<sc_uint<16>> out;

    SC_CTOR(TestModule) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        // Reset body: called every clock cycle while rst_bar=0.
        // With a 5-cycle reset (see top.h reset_proc), reset_push() is
        // called 5 times from the same process. This must not error out.
        out.reset_push();
        wait();

        // After reset (rst_bar=1), use the port normally.
        out.push(100);
        std::cout << "SUCCESS: Multi-cycle reset for msg_out_if completed without error" << std::endl;
        sc_stop();
    }
};

    // ============================================================================
    // SUCCESS TEST: Multi-cycle reset for msg_in_if
    //
    // Same pattern as above but for the reader side.
    // reset_pop() is called multiple times from the same process during reset.
    // After reset, pop_nb() is called (non-blocking, returns false if no data).
    // ============================================================================

#elif defined(TEST_MULTI_RESET_IN)

SC_MODULE(TestModule) {
    sc_in<bool> clk;
    sc_in<bool> rst_bar;
    msg_in<sc_uint<16>> in;

    SC_CTOR(TestModule) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        // Reset body: called every clock cycle while rst_bar=0.
        // reset_pop() is called multiple times from the same process.
        in.reset_pop();
        wait();

        // After reset (rst_bar=1), use the port normally.
        sc_uint<16> val;
        bool got = in.pop_nb(val);
        std::cout << "SUCCESS: Multi-cycle reset for msg_in_if completed without error (got=" << got
                  << ")" << std::endl;
        sc_stop();
    }
};

#else
#error                                                                                             \
    "Must define one of: TEST_OUT_PUSH, TEST_OUT_PUSHNB, TEST_IN_POP, TEST_IN_POPNB, TEST_IN_PEEKNB, TEST_MULTI_RESET_OUT, TEST_MULTI_RESET_IN"
#endif

#endif // DUT_H