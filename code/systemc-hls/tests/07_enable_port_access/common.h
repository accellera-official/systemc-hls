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
 * common.h — shared harness for tests/07_enable_port_access.
 *
 * Provides a capturing sc_report_handler that records SC_ERROR / SC_FATAL
 * reports (msg_type + message text) without aborting or throwing, so the test
 * can inspect what was reported and decide PASS/FAIL.
 *
 * Each sub-test cpp:
 *   1) installs the capturing handler,
 *   2) builds a tiny design that misuses enable_port_access(),
 *   3) runs sc_start(),
 *   4) asserts that the expected SC_HLS error was captured.
 */

#pragma once

#include <systemc-hls>
#include <iostream>
#include <string>
#include <vector>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

struct CapturedReport {
    std::string msg_type;
    std::string msg;
    sc_core::sc_severity severity;
};

inline std::vector<CapturedReport>& captured_reports() {
    static std::vector<CapturedReport> v;
    return v;
}

inline void capturing_report_handler(const sc_core::sc_report& report,
                                     const sc_core::sc_actions& actions) {
    if (report.get_severity() == sc_core::SC_ERROR ||
        report.get_severity() == sc_core::SC_FATAL) {
        captured_reports().push_back(
            {report.get_msg_type() ? report.get_msg_type() : "",
             report.get_msg() ? report.get_msg() : "",
             report.get_severity()});
    }
    // Suppress abort/throw so the sim continues and we can inspect the counts.
    sc_core::sc_actions effective = actions
                                    & ~sc_core::SC_ABORT
                                    & ~sc_core::SC_THROW;
    sc_core::sc_report_handler::default_handler(report, effective);
}

inline void install_capturing_handler() {
    sc_core::sc_report_handler::set_handler(capturing_report_handler);
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated",
                                            sc_core::SC_DO_NOTHING);
}

// Returns true if any captured SC_HLS report's message contains `needle`.
inline bool saw_sc_hls_error_containing(const std::string& needle) {
    for (const auto& r : captured_reports()) {
        if (r.msg_type == "SC_HLS" && r.msg.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}
