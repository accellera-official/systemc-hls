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
sc_hls_view_tag.h

View Tag Definitions for Tag-Based View Selection

This file defines the view tags used for template-based view selection in
the sc_hls library. View tags are empty structs used as template discriminators
to select between different port/channel implementations.

Version: 4.0
*/

#pragma once

namespace sc_hls { namespace msg_lib {

// ============================================================
// View Tags - Empty structs used as template discriminators
// ============================================================

/// sim_view_tag - Standard simulation view provided by the library
/// This view provides cycle-accurate, signal-based communication with
/// full msg/msg_dv protocol support, random stall injection, and back-annotation.
struct sim_view_tag {};

#ifndef __SYNTHESIS__
/// tlm_view_tag - Fast simulation view using TLM FIFOs
/// This view provides fast functional simulation using tlm::tlm_fifo internally
/// instead of cycle-accurate signal-based communication. Useful for:
/// - Fast functional verification
/// - Software development and testing
/// - Early system-level exploration
/// Note: Simulation-only, not available in synthesis context.
struct tlm_view_tag {};
#endif // __SYNTHESIS__

}}

// ============================================================
// Vendor Extension Point
// ============================================================
// Vendors define their own tags in their headers:
//
//   namespace sc_hls { namespace msg_lib {
//       struct Vendor1SynthTag {};
//   }
//
// Then provide partial specializations for their ports/channels.
// ============================================================

// Note: The following is not compatible with C++-11 and has been
// removed. Vendor-specific implementations of the library classes
// must be included explicitly. This may change in future versions of
// this library.

// #ifdef __SYNTHESIS__
//     #if defined(__has_include)
//         #if __has_include("vendor/sc_hls_vendor_tag.h")
//             #include "vendor/sc_hls_vendor_tag.h"
//         #endif
//     #endif
// #endif

namespace sc_hls { namespace msg_lib {
// ============================================================
// Default View Selection
// ============================================================
#ifdef __SYNTHESIS__
    // In synthesis context, check for vendor-specific default
    #ifdef SC_HLS_DEFAULT_SYNTH_VIEW_TAG
        // The default synth view tag coming from sc_hls_vendor.h file.
        using default_view_tag = SC_HLS_DEFAULT_SYNTH_VIEW_TAG;
    #else
        // No vendor tag defined - fall back to sim_view_tag
        #error "Default synth view tag must be defined"
        //using default_view_tag = sc_hls::msg_lib::sim_view_tag;
    #endif
#else
    // In simulation context, always use sim_view_tag
    using default_view_tag = sim_view_tag;
#endif
} } // namespace sc_hls::msg_lib
