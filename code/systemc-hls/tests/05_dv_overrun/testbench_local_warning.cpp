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
 * test_dv_overrun_local_warning
 *
 * Configuration:
 *   - Global default left at library default = dv_overrun_policy::Mute.
 *   - Per-port override on top.in_chan reader = dv_overrun_policy::Warning.
 *   - Producer pushes every cycle, consumer is II=3 → guaranteed overruns.
 *
 * Pass condition:
 *   ≥ 1 SC_WARNING with msg_type "SC_HLS-401", and zero SC_ERRORs.
 *   Demonstrates that the per-port Warning override beats the global Mute
 *   default and that the warning path actually fires.
 */

#include "common.h"

int sc_main(int argc, char** argv) {
    install_capturing_handler();
    clear_captured();

    // Global default remains dv_overrun_policy::Mute.

    auto top = std::make_shared<Top>("top");

    // Per-port: downgrade to Warning. The actual reader is the msg_dv_in port
    // inside the dut (bound to the channel's vld/dat); that's where Pre()
    // runs and detects overruns.
    top->dut1.in1.set_overrun_policy(sc_hls::msg_lib::dv_overrun_policy::Warning);

    sc_start();

    int errors   = count_401_errors();
    int warnings = count_401_warnings();
    int total    = count_401_total();

    std::cout << "[local_warning] SC_HLS-401 errors=" << errors
              << " warnings=" << warnings << " total=" << total << std::endl;

    if (warnings >= 1 && errors == 0) {
        std::cout << "PASS: per-port Warning override produces SC_WARNING and no SC_ERROR"
                  << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected ≥1 SC_WARNING and 0 SC_ERROR SC_HLS-401; got"
              << " errors=" << errors << " warnings=" << warnings << std::endl;
    return 1;
}