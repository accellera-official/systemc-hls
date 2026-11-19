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
sc_hls_interfaces.h

Base Interface Definitions

This file defines the fundamental interface contracts used throughout the
sc_hls library:

- msg_in_if<Message>: Interface that all input channels must implement
- msg_out_if<Message>: Interface that all output channels must implement
- sim_helper_if<IS_WRITER>: Interface for throughput-accurate simulation timing
  (not for synthesis)

The msg_in_if and msg_out_if interfaces define the API contract for data transfer
operations, including blocking (pop/push) and non-blocking (pop_nb/push_nb/peek_nb)
methods. These interfaces are used in both simulation and synthesis.

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
   to control the order of pre() and post() calls. The current implementation
   processes writers before readers in both phases, but this ordering can be
   adjusted if needed for specific timing semantics.

4. Semantic Clarity: The type system explicitly encodes whether an endpoint is
   a reader or writer, making the code's intent clear and preventing accidental
   misuse (e.g., registering a reader in the writer list).

This approach follows the principle of using the type system to enforce design
invariants at compile time.

Version: 3.0
*/

#pragma once

#include <string>
#include <systemc.h>

// sim_helper_if is simulation-only infrastructure. This header itself does not
// reference it (the protocol classes that need it include sim_helper directly),
// so it must not be pulled into the HLS view.
#ifndef __SYNTHESIS__
#include "internal/sc_hls_sim_helper.h"
#endif

/// ============================================================================
// Back-annotation and random stall are always compiled in.
// The features remain dormant at runtime unless explicitly configured:
//   - annotation: call set_latency_capacity() or load an annotation file
//   - rand stall:  call sc_hls::msg_lib::rand_stall::set()
// ============================================================================
#define SC_HLS_ANNOTATE_ENABLED    1
#define SC_HLS_RAND_STALL_ENABLED  1

namespace sc_hls { namespace msg_lib {

    // ========================================================================
    // trace_marker_if - Interface for hierarchical tracing
    // ========================================================================

    // Marker interface that channels implement to support trace_hierarchy()
    struct trace_marker_if {
        virtual void set_trace(sc_trace_file* tf) = 0;
        virtual ~trace_marker_if() = default;
    };

    // ========================================================================
    // back_annotation_if - Interface for latency back-annotation
    // ========================================================================
    //
    // Interface for channels/ports that support latency back-annotation.
    // Used by sc_hls_annotate.h loaders to configure channel latencies.
    //
    // Latency: Number of clock cycles delay from input to output.
    // Capacity: Maximum number of items that can be in-flight simultaneously.
    //           For II=1 pipelines, capacity typically equals latency.
    //           For II>1 pipelines, capacity = ceil(latency / II).
    //           Example: latency=20, II=4 → capacity=5 (5 items in flight)
    //
    // All channels always inherit from this interface (SC_HLS_ANNOTATE_ENABLED
    // is always 1). The feature remains dormant unless set_latency_capacity() is called.
    struct back_annotation_if {
        virtual ~back_annotation_if() = default;

        // Configure latency and capacity for this channel.
        // latency: Delay in clock cycles (0 = no annotation)
        // capacity: Max items in flight (must be >= 1)
        virtual void set_latency_capacity(int latency, int capacity) = 0;

        // Get the configured latency
        virtual int get_latency() const = 0;

        // Get the configured capacity
        virtual int get_capacity() const = 0;

        // Get the hierarchical path name for annotation file matching
        virtual std::string get_annotation_path() const = 0;
    };

    // ========================================================================
    // tlm_fifo_depth_if - Interface for TLM FIFO depth configuration
    // ========================================================================
    //
    // Interface for TLM channels that support FIFO depth configuration.
    // Similar to back_annotation_if but for TLM FIFO sizing.
    //
    // TLM channels use tlm::tlm_fifo internally for fast functional simulation.
    // The FIFO depth is set at construction time and determines the buffering
    // capacity of the channel.
    //
    // Note: This interface is simulation-only (not available in synthesis).
    struct tlm_fifo_depth_if {
        virtual ~tlm_fifo_depth_if() = default;

        // Set the FIFO depth for this TLM channel
        // Must be called before elaboration (before sc_start)
        // depth: Number of entries the FIFO can hold
        virtual void set_fifo_depth(unsigned int depth) = 0;

        // Get the configured FIFO depth
        virtual unsigned int get_fifo_depth() const = 0;

        // Get the hierarchical path for annotation file matching
        virtual std::string get_fifo_path() const = 0;
    };

    // msg_in_if and msg_out_if are pure API contracts - they define the functional interface
    // without any simulation infrastructure. The simulation timing layer (sim_helper_if)
    // is added by the protocol implementation classes (e.g., msg_reader_protocol).

    template <typename Message> class msg_in_if : public sc_interface {
    public:
        virtual void reset_pop() = 0;
        virtual Message pop() = 0;
        virtual bool pop_nb(Message& m) = 0;
        virtual bool peek_nb(Message& m) = 0;
    };

    template <typename Message> class msg_out_if : public sc_interface {
    public:
        virtual void reset_push() = 0;
        virtual void push(const Message& m) = 0;
        virtual bool push_nb(const Message& m) = 0;
    };

} } // namespace sc_hls::msg_lib