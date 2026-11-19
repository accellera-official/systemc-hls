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
 * common.h — shared harness for tests/05_dv_overrun.
 *
 * Provides:
 *   - A captured-error vector keyed by (msg_type, severity).
 *   - install_capturing_handler(): installs a custom sc_report_handler that
 *     records reports without aborting/throwing on SC_ERROR, so the rest of
 *     the simulation can run to completion and we can inspect the count.
 *   - count_401_errors() / count_401_warnings(): convenience accessors.
 *   - Top: a tight stim that pushes every cycle for ~30 cycles, then sc_stop().
 *
 * Each per-sub-test cpp file:
 *   1) Optionally calls sc_hls::msg_lib::set_default_dv_overrun_policy(...).
 *   2) Constructs Top.
 *   3) Optionally calls top->dut1.in1.set_overrun_policy(...).
 *   4) Calls install_capturing_handler() then sc_start().
 *   5) Inspects the captured counts and prints PASS:/FAIL:.
 */

#pragma once

#include "dut.h"
#include "sysc/utils/sc_report.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// Captured-error harness
// ============================================================================

struct CapturedReport {
    std::string msg_type;
    sc_core::sc_severity severity;
};

inline std::vector<CapturedReport>& captured_reports() {
    static std::vector<CapturedReport> v;
    return v;
}

inline void clear_captured() {
    captured_reports().clear();
}

inline void capturing_report_handler(const sc_core::sc_report& report,
                                     const sc_core::sc_actions& actions) {
    CapturedReport rec;
    rec.msg_type = report.get_msg_type() ? report.get_msg_type() : "";
    rec.severity = report.get_severity();
    captured_reports().push_back(rec);

    // Display the report but suppress abort/throw so the simulation can
    // continue running and accumulate further events.
    sc_core::sc_actions effective = actions
                                    & ~sc_core::SC_ABORT
                                    & ~sc_core::SC_THROW;
    sc_core::sc_report_handler::default_handler(report, effective);
}

inline void install_capturing_handler() {
    sc_core::sc_report_handler::set_handler(capturing_report_handler);
    // We want SC_ERROR to be displayed but not to kill the sim.
    sc_core::sc_report_handler::set_actions(
        sc_core::SC_ERROR, sc_core::SC_DISPLAY);
    sc_core::sc_report_handler::set_actions(
        "/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);
}

inline int count_401_errors() {
    int n = 0;
    for (const auto& r : captured_reports()) {
        if (r.msg_type == "SC_HLS-401" &&
            r.severity == sc_core::SC_ERROR) {
            ++n;
        }
    }
    return n;
}

inline int count_401_warnings() {
    int n = 0;
    for (const auto& r : captured_reports()) {
        if (r.msg_type == "SC_HLS-401" &&
            r.severity == sc_core::SC_WARNING) {
            ++n;
        }
    }
    return n;
}

inline int count_401_total() {
    int n = 0;
    for (const auto& r : captured_reports()) {
        if (r.msg_type == "SC_HLS-401" && r.severity!=sc_core::SC_INFO) ++n;
    }
    return n;
}

// ============================================================================
// Top: dut + tight producer that drives vld every cycle
// ============================================================================

SC_MODULE(Top) {
public:
    dut SC_NAMED(dut1);

    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    msg_dv_channel<unsigned> SC_NAMED(in_chan);
    msg_dv_channel<unsigned> SC_NAMED(out_chan);

    SC_CTOR(Top) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
        dut1.clk(clk);
        dut1.rst_bar(rst_bar);
        dut1.in1(in_chan);
        dut1.out1(out_chan);

        SC_CTHREAD(reset, clk);

        SC_THREAD(stim);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(drain);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst_bar, false);
    }

    // Producer: pushes every cycle for 30 iterations.
    // Combined with the dut's II=3 consumer this guarantees overruns.
    void stim() {
        in_chan.reset_push();
        wait();
        for (unsigned i = 0; i < 30; i++) {
            in_chan.push(i);
            wait();
        }
        // Give the consumer time to drain anything still in flight,
        // then stop the simulation.
        wait(30);
        sc_stop();
        wait();
    }

    // Drain output so out_buf doesn't back up. We don't check values here;
    // the test only cares about overrun reports on the input side.
    void drain() {
        out_chan.reset_pop();
        wait();
        while (true) {
            unsigned v;
            (void)out_chan.pop_nb(v);
            wait();
        }
    }

    void reset() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};