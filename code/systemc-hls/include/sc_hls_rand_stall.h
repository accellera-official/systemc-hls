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
sc_hls_rand_stall.h

Random Stall Injection for Verification

This file provides random stall injection for verification and stress testing.
It is always compiled in. The feature remains dormant unless rand_stall::set()
is called to inject stall generators into the channel hierarchy.

Architecture (from mini_matchlib):
- stable_random: Deterministic RNG with pathname-based seeding
- rand_force_if: Interface for stall decision (force_zero())
- pacer: Two-probability stall generator (stall_prob + hold_stall_prob)
- rand_stall: Hierarchy traversal to enable stalls on all channels

Usage:
  #include <systemc-hls>   // sc_hls_rand_stall.h is included automatically

  // In sc_main, after creating top module:
  sc_hls::msg_lib::rand_stall::set(*top);          // enable stalls (silent)
  sc_hls::msg_lib::rand_stall::set(*top, true);    // enable stalls (verbose: print probabilities)

Environment Variables:
  SC_HLS_RAND_SEED - Seed for random number generation (default: 0)

Version: 3.0
*/

#pragma once

#include <cstdlib>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <systemc.h>

namespace sc_hls { namespace msg_lib {

    // ============================================================================
    // rand_force_if: Interface for stall decision
    // ============================================================================
    // Implemented by pacer. Called every cycle by reader protocol's pre()/post().
    // Returns true if the reader should stall (force rdy=0, don't accept data).

    class rand_force_if {
    public:
        virtual ~rand_force_if() = default;
        virtual bool force_zero() = 0;
        virtual void reset() = 0;
    };

    // ============================================================================
    // set_rand_force_if: Interface for setting stall generator on channels
    // ============================================================================
    // Implemented by msg_channel and msg_dv_channel. Used by rand_stall::set() to
    // inject a pacer into each channel during hierarchy traversal.

    class set_rand_force_if {
    public:
        virtual ~set_rand_force_if() = default;
        virtual void set_rand_force(std::shared_ptr<rand_force_if> r_if) = 0;
    };

    // ============================================================================
    // stable_random: Deterministic RNG with pathname-based seeding
    // ============================================================================
    // Provides reproducible random numbers. Each channel gets its own generator
    // seeded by: base_seed + hash(pathname). This ensures:
    // - Same stall patterns across runs with same seed
    // - Different stall patterns for different channels
    // - Reproducibility for debugging

    class stable_random {
    private:
        std::hash<std::string> hasher_;
        std::mt19937 generator_;
        std::uniform_int_distribution<unsigned> distribution_;
        bool seeded_ = false;

    public:
        std::string pathname; // Set to channel pathname for seeding

        stable_random() : distribution_(0, UINT_MAX) {}

        void do_seed() {
            unsigned int seed = 0;

            // Check environment variable for seed
            const char* env_seed = std::getenv("SC_HLS_RAND_SEED");
            if (env_seed != nullptr) {
                seed = static_cast<unsigned int>(std::atoi(env_seed));
            }

            // Combine base seed with pathname hash for per-channel uniqueness
            if (!pathname.empty()) {
                generator_.seed(seed + static_cast<unsigned int>(hasher_(pathname)));
            } else {
                generator_.seed(seed);
            }
            seeded_ = true;
        }

        unsigned get() {
            if (!seeded_) {
                do_seed();
            }
            return distribution_(generator_);
        }
    };

    // ============================================================================
    // pacer: Two-probability stall generator
    // ============================================================================
    // Implements a state machine with two probabilities:
    // - stall_prob: Probability to START stalling (enter stall state)
    // - hold_stall_prob: Probability to CONTINUE stalling (stay in stall state)
    //
    // This creates variable-length stall bursts, which is more realistic than
    // independent per-cycle stall decisions.
    //
    // Example: stall_prob=0.2, hold_stall_prob=0.8
    // - 20% chance to start stalling
    // - Once stalling, 80% chance to continue each cycle
    // - Average stall duration = 1 / (1 - 0.8) = 5 cycles

    class pacer : public rand_force_if {
    private:
        static constexpr unsigned PRECISION = 1000;

        unsigned stall_prob_;      // Probability to START stalling (0-1000)
        unsigned hold_stall_prob_; // Probability to CONTINUE stalling (0-1000)
        bool stalled_ = false;     // Current stall state
        stable_random gen_;        // Per-channel RNG

        bool random(unsigned prob) {
            return (gen_.get() % (PRECISION + 1)) < prob;
        }

    public:
        // Constructor: probabilities are floats 0.0-1.0
        pacer(float stall_prob, float hold_stall_prob)
            : stall_prob_(static_cast<unsigned>(stall_prob * PRECISION)),
              hold_stall_prob_(static_cast<unsigned>(hold_stall_prob * PRECISION)) {}

        // Set pathname for deterministic seeding
        void set_pathname(const std::string& path) {
            gen_.pathname = path;
        }

        // Reset stall state (called on channel reset)
        void reset() override {
            stalled_ = false;
        }

        // Main stall decision function - called every cycle
        // Returns true if should stall this cycle
        bool force_zero() override {
            if (stalled_) {
                // Currently stalling - check if we should stop
                if (!random(hold_stall_prob_)) {
                    stalled_ = false;
                }
            } else {
                // Not stalling - check if we should start
                if (random(stall_prob_)) {
                    stalled_ = true;
                }
            }
            return stalled_;
        }
    };

    // ============================================================================
    // rand_stall: Hierarchy traversal to enable stalls
    // ============================================================================
    // Traverses the SystemC object hierarchy starting from a given object.
    // For each object that implements set_rand_force_if (channels), creates
    // a pacer with random probabilities and injects it.

    struct rand_stall {
        // Enable random stalls on all channels under the given object
        // verbose: if true, print stall probabilities for each channel
        static void set(sc_object& obj, bool verbose = false) {
            stable_random gen;
            set_helper(&obj, gen, verbose);
        }

    private:
        static void set_helper(sc_object* obj, stable_random& gen, bool verbose) {
            // Try to cast to set_rand_force_if (channels implement this)
            if (auto* p = dynamic_cast<set_rand_force_if*>(obj)) {
                // Generate random probabilities for this channel (0-99%)
                float stall_prob = static_cast<float>(gen.get() % 100) / 100.0f;
                float hold_prob = static_cast<float>(gen.get() % 100) / 100.0f;

                // Create pacer with these probabilities
                auto staller = std::make_shared<pacer>(stall_prob, hold_prob);
                staller->set_pathname(obj->name());

                // Inject into channel
                p->set_rand_force(staller);

                if (verbose) {
                    std::cout << "rand_stall: " << obj->name()
                              << " stall%=" << static_cast<int>(stall_prob * 100)
                              << " hold%=" << static_cast<int>(hold_prob * 100) << "\n";
                }
            }

            // Recurse into children
            std::vector<sc_object*> children = obj->get_child_objects();
            for (sc_object* child : children) {
                if (child != nullptr) {
                    set_helper(child, gen, verbose);
                }
            }
        }
    };

} } // namespace sc_hls::msg_lib