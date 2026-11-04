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
sc_hls_logging.h

Transaction Logging Support

This file provides transaction logging capabilities for sc_hls library types.
Every push/pop on a logged channel is emitted to a file by a compile-time
selected "transaction emitter".

The library ships exactly ONE built-in format (a pipe-delimited text line).
The output format is an extension point: library users can define their own
emitter by specializing `transaction_emitter<Message, Tag>` on their own tag
type and selecting it with the `SC_HLS_EMITTER` macro. The working group does
not standardize on any particular structured format (JSON, CSV, ...); those are
demonstrated as user code in examples/10_logging.

Usage (built-in format):
  sc_hls::msg_lib::channel_logs logs;
  logs.enable("sim_logs");        // writes sim_logs.txt
  logs.log_hierarchy(top);

Extending the format (see examples/10_logging):
  struct my_tag {};
  template <typename Message>
  struct sc_hls::msg_lib::transaction_emitter<Message, my_tag> {
      static void emit(std::ostream* os, const Message& m, int id,
                       const std::string& port_name, const std::string& direction,
                       const sc_hls::msg_lib::emitter_options& opts) { ... }
  };
  // then build the design with -DSC_HLS_EMITTER=my_tag

Version: 4.0
*/

#pragma once

#include <cstdint>
#include <fstream>
#include <ios>
#include <ostream>
#include <string>
#include <systemc.h>
#include <vector>

namespace sc_hls { namespace msg_lib {

    // ========================================================================
    // Emitter Options
    // ========================================================================

    // Base options passed to every emit(). Specialized emitters that need extra
    // configuration may derive from this and add fields.
    struct emitter_options {
        bool log_direction = true; // include the PUSH/POP indicator
        bool unbuffered    = false; // flush the stream after each write
    };

    // ========================================================================
    // Transaction Emitter (compile-time format selection point)
    // ========================================================================

    // Tag selecting the built-in emitter.
    struct default_emitter_tag {};

    // The active emitter is selected at compile time. Users override by defining
    // SC_HLS_EMITTER to their own tag type (e.g. -DSC_HLS_EMITTER=my_tag) after
    // providing a matching transaction_emitter specialization.
#ifndef SC_HLS_EMITTER
#define SC_HLS_EMITTER sc_hls::msg_lib::default_emitter_tag
#endif

    // Primary template = the one built-in format: pipe-delimited text.
    //   "<port_name> | <dir> | <data> | <time>"   (dir omitted if !log_direction)
    // The payload is rendered with its operator<<, using the stream's default
    // formatting. Users who want a different layout or payload rendering
    // specialize this on their own tag.
    template <typename Message, typename Tag>
    struct transaction_emitter {
        static void emit(std::ostream* os, const Message& m, int /*id*/,
                         const std::string& port_name, const std::string& direction,
                         const emitter_options& opts) {
            if (!os)
                return;
            *os << port_name << " | ";
            if (opts.log_direction)
                *os << direction << " | ";
            *os << m << " | " << sc_core::sc_time_stamp().to_string() << "\n";
        }
    };

    // ========================================================================
    // logger Interface (for type erasure during hierarchy walk)
    // ========================================================================

    class log_marker_if {
    public:
        virtual ~log_marker_if() = default;
        virtual bool set_log(std::ostream* os, int log_id, const std::string& port_name,
                             const emitter_options& opts) = 0;
        virtual std::string get_port_name() const = 0;
    };

    // ========================================================================
    // logger Template (mixed into each channel endpoint)
    // ========================================================================

    template <typename Message> class logger : public log_marker_if {
    public:
        logger() = default;

        // Enable logging (from log_marker_if)
        virtual bool set_log(std::ostream* os, int log_id, const std::string& port_name,
                             const emitter_options& opts) override {
            stream_    = os;
            log_id_    = log_id;
            port_name_ = port_name;
            opts_      = opts;
            return true;
        }

        virtual std::string get_port_name() const override {
            return port_name_;
        }

        // Emit a log entry via the compile-time selected emitter.
        void emit(const Message& m, const std::string& direction) {
            if (!stream_)
                return;
            transaction_emitter<Message, SC_HLS_EMITTER>::emit(stream_, m, log_id_, port_name_,
                                                               direction, opts_);
            if (opts_.unbuffered)
                stream_->flush();
        }

    protected:
        std::string port_name_;

    private:
        std::ostream* stream_ = nullptr;
        int log_id_ = 0;
        emitter_options opts_;
    };

    // ========================================================================
    // Channel Logs Manager
    // ========================================================================

    class channel_logs {
    public:
        channel_logs() = default;
        ~channel_logs() {
            if (log_stream_.is_open())
                log_stream_.close();
        }

        // Enable logging. Writes to a single file named exactly `fname` (the caller
        // chooses the extension). Returns 0 on success, 1 if the file cannot be opened.
        int enable(const std::string& fname = "channel_logs.txt",
                   const emitter_options& opts = emitter_options()) {
            opts_ = opts;
            if (opts.unbuffered) {
                log_stream_.rdbuf()->pubsetbuf(0, 0);
            }
            log_stream_.open(fname);
            if (!log_stream_.is_open()) {
                std::cerr << "Cannot open file '" << fname << "'" << std::endl;
                return 1;
            }
            return 0;
        }

        // Register hierarchy to log (uses a spawned thread so full path names are
        // resolved after start_of_simulation).
        void log_hierarchy(sc_object& obj) {
            target_obj_ = &obj;
            sc_spawn(sc_bind(&channel_logs::init_logging, this), "channel_logs_init");
        }

    private:
        std::ofstream log_stream_;
        emitter_options opts_;
        int log_num_ = 0;
        sc_object* target_obj_ = nullptr;

        void init_logging() {
            wait(SC_ZERO_TIME); // Wait for start_of_simulation to complete
            log_hier_helper(target_obj_);
        }

        void log_hier_helper(sc_object* obj) {
            if (log_marker_if* p = dynamic_cast<log_marker_if*>(obj)) {
                // Use sc_object's name directly instead of get_port_name(): this
                // avoids timing issues where port_name_ may not be set yet.
                if (log_stream_.is_open()) {
                    ++log_num_;
                    p->set_log(&log_stream_, log_num_, obj->name(), opts_);
                }
            }

            std::vector<sc_object*> children = obj->get_child_objects();
            for (unsigned i = 0; i < children.size(); i++) {
                if (children[i]) {
                    log_hier_helper(children[i]);
                }
            }
        }
    };

} } // namespace sc_hls::msg_lib
