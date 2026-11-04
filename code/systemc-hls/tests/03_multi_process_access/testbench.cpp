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
 * Testbench for Multi-Process Access Test
 *
 * Compile with one of:
 *   -DTEST_OUT_PUSH      msg_out_if push() called from wrong process  -> SC_HLS-102
 *   -DTEST_OUT_PUSHNB    msg_out_if push_nb() called from wrong process -> SC_HLS-102
 *   -DTEST_IN_POP        msg_in_if pop() called from wrong process    -> SC_HLS-102
 *   -DTEST_IN_POPNB      msg_in_if pop_nb() called from wrong process  -> SC_HLS-102
 *   -DTEST_IN_PEEKNB     msg_in_if peek_nb() called from wrong process -> SC_HLS-102
 *   -DTEST_MULTI_RESET_OUT  Same process calls reset_push() 5x    -> no error
 *   -DTEST_MULTI_RESET_IN   Same process calls reset_pop() 5x     -> no error
 */

#include "timeout_thread.h"
#include "top.h"
#include <systemc.h>

int sc_main(int argc, char** argv) {
#if defined(TEST_OUT_PUSH)
    std::cout << "=== Test: msg_out_if push() from wrong process ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error" << std::endl;
#elif defined(TEST_OUT_PUSHNB)
    std::cout << "=== Test: msg_out_if push_nb() from wrong process ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error" << std::endl;
#elif defined(TEST_IN_POP)
    std::cout << "=== Test: msg_in_if pop() from wrong process ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error" << std::endl;
#elif defined(TEST_IN_POPNB)
    std::cout << "=== Test: msg_in_if pop_nb() from wrong process ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error" << std::endl;
#elif defined(TEST_IN_PEEKNB)
    std::cout << "=== Test: msg_in_if peek_nb() from wrong process ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error" << std::endl;
#elif defined(TEST_MULTI_RESET_OUT)
    std::cout << "=== Test: Multi-cycle reset for msg_out_if (same process calls reset_push 5x) ==="
              << std::endl;
    std::cout << "Expected: no error, clean exit" << std::endl;
#elif defined(TEST_MULTI_RESET_IN)
    std::cout << "=== Test: Multi-cycle reset for msg_in_if (same process calls reset_pop 5x) ==="
              << std::endl;
    std::cout << "Expected: no error, clean exit" << std::endl;
#endif

    // Timeout prevents infinite hangs (500ns timeout, 100ns ping interval)
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    Top top("top");
    sc_start();

    return 0;
}