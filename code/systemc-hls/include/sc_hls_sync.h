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
sc_hls_sync.h

Rendezvous Synchronization — Public Declarations

This file declares the Sync protocol primary templates:
- sync_channel: A channel implementing strict rendezvous semantics
- sync_in: Input port for rendezvous synchronization
- sync_out: Output port for rendezvous synchronization

Only the primary template *declarations* live here so they are visible in both
the simulation and vendor synthesis views (vendors provide partial
specializations keyed on their own view tag). The simulation-view class bodies
(sim_view_tag) live in sim_views/sc_hls_sync_sim_view.h and are included only
under the non-synthesis path of the umbrella <systemc-hls>.

Unlike the msg and msg_dv protocols which transfer data, the sync protocol provides
cycle-accurate synchronization between processes without data payload. It does
not participate in the global pre/post scheduling; the simulation view
implements synchronization directly using SystemC wait()/signal primitives.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// Primary Template Declarations
// ============================================================================

/// sync_in - Input port for rendezvous synchronization
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename ViewTag = default_view_tag>
class sync_in;

/// sync_out - Output port for rendezvous synchronization
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename ViewTag = default_view_tag>
class sync_out;

/// sync_channel - Channel for rendezvous synchronization
/// @tparam ViewTag  View selection tag (default: default_view_tag)
template <typename ViewTag = default_view_tag>
class sync_channel;

} } // namespace sc_hls::msg_lib
