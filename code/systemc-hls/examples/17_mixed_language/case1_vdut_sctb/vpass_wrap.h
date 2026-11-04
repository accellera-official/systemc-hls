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
 * Case 1 wrapper: SystemC view of the Verilog DUT `vpass`.
 *
 * This is the wrapper an HLS / co-simulation flow would generate around RTL:
 * a pins-only sc_module (R1) whose port names are the RTL port names. It
 * holds the Verilated model and binds it port-to-port. There is no msg_in /
 * msg_out here -- the RTL has none, only the flattened modular-port pins.
 *
 * The SystemC testbench binds its msg_channel signals directly to these pins
 * (R8): the channel's DUT-side endpoint is never reset_pop()/reset_push()'d,
 * so the library does not drive those pins -- the Verilog does.
 */

#pragma once

#include <systemc>
#include <cstdint>
#include "Vvpass.h"

using namespace sc_core;

SC_MODULE(vpass_wrap) {
    sc_in<bool>      SC_NAMED(clk);
    sc_in<bool>      SC_NAMED(rst_n);

    sc_out<bool>     SC_NAMED(in1_rdy);
    sc_in<bool>      SC_NAMED(in1_vld);
    sc_in<uint32_t>  SC_NAMED(in1_dat);

    sc_in<bool>      SC_NAMED(out1_rdy);
    sc_out<bool>     SC_NAMED(out1_vld);
    sc_out<uint32_t> SC_NAMED(out1_dat);

    Vvpass SC_NAMED(rtl);

    SC_CTOR(vpass_wrap) {
        rtl.clk(clk);
        rtl.rst_n(rst_n);
        rtl.in1_rdy(in1_rdy);
        rtl.in1_vld(in1_vld);
        rtl.in1_dat(in1_dat);
        rtl.out1_rdy(out1_rdy);
        rtl.out1_vld(out1_vld);
        rtl.out1_dat(out1_dat);
    }
};
