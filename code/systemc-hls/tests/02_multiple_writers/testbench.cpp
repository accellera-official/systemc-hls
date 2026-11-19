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
 * Testbench for Multiple Writers Detection
 *
 * This test verifies that the sc_hls library correctly detects and reports
 * the error when multiple writers attempt to write to the same channel:
 * - A submodule's msg_out_if port bound to a channel
 * - A parent process also writing to the same channel
 *
 * Expected: SC_HLS-102 error should be reported
 */

#include "timeout_thread.h"
#include "top.h"
#include <systemc.h>

int sc_main(int argc, char** argv) {
    std::cout << "=== Multiple Writers Detection Test ===" << std::endl;
    std::cout << "Expected: SC_HLS-102 error when multiple writers detected" << std::endl;
    std::cout << std::endl;

    // Add timeout to prevent infinite hangs (500ns timeout, 100ns ping interval)
    add_timeout_thread(500, SC_NS, 100, SC_NS);

    Top top("top");

    // Start simulation - should detect the violation during reset_push
    sc_start();

    std::cout << "\nTest completed" << std::endl;
    return 0;
}