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
 * Test: 04_incorrect_annotation
 *
 * Verifies that annotating a msg_channel (instead of the msg_out port bound to it)
 * produces a SC_HLS-ANNOTATE error at end of simulation.
 *
 * The test intentionally annotates 'top_inst.bout' (a msg_channel) rather than
 * 'top_inst.block_inst.dout' (the msg_out port that calls reset_push()). The
 * annotation is therefore inactive, and the validation logic in channel_manager
 * should detect this and report a SC_HLS-ANNOTATE error when sc_stop() is called.
 *
 * Expected result: SC_HLS-ANNOTATE error is reported → test PASSES.
 * If no error is reported → test FAILS.
 */

// SC_HLS_BACK_ANNOTATE must be defined before any sc_hls include.
// It is provided via -DSC_HLS_BACK_ANNOTATE in the Makefile.

#include "sc_hls_annotate.h"
#include "dut.h"
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// Custom error handler to capture SC_HLS-ANNOTATE errors
// ============================================================================

struct CapturedError {
    std::string msg_type;
    std::string msg;
    sc_severity severity;
};

std::vector<CapturedError> captured_errors;

void custom_report_handler(const sc_report& report, const sc_actions& actions) {
    if (report.get_severity() == SC_ERROR) {
        CapturedError err;
        err.msg_type = report.get_msg_type();
        err.msg = report.get_msg();
        err.severity = report.get_severity();
        captured_errors.push_back(err);
    }
    // Use default handler for display but suppress abort/throw on SC_ERROR
    sc_report_handler::default_handler(report, actions & ~SC_ABORT & ~SC_THROW);
}

int sc_main(int argc, char** argv) {
    // Install custom handler to capture errors without aborting
    sc_report_handler::set_handler(custom_report_handler);
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    // Allow SC_ERROR to be displayed but not abort
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);

    // Create design
    sc_clock clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true);
    sc_signal<bool> rstn("rstn");

    auto top_inst = std::make_shared<top>("top_inst");
    top_inst->clk(clk);
    top_inst->rstn(rstn);

    // =========================================================================
    // Intentionally annotate the msg_channel 'bout' instead of the msg_out port
    // 'block_inst.dout'. This is the incorrect target - the annotation will be
    // inert because Pre()/Post() are called on block_inst.dout, not on bout.
    // The validation logic should detect this and report SC_HLS-ANNOTATE.
    // =========================================================================
    top_inst->bout.set_latency_capacity(3, 3);

    // Drive reset
    rstn.write(false);

    // Run simulation for a few cycles (enough for reset and some activity)
    sc_start(10, SC_NS);

    // Explicitly stop simulation to trigger SC_PRE_STOP callback,
    // which invokes channel_manager::validate_annotations().
    sc_stop();

    // =========================================================================
    // Check that SC_HLS-ANNOTATE error was reported
    // =========================================================================
    bool found_annotation_error = false;
    for (const auto& err : captured_errors) {
        if (err.msg_type == "SC_HLS" && err.severity == SC_ERROR) {
            found_annotation_error = true;
            break;
        }
    }

    if (found_annotation_error) {
        std::cout << "PASS: SC_HLS error correctly detected for inactive annotation"
                  << std::endl;
        return 0;
    }

    std::cout << "FAIL: Expected SC_HLS error for annotation on inactive channel"
              << std::endl;
    return 1;
}