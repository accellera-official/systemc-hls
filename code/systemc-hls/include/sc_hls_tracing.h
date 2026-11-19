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
sc_hls_tracing.h

VCD Tracing Support

This file provides automatic tracing support for sc_hls library types.
It uses SFINAE (Substitution Failure Is Not An Error) to detect channel
types and automatically trace their signals.

Supported Types:
- msg channels/ports (vld, rdy, dat)
- msg_dv channels/ports (vld, dat)
- Sync channels/ports (vld, rdy)
- sc_vector of channels

Usage:
  sc_trace_file *tf = sc_create_vcd_trace_file("waves");
  sc_trace(tf, my_msg_channel, "my_msg_channel");
  sc_trace(tf, my_msg_dv_channel, "my_msg_dv_channel");
  sc_trace(tf, my_sync_channel, "my_sync_channel");

Version: 3.0
*/

#pragma once

#include <string>
#include <systemc.h>

#include "sc_hls_interfaces.h"
#include "internal/sc_hls_tracing_sfinae.h"

namespace sc_hls { namespace msg_lib {
    // ========================================================================
    // trace_hierarchy - Recursively trace all channels in hierarchy
    // ========================================================================

    // Walks the SystemC object hierarchy and traces all channels that
    // implement trace_marker_if
    inline void trace_hierarchy(sc_object* obj, sc_trace_file* tf) {
        if (trace_marker_if* p = dynamic_cast<trace_marker_if*>(obj)) {
            p->set_trace(tf);
        }
        std::vector<sc_object*> children = obj->get_child_objects();
        for (unsigned i = 0; i < children.size(); i++) {
            if (children[i]) {
                trace_hierarchy(children[i], tf);
            }
        }
    }

    // Convenience wrapper for trace_hierarchy that takes a reference
    inline void trace_hierarchy(sc_object& obj, sc_trace_file* tf) {
        trace_hierarchy(&obj, tf);
    }

    // Alias for compatibility
    inline void add_trace(sc_trace_file* tf, sc_object& obj, const std::string&) {
        trace_hierarchy(&obj, tf);
    }

    // SFINAE helpers are in internal/sc_hls_tracing_sfinae.h

} } // end namespace sc_hls::msg_lib

// Trace function for msg types with vld, rdy, and dat (msg_in, msg_out, msg_channel)
template <typename T>
typename std::enable_if<sc_hls::msg_lib::has_vld_rdy_dat<T>::value, void>::type sc_trace(sc_trace_file* tf, T& obj,
                                                                     const std::string& base_name) {
    sc_trace(tf, obj.vld, base_name + ".vld");
    sc_trace(tf, obj.rdy, base_name + ".rdy");
    sc_trace(tf, obj.dat, base_name + ".dat");
}

// Trace function for Sync types with only vld and rdy (sync_in, sync_out, sync_channel)
template <typename T>
typename std::enable_if<sc_hls::msg_lib::has_vld_rdy<T>::value && !sc_hls::msg_lib::has_vld_rdy_dat<T>::value, void>::type
sc_trace(sc_trace_file* tf, T& obj, const std::string& base_name) {
    sc_trace(tf, obj.vld, base_name + ".vld");
    sc_trace(tf, obj.rdy, base_name + ".rdy");
}

// Trace function for msg_dv types with only vld and dat (msg_dv_in, msg_dv_out, msg_dv_channel)
template <typename T>
typename std::enable_if<sc_hls::msg_lib::has_vld_dat<T>::value && !sc_hls::msg_lib::has_vld_rdy_dat<T>::value &&
                     !sc_hls::msg_lib::has_vld_rdy<T>::value,
                        void>::type
sc_trace(sc_trace_file* tf, T& obj, const std::string& base_name) {
    sc_trace(tf, obj.vld, base_name + ".vld");
    sc_trace(tf, obj.dat, base_name + ".dat");
}

// Trace function for sc_vector - handles both simple signals and channel types
template <typename ElemT>
void sc_trace(sc_trace_file* tf, sc_core::sc_vector<ElemT>& vec, const std::string& base_name) {
    for (std::size_t i = 0; i < vec.size(); ++i) {
#if 0
        // For simple sc_signal types, trace directly with indexed name
        if constexpr (!sc_hls::msg_lib::has_vld_rdy<ElemT>::value &&
                      !sc_hls::msg_lib::has_vld_rdy_dat<ElemT>::value &&
                      !sc_hls::msg_lib::has_vld_dat<ElemT>::value) {
            std::string next_name = base_name + std::string("_") + std::to_string(i);
            sc_trace(tf, vec[i], next_name);
        } else {
            // For complex types (msg_channel, sync_channel, msg_dv_channel, etc.), recursively call
            // sc_trace
            std::string next_name = base_name + std::string("_") + std::to_string(i);
            sc_trace(tf, vec[i], next_name);
        }
#else
        std::string next_name = base_name + std::string("_") + std::to_string(i);
        sc_trace(tf, vec[i], next_name);
#endif
        
    }
}
