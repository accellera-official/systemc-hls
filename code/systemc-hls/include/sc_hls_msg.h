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
sc_hls_msg.h

Message Passing with Flow Control (dat/vld/rdy) — Public Declarations

This file declares the primary templates for message passing with flow control:
- msg_channel: Channel with vld/rdy handshake
- msg_in: Input port
- msg_out: Output port

Only the primary template *declarations* live here so they are visible in both
the simulation and vendor synthesis views (vendors provide partial
specializations keyed on their own view tag). The simulation-view class bodies
(sim_view_tag / tlm_view_tag), together with the pre/post scheduling
infrastructure they depend on, live in sim_views/sc_hls_msg_sim_view.h and are
included only under the non-synthesis path of the umbrella <systemc-hls>.

Message passing with flow control provides full flow control with back-pressure support through
the ready signal. Both valid and ready must be asserted for a transfer to occur.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// Primary Template Declarations
// ============================================================================

/// msg_in - Input port for message passing with flow control
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_in;

/// msg_out - Output port for message passing with flow control
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_out;

/// msg_channel - Channel for message passing with flow control
/// @tparam Message  Data type
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename Message, typename ViewTag = default_view_tag>
class msg_channel;

} } // namespace sc_hls::msg_lib
