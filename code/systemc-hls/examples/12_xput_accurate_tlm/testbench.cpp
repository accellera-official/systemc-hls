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
 * TLM Fast Simulation Testbench
 *
 * This testbench demonstrates:
 *   - The TLM view for fast functional simulation (tlm_view_tag)
 *   - Configurable FIFO depths (depth=20)
 *   - Sending 40 values through the design (more than the default of 10)
 *   - The "structured testbench" pattern: top.h declares templated `driver`
 *     and `monitor` modules instantiated as `drv_` / `mon_` inside the
 *     templated `testbench`
 *
 * Note: VCD tracing is not meaningful for TLM mode. TLM channels use
 * tlm::tlm_fifo internally and have no signals to record. We deliberately
 * skip `sc_create_vcd_trace_file` / `trace_hierarchy` here.
 */

#include "timeout_thread.h"
#include "testbench.h"
#include <memory>

int sc_main(int argc, char** argv) {
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);

    auto tb_top = std::make_shared<testbench>("tb_top");

    // Longer timeout in TLM mode since the simulation processes more data.
    add_timeout_thread(2000, SC_NS, 100, SC_NS);

    // No VCD: TLM channels have no signal-level visibility.

    sc_start();

    if ((!tb_top->mon_.test_passed) ||
        (sc_report_handler::get_count(SC_ERROR) > 0) ||
        (sc_report_handler::get_count(SC_FATAL) > 0))
    {
        std::cout << "Simulation FAILED" << std::endl;
        return -1;
    }
    std::cout << "Simulation PASSED" << std::endl;
    return 0;
}