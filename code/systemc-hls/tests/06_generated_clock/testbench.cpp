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

#include "top.h"
#include <memory>
#include <string>
#include <vector>

struct CapturedError {
    std::string msg_type;
    std::string msg;
    sc_severity severity;
};

std::vector<CapturedError> captured_errors;

void custom_report_handler(const sc_report& report, const sc_actions& actions) {
    if (report.get_severity() == SC_FATAL || report.get_severity() == SC_ERROR) {
        CapturedError err;
        err.msg_type = report.get_msg_type();
        err.msg = report.get_msg();
        err.severity = report.get_severity();
        captured_errors.push_back(err);
    }
    sc_report_handler::default_handler(report, actions);
}

int sc_main(int argc, char** argv) {
    sc_report_handler::set_handler(custom_report_handler);
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_FATAL, SC_DISPLAY | SC_THROW);

    auto top = std::make_shared<Top>("top");

    try {
        sc_start(20, SC_NS);
    } catch (const sc_report& r) {
    } catch (...) {
    }

    bool pass = false;
    for (const auto& err : captured_errors) {
        if (err.msg_type == "SC_HLS" && err.severity == SC_ERROR &&
            err.msg.find("No clocks found") != std::string::npos) {
            pass = true;
            break;
        }
    }

    if (pass) {
        std::cout << "PASS: SC_HLS \"No clocks found\" error detected" << std::endl;
        return 0;
    }

    std::cout << "FAIL: Expected SC_HLS \"No clocks found\" error" << std::endl;
    return 1;
}
