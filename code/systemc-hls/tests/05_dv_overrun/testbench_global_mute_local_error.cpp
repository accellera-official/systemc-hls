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
 * test_dv_overrun_global_mute_local_error
 *
 * Configuration:
 *   - Global default = dv_overrun_policy::Mute.
 *   - Per-port override on top.dut1.in1 (msg_dv_in) = dv_overrun_policy::Error.
 *   - Producer pushes every cycle, consumer is II=3 → guaranteed overruns.
 *
 * Pass condition:
 *   ≥ 1 SC_ERROR with msg_type "SC_HLS-401" was reported, demonstrating
 *   that the per-port override beats the global default.
 */

#include "common.h"

int sc_main(int argc, char** argv) {
    install_capturing_handler();
    clear_captured();

    // Globally silence msg_dv_in overruns…
    sc_hls::msg_lib::set_default_dv_overrun_policy(sc_hls::msg_lib::dv_overrun_policy::Mute);

    auto top = std::make_shared<Top>("top");

    // …but override Error on the actual reader: the msg_dv_in port inside the
    // dut. (It is the port — bound to the channel's vld/dat — that runs the
    // Pre() logic which detects overruns.)
    top->dut1.in1.set_overrun_policy(sc_hls::msg_lib::dv_overrun_policy::Error);

    sc_start();

    int errors   = count_401_errors();
    int warnings = count_401_warnings();
    int total    = count_401_total();

    std::cout << "[global_mute_local_error] SC_HLS errors=" << errors
              << " warnings=" << warnings << " total=" << total << std::endl;

    if (errors >= 1) {
        std::cout << "PASS: per-port Error override beats global Mute"
                  << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected at least one SC_ERROR SC_HLS, got "
              << errors << std::endl;
    return 1;
}