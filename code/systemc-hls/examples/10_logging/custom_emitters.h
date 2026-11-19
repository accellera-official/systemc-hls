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
 * custom_emitters.h — USER extension code (not part of the sc_hls library).
 *
 * Demonstrates how a library user plugs a custom transaction-log format into
 * sc_hls without the working group having to standardize on it: define a tag
 * type, specialize sc_hls::msg_lib::transaction_emitter on it, and select it at
 * build time with -DSC_HLS_EMITTER=<tag>.
 *
 * This header must be included AFTER <systemc-hls> (it specializes a template
 * declared there). The active emitter is chosen by the SC_HLS_EMITTER macro,
 * normally passed on the compile line by the Makefile:
 *
 *   built-in text : (no macro)                       -> sc_hls default
 *   JSON Lines    : -DSC_HLS_EMITTER=json_emitter_tag
 *   CSV           : -DSC_HLS_EMITTER=csv_emitter_tag
 */

#pragma once

// Tag types selecting each custom format. These must be declared BEFORE
// <systemc-hls> is included: the SC_HLS_EMITTER macro (passed on the compile
// line) is substituted into logger<Message>::emit inside the library header,
// where the tag is a non-dependent name resolved at template-definition time.
struct json_emitter_tag {};
struct csv_emitter_tag {};

#include <ostream>
#include <string>
#include <systemc-hls>

// ---------------------------------------------------------------------------
// JSON Lines: one JSON object per transaction.
//   {"port":"tb_top.in_chan","dir":"PUSH","data":"0x0","time":"60 ns"}
// ---------------------------------------------------------------------------
template <typename Message>
struct sc_hls::msg_lib::transaction_emitter<Message, json_emitter_tag> {
    static void emit(std::ostream* os, const Message& m, int /*id*/,
                     const std::string& port_name, const std::string& direction,
                     const sc_hls::msg_lib::emitter_options& opts) {
        if (!os)
            return;
        *os << "{\"port\":\"" << port_name << "\"";
        if (opts.log_direction)
            *os << ",\"dir\":\"" << direction << "\"";
        *os << ",\"data\":\"0x" << std::hex << m << std::dec << "\",\"time\":\""
            << sc_core::sc_time_stamp().to_string() << "\"}\n";
    }
};

// ---------------------------------------------------------------------------
// CSV: one comma-separated row per transaction (no header row).
//   "tb_top.in_chan",PUSH,0x0,60 ns
// ---------------------------------------------------------------------------
template <typename Message>
struct sc_hls::msg_lib::transaction_emitter<Message, csv_emitter_tag> {
    static void emit(std::ostream* os, const Message& m, int /*id*/,
                     const std::string& port_name, const std::string& direction,
                     const sc_hls::msg_lib::emitter_options& opts) {
        if (!os)
            return;
        *os << "\"" << port_name << "\"";
        if (opts.log_direction)
            *os << "," << direction;
        *os << ",0x" << std::hex << m << std::dec << "," << sc_core::sc_time_stamp().to_string()
            << "\n";
    }
};
