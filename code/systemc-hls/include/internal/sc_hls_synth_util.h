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
sc_hls_synth_util.h

Shared macros needed in BOTH the simulation and vendor synthesis views.

These two macros must be visible regardless of __SYNTHESIS__, so they live in
this small header (included unconditionally by the umbrella <systemc-hls>)
rather than in the simulation-only infrastructure headers.

- SC_HLS_SYNTH_NAME(prefix, nm): builds a hierarchical sc_object name for
  simulation; expands to "" under synthesis (HLS ignores object names). Used by
  the synthesizable FIFO (sc_hls_fifo_module.h / sc_hls_fifo_channel.h).

- SC_HLS_LOGP(Str): prints "(<time>): <process_name>: <message>" to stdout.
  Expands to nothing under synthesis.
*/

#pragma once

#include <systemc.h>

// SC_HLS_SYNTH_NAME: hierarchical name in simulation, empty under synthesis.
#ifndef SC_HLS_SYNTH_NAME
  #ifdef __SYNTHESIS__
    #define SC_HLS_SYNTH_NAME(prefix, nm) ""
  #else
    #define SC_HLS_SYNTH_NAME(prefix, nm) (std::string(prefix) + nm).c_str()
  #endif
#endif

// SC_HLS_LOGP: process-tagged stdout logging in simulation; no-op under synthesis.
#ifndef SC_HLS_LOGP
#ifndef __SYNTHESIS__
#define SC_HLS_LOGP(Str)                                                                           \
    std::cout << "(" << sc_time_stamp() << "): " << sc_core::sc_get_current_process_b()->name()    \
              << ": " << Str << std::endl
#else
#define SC_HLS_LOGP(Str)
#endif
#endif
