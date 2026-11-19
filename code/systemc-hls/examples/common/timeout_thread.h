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
 * Timeout Thread Utility
 *
 * Provides a reusable timeout mechanism for testbenches to prevent
 * infinite hangs during simulation. The timeout thread pings periodically
 * and stops simulation with an error if the timeout is reached.
 *
 * Usage in testbench:
 *   #include "timeout_thread.h"
 *
 *   int sc_main(int argc, char **argv) {
 *       // ... setup code ...
 *
 *       // Add timeout (e.g., 500ns with 100ns ping interval)
 *       add_timeout_thread(500, SC_NS, 100, SC_NS);
 *
 *       sc_start();
 *       // ... rest of code ...
 *   }
 */

#pragma once

#include <iostream>
#include <systemc.h>

// Timeout thread function
inline void timeout_thread_func(sc_time timeout, sc_time ping_interval) {
    sc_time elapsed = SC_ZERO_TIME;
    int ping_count = 0;
    int max_pings = static_cast<int>(timeout / ping_interval);

    while (elapsed < timeout) {
        wait(ping_interval);
        elapsed += ping_interval;
        ping_count++;

        std::cout << "(" << sc_time_stamp() << "): Timeout ping " << ping_count << "/" << max_pings
                  << std::endl;
    }

    SC_REPORT_ERROR("TIMEOUT", ("Test timed out after " + timeout.to_string() +
                                " - simulation appears to be hanging")
                                   .c_str());
    sc_stop();
}

// Helper function to add timeout thread
inline void add_timeout_thread(double timeout_value, sc_time_unit timeout_unit, double ping_value,
                               sc_time_unit ping_unit) {
    sc_time timeout(timeout_value, timeout_unit);
    sc_time ping_interval(ping_value, ping_unit);

    sc_spawn(sc_bind(&timeout_thread_func, timeout, ping_interval), "timeout_thread");
}