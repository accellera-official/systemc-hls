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
sc_hls_fifo_channel.h

FIFO Channel Wrapper

This file provides msg_fifo_channel<Message, Depth, ViewTag>: a module that
wraps a msg_fifo_module together with an input channel and an output channel,
both connected to the internal FIFO. It lets two processes communicate directly
through a buffered msg channel — one process pushes into the input channel, the
other pops from the output channel — without having to bind and manage the
FIFO's raw enq/deq ports themselves.

Compared to using msg_fifo_module directly (see examples/04_fifo), the wrapper
exposes the FIFO as a pair of ready-to-use channels rather than raw ports the
FIFO also drives.

Version: 4.0 - Tag-based view selection
*/

#pragma once

#include <systemc.h>
#include "sc_hls_view_tag.h"
#include "sc_hls_msg.h"
#include "sc_hls_fifo_module.h"
#include "internal/sc_hls_synth_util.h"

namespace sc_hls { namespace msg_lib {

// ============================================================================
// msg_fifo_channel - FIFO wrapped as a pair of channels
// ============================================================================

/// msg_fifo_channel - Buffered msg channel usable directly between two processes
/// @tparam Message  Data type
/// @tparam Depth    FIFO depth (number of entries)
/// @tparam ViewTag  View selection tag (default: default_view_tag)
///
/// Wraps a msg_fifo_module<Message, Depth, ViewTag> plus two msg_channels:
///   - in  : producer side  — a process pushes messages here
///   - out : consumer side  — a process pops messages here
/// The internal FIFO's enq/deq ports are bound to these channels in the
/// constructor, so user code only ever touches in/out.
template <typename Message, unsigned int Depth, typename ViewTag = default_view_tag>
class msg_fifo_channel : public sc_module {
public:
    sc_in_clk clk;
    sc_in<bool> rst;

    // Public channels: bind producer/consumer ports here, or push()/pop()
    // directly from processes in this module's parent.
    msg_channel<Message, ViewTag> SC_NAMED(in);   // producer pushes here
    msg_channel<Message, ViewTag> SC_NAMED(out);  // consumer pops here

    msg_fifo_channel(sc_module_name n = sc_gen_unique_name("msg_fifo_channel"))
        : sc_module(n),
          clk("clk"),
          rst("rst"),
          fifo(SC_HLS_SYNTH_NAME(n, "fifo")) {
        fifo.clk(clk);
        fifo.rst(rst);
        fifo.enq(in);   // FIFO reads (dequeues from) the input channel
        fifo.deq(out);  // FIFO writes (enqueues into) the output channel
    }

    // Convenience forwarders to the producer-side (in) and consumer-side (out)
    // channels, so callers don't have to reach through in./out. explicitly.

    // Producer side (forwards to in)
    void reset_push() { in.reset_push(); }
    void push(const Message &m) { in.push(m); }
    bool push_nb(const Message &m) { return in.push_nb(m); }

    // Consumer side (forwards to out)
    void reset_pop() { out.reset_pop(); }
    Message pop() { return out.pop(); }
    bool pop_nb(Message &m) { return out.pop_nb(m); }
    bool peek_nb(Message &m) { return out.peek_nb(m); }

private:
    msg_fifo_module<Message, Depth, ViewTag> fifo;
};

} } // namespace sc_hls::msg_lib
