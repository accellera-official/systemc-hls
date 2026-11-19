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
sc_hls_sim_helper.h

Simulation Helper Interface (Internal)

This file defines the sim_helper_if interface used for throughput-accurate
simulation timing. This is internal infrastructure that users should not
need to interact with directly.

sim_helper_if Design Rationale:

The sim_helper_if template is parameterized with IS_WRITER to enable type-safe
separation of reader and writer endpoints in the channel manager. This design
provides several benefits:

1. Type Safety: The template parameter creates distinct types for readers
   (sim_helper_if<0>) and writers (sim_helper_if<1>), allowing the channel_manager
   to maintain separate type-safe tracking lists (tracked_rd and tracked_wr).

2. Compile-Time Dispatch: Protocol implementations (msg/msg_dv) use the compile-time
   constant IsWriter in reset_pop()/reset_push() to dispatch to the correct
   channel_manager::add<IS_WRITER>() template instantiation, ensuring endpoints are
   registered in the appropriate list without runtime overhead.

3. Scheduling Flexibility: Separate reader and writer lists allow channel_manager
   to control the order of pre() and post() calls.

4. Semantic Clarity: The type system explicitly encodes whether an endpoint is
   a reader or writer, making the code's intent clear and preventing accidental
   misuse.

Version: 3.0
*/

#pragma once

#include <systemc.h>

namespace sc_hls { namespace msg_lib {

    template <bool IS_WRITER> class sim_helper_if {
    public:
        static constexpr int IsWriter = IS_WRITER;

        virtual void post() = 0;
        virtual void pre() = 0;

        bool was_reset{false};
        int clock_number{0};
        sc_process_handle owning_process;
        sc_interface* driven_signal{nullptr};
    };

} } // namespace sc_hls::msg_lib