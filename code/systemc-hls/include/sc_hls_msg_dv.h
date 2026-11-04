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
sc_hls_msg_dv.h

Message Passing without Back-Pressure (dat/vld) — Public Declarations

This file declares the primary templates for message passing without
back-pressure:
- msg_dv_channel: Channel with valid-only handshake (no ready)
- msg_dv_in: Input port
- msg_dv_out: Output port

Only the primary template *declarations* live here so they are visible in both
the simulation and vendor synthesis views (vendors provide partial
specializations keyed on their own view tag). The simulation-view class bodies
(sim_view_tag), together with the pre/post scheduling infrastructure they
depend on, live in sim_views/sc_hls_msg_dv_sim_view.h and are included only
under the non-synthesis path of the umbrella <systemc-hls>.

Message passing without back-pressure provides valid-only signaling. The
producer drives data whenever it has data available, and the consumer must be
ready to accept data every cycle when valid is asserted.

The dv_overrun_policy enum and its global default free functions
(set_default_dv_overrun_policy / get_default_dv_overrun_policy) are defined in
this header so they form part of the public API surface. The reader's
sticky-buffer semantics and per-port overrun-policy methods are implemented in
internal/sc_hls_msg_dv_protocol.h (simulation view).

Version: 4.1 - Sticky reader buffer + dv_overrun_policy
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// Primary Template Declarations
// ============================================================================

/// msg_dv_in - Input port for message passing without back-pressure
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_dv_in;

/// msg_dv_out - Output port for message passing without back-pressure
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_dv_out;

/// msg_dv_channel - Channel for message passing without back-pressure
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_dv_channel;

// ============================================================================
// dv_overrun_policy — public API and supporting helpers
// ============================================================================
// The msg_dv protocol has no rdy/back-pressure signal. When a new valid beat
// arrives while the reader's sticky buffer is still occupied, the previous beat
// is overwritten (bus-faithful). Each such overrun event is reported at a
// configurable severity selected by this policy. The default is Mute (overruns
// are not reported).
//
// EXPERIMENTAL: the overrun policy API is not part of the draft standard.
//
// The enum and the global-default free functions are declared here so they are
// part of the public API surface. The internal protocol engine
// (internal/sc_hls_msg_dv_protocol.h) includes this header and uses them.

enum class dv_overrun_policy { Unset = 0, Error, Warning, Mute };

// Support optional use
struct dv_overrun_policy_value {
    dv_overrun_policy p { dv_overrun_policy::Unset };

    dv_overrun_policy_value &operator=( const dv_overrun_policy_value &o) { p = o.p; return *this; };
    dv_overrun_policy_value &operator=( const dv_overrun_policy &o) { p = o; return *this; };
    constexpr explicit operator bool() const { return (!(p == dv_overrun_policy::Unset)); }
    constexpr bool has_value() const { return operator bool(); }
    constexpr dv_overrun_policy operator *() const { return p; }
    void reset() { p = dv_overrun_policy::Unset; }
};

// Meyers singleton — header-only, ODR-safe. (Formerly in namespace detail,
// now folded directly into msg_lib.)
inline dv_overrun_policy& dv_overrun_policy_default() {
    static dv_overrun_policy p = dv_overrun_policy::Mute;
    return p;
}

inline void set_default_dv_overrun_policy(dv_overrun_policy p) {
    dv_overrun_policy_default() = p;
}

inline dv_overrun_policy get_default_dv_overrun_policy() {
    return dv_overrun_policy_default();
}


} } // namespace sc_hls::msg_lib
