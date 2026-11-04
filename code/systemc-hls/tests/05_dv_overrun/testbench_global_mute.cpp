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
 * test_dv_overrun_global_mute
 *
 * Configuration:
 *   - Global default = dv_overrun_policy::Mute.
 *   - No per-port override.
 *   - Producer pushes every cycle, consumer is II=3 → overruns occur but
 *     should be silenced.
 *
 * Pass condition:
 *   Zero SC_HLS-401 reports of any severity. Demonstrates that
 *   dv_overrun_policy::Mute fully suppresses overrun reporting and that
 *   ports without an explicit override follow the global default.
 */

#include "common.h"

int sc_main(int argc, char** argv) {
    install_capturing_handler();
    clear_captured();

    sc_hls::msg_lib::set_default_dv_overrun_policy(sc_hls::msg_lib::dv_overrun_policy::Mute);

    auto top = std::make_shared<Top>("top");

    sc_start();

    int errors   = count_401_errors();
    int warnings = count_401_warnings();
    int total    = count_401_total();

    std::cout << "[global_mute] SC_HLS errors=" << errors
              << " warnings=" << warnings << " total=" << total << std::endl;

    if (total == 0) {
        std::cout << "PASS: global Mute policy suppresses all overrun reports"
                  << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected 0 SC_HLS reports, got " << total
              << std::endl;
    return 1;
}