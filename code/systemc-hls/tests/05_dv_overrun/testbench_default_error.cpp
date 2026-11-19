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
 * test_dv_overrun_default_error
 *
 * Configuration:
 *   - Global default explicitly set to dv_overrun_policy::Error
 *     (library default is Mute).
 *   - Producer pushes every cycle, consumer is II=3 → guaranteed overruns.
 *
 * Pass condition:
 *   ≥ 1 SC_ERROR with msg_type "SC_HLS-401" was reported.
 */

#include "common.h"

int sc_main(int argc, char** argv) {
    install_capturing_handler();
    clear_captured();

    // Library default is dv_overrun_policy::Mute; select Error globally.
    sc_hls::msg_lib::set_default_dv_overrun_policy(sc_hls::msg_lib::dv_overrun_policy::Error);

    auto top = std::make_shared<Top>("top");

    sc_start();

    int errors   = count_401_errors();
    int warnings = count_401_warnings();
    int total    = count_401_total();

    std::cout << "[default_error] SC_HLS errors=" << errors
              << " warnings=" << warnings << " total=" << total << std::endl;

    if (errors >= 1) {
        std::cout << "PASS: global Error policy correctly raises SC_ERROR on overrun"
                  << std::endl;
        return 0;
    }
    std::cout << "FAIL: expected at least one SC_ERROR SC_HLS, got "
              << errors << std::endl;
    return 1;
}