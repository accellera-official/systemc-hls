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
sc_hls_annotate.h

Back-Annotation File I/O

Provides functions to load latency/capacity annotations from files and apply
them to channels in the design hierarchy.

Back-annotation is always compiled in. The feature remains dormant unless
set_latency_capacity() is called or an annotation file is loaded.

Usage:
  #include <systemc-hls>   // sc_hls_annotate.h is included automatically

  // In sc_main or testbench, after elaboration but before sc_start():
  sc_hls::msg_lib::annotate::load_annotations("latencies.txt");

  // Or set programmatically:
  my_module.out_port.set_latency_capacity(20, 5);

Annotation File Format (simple text):
  # Comment lines start with #
  # Format: channel_path latency [capacity]
  # If capacity is omitted, it defaults to latency (models II=1 pipeline)
  #
  # Examples:
  top.dut.out_port 5        # latency=5, capacity=5 (II=1)
  top.dut.chan 20 5         # latency=20, capacity=5 (II=4)

Channel Path Names:
  The path name is the full hierarchical SystemC name of the port or channel.
  For a msg_out port named "out" inside module "dut" inside "top":
    Path = "top.dut.out"
  For a msg_channel named "chan" inside "top":
    Path = "top.chan"

  Use write_annotations() to generate a template file with all channel paths.

Version: 3.0
*/

#pragma once

#include "sc_hls_interfaces.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <systemc.h>
#include <vector>

// Back-annotation support is always compiled in.
// The feature remains dormant unless set_latency_capacity() is called or
// an annotation file is loaded via load_annotations() / load_annotations_json().

namespace sc_hls { namespace msg_lib {
    namespace annotate {

        // ============================================================================
        // Helper: Recursively find all back_annotation_if objects in the hierarchy
        // ============================================================================
        // Traverses the SystemC object hierarchy starting from 'obj' and collects
        // all objects that implement back_annotation_if (i.e., msg_out, msg_dv_out, msg_channel,
        // msg_dv_channel when compiled with SC_HLS_BACK_ANNOTATE).
        inline void find_annotatable_objects(sc_object* obj,
                                             std::vector<back_annotation_if*>& result) {
            // Try to cast this object to back_annotation_if
            back_annotation_if* ann = dynamic_cast<back_annotation_if*>(obj);
            if (ann) {
                result.push_back(ann);
            }

            // Recurse into children
            std::vector<sc_object*> children = obj->get_child_objects();
            for (auto* child : children) {
                if (child) {
                    find_annotatable_objects(child, result);
                }
            }
        }

        // ============================================================================
        // load_annotations: Load annotations from a text file and apply to design
        // ============================================================================
        // Reads the annotation file, finds matching channels in the design hierarchy,
        // and calls set_latency_capacity() on each matched channel.
        //
        // File format:
        //   # Comment
        //   channel_path latency [capacity]
        //
        // If capacity is omitted, it defaults to latency (II=1 pipeline).
        // Returns true if at least one annotation was applied.
        //
        // Must be called after elaboration (after sc_start() is called or during
        // start_of_simulation callback) so that all SC objects exist in the hierarchy.
        inline bool load_annotations(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                  ("Failed to open annotation file: " + filename).c_str());
                return false;
            }

            // Find all annotatable objects in design hierarchy
            std::vector<back_annotation_if*> objects;
            const std::vector<sc_object*> tops = sc_get_top_level_objects();
            for (auto* top : tops) {
                if (top) {
                    find_annotatable_objects(top, objects);
                }
            }

            // Build path-to-object map for fast lookup
            std::map<std::string, back_annotation_if*> path_map;
            for (auto* obj : objects) {
                path_map[obj->get_annotation_path()] = obj;
            }

            // Parse file and apply annotations
            std::string line;
            int line_num = 0;
            int applied = 0;

            while (std::getline(file, line)) {
                ++line_num;

                // Skip empty lines and comments
                if (line.empty() || line[0] == '#') {
                    continue;
                }

                // Parse: channel_path latency [capacity]
                std::istringstream iss(line);
                std::string path;
                int latency = 0;
                int capacity = 0;

                if (!(iss >> path >> latency)) {
                    SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                      ("Invalid format at line " + std::to_string(line_num) +
                                       ": expected 'path latency [capacity]'")
                                          .c_str());
                    continue;
                }

                // Capacity is optional; defaults to latency (models II=1 pipeline)
                if (!(iss >> capacity)) {
                    capacity = latency;
                }

                // Find and configure the channel
                auto it = path_map.find(path);
                if (it != path_map.end()) {
                    it->second->set_latency_capacity(latency, capacity);
                    ++applied;

                    std::ostringstream msg;
                    msg << "Applied latency=" << latency << " capacity=" << capacity << " to "
                        << path;
                    SC_REPORT_INFO("SC_HLS-ANNOTATE", msg.str().c_str());
                } else {
                    SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                      ("Channel not found in design hierarchy: " + path).c_str());
                }
            }

            SC_REPORT_INFO(
                "SC_HLS-ANNOTATE",
                ("Applied " + std::to_string(applied) + " annotations from " + filename).c_str());

            return applied > 0;
        }

        // ============================================================================
        // write_annotations: Write current channel list to a file (template generator)
        // ============================================================================
        // Writes all annotatable channels with their current latency/capacity settings.
        // Useful for generating a template annotation file showing all available paths.
        // Channels with latency=0 have not been annotated yet.
        //
        // Must be called after elaboration so that all SC objects exist.
        inline void write_annotations(const std::string& filename) {
            std::ofstream file(filename);
            if (!file.is_open()) {
                SC_REPORT_ERROR("SC_HLS",
                                ("Failed to create annotation file: " + filename).c_str());
                return;
            }

            file << "# Channel latency annotations\n";
            file << "# Format: channel_path latency capacity\n";
            file << "# If capacity is omitted, it defaults to latency (II=1 pipeline)\n";
            file << "# Generated by sc_hls::msg_lib::annotate::write_annotations()\n\n";

            // Find all annotatable objects
            std::vector<back_annotation_if*> objects;
            const std::vector<sc_object*> tops = sc_get_top_level_objects();
            for (auto* top : tops) {
                if (top) {
                    find_annotatable_objects(top, objects);
                }
            }

            // Write each channel's settings
            for (auto* obj : objects) {
                file << obj->get_annotation_path() << " " << obj->get_latency() << " "
                     << obj->get_capacity() << "\n";
            }

            SC_REPORT_INFO(
                "SC_HLS-ANNOTATE",
                ("Wrote " + std::to_string(objects.size()) + " channels to " + filename).c_str());
        }

        // ============================================================================
        // load_annotations_json: Load annotations from a JSON file and apply to design
        // ============================================================================
        // Reads a JSON annotation file with the following format:
        //
        //   {
        //     "channels": {
        //       "channel_path": { "latency": N, "capacity": M },
        //       "channel_path": { "latency": N }
        //     }
        //   }
        //
        // If "capacity" is omitted, it defaults to the value of "latency" (II=1).
        // Returns true if at least one annotation was applied.
        //
        // This is a self-contained JSON parser for the specific format above.
        // It does not require any external JSON library.
        //
        // Must be called after elaboration so that all SC objects exist in the hierarchy.
        inline bool load_annotations_json(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                  ("Failed to open JSON annotation file: " + filename).c_str());
                return false;
            }

            // Read entire file into a string
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

            // Find all annotatable objects in design hierarchy
            std::vector<back_annotation_if*> objects;
            const std::vector<sc_object*> tops = sc_get_top_level_objects();
            for (auto* top : tops) {
                if (top) {
                    find_annotatable_objects(top, objects);
                }
            }

            // Build path-to-object map for fast lookup
            std::map<std::string, back_annotation_if*> path_map;
            for (auto* obj : objects) {
                path_map[obj->get_annotation_path()] = obj;
            }

            // -------------------------------------------------------------------------
            // Simple JSON parser for the specific format:
            //   { "channels": { "path": { "latency": N, "capacity": M }, ... } }
            //
            // Strategy: scan for quoted strings followed by { "latency": N, ... }
            // -------------------------------------------------------------------------

            // Helper lambda: extract a quoted string starting at pos, advance pos past it
            auto read_string = [&](size_t& pos) -> std::string {
                // Skip to opening quote
                while (pos < content.size() && content[pos] != '"')
                    ++pos;
                if (pos >= content.size())
                    return "";
                ++pos; // skip opening quote
                std::string result;
                while (pos < content.size() && content[pos] != '"') {
                    if (content[pos] == '\\' && pos + 1 < content.size()) {
                        ++pos; // skip escape char
                    }
                    result += content[pos++];
                }
                if (pos < content.size())
                    ++pos; // skip closing quote
                return result;
            };

            // Helper lambda: extract an integer starting at pos, advance pos past it
            auto read_int = [&](size_t& pos) -> int {
                while (pos < content.size() && !std::isdigit(content[pos]) && content[pos] != '-')
                    ++pos;
                if (pos >= content.size())
                    return 0;
                int sign = 1;
                if (content[pos] == '-') {
                    sign = -1;
                    ++pos;
                }
                int value = 0;
                while (pos < content.size() && std::isdigit(content[pos])) {
                    value = value * 10 + (content[pos++] - '0');
                }
                return sign * value;
            };

            // Find the "channels" object
            size_t channels_pos = content.find("\"channels\"");
            if (channels_pos == std::string::npos) {
                SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                  ("No 'channels' key found in JSON file: " + filename).c_str());
                return false;
            }

            // Find the opening brace of the channels object
            size_t brace_pos = content.find('{', channels_pos + 10);
            if (brace_pos == std::string::npos) {
                SC_REPORT_WARNING(
                    "SC_HLS-ANNOTATE",
                    ("Malformed JSON: no '{' after 'channels' in: " + filename).c_str());
                return false;
            }

            int applied = 0;
            size_t pos = brace_pos + 1;

            // Parse each channel entry: "path": { "latency": N, "capacity": M }
            while (pos < content.size()) {
                // Skip whitespace and commas
                while (pos < content.size() && (std::isspace(content[pos]) || content[pos] == ','))
                    ++pos;

                // Check for end of channels object
                if (pos >= content.size() || content[pos] == '}')
                    break;

                // Read channel path (quoted string)
                if (content[pos] != '"') {
                    ++pos;
                    continue;
                }
                std::string path = read_string(pos);
                if (path.empty())
                    break;

                // Skip to opening brace of channel's value object
                while (pos < content.size() && content[pos] != '{')
                    ++pos;
                if (pos >= content.size())
                    break;
                size_t obj_start = pos;
                ++pos; // skip '{'

                // Find closing brace of this channel's value object
                size_t obj_end = content.find('}', pos);
                if (obj_end == std::string::npos)
                    break;
                std::string obj_str = content.substr(obj_start, obj_end - obj_start + 1);

                // Parse "latency" and "capacity" from the value object.
                // Note: read_int reads from 'content', so p must be an absolute
                // offset into 'content', not a relative offset into 'obj_str'.
                int latency = 0;
                int capacity = 0;

                size_t lat_pos = obj_str.find("\"latency\"");
                if (lat_pos != std::string::npos) {
                    size_t p = obj_start + lat_pos + 9; // absolute offset into content
                    latency = read_int(p);
                }

                size_t cap_pos = obj_str.find("\"capacity\"");
                if (cap_pos != std::string::npos) {
                    size_t p = obj_start + cap_pos + 10; // absolute offset into content
                    capacity = read_int(p);
                }

                // Default capacity to latency if not specified
                if (capacity <= 0) {
                    capacity = latency;
                }

                // Find and configure the channel
                auto it = path_map.find(path);
                if (it != path_map.end()) {
                    it->second->set_latency_capacity(latency, capacity);
                    ++applied;

                    std::ostringstream msg;
                    msg << "Applied latency=" << latency << " capacity=" << capacity << " to "
                        << path;
                    SC_REPORT_INFO("SC_HLS-ANNOTATE", msg.str().c_str());
                } else {
                    SC_REPORT_WARNING("SC_HLS-ANNOTATE",
                                      ("Channel not found in design hierarchy: " + path).c_str());
                }

                pos = obj_end + 1;
            }

            SC_REPORT_INFO("SC_HLS-ANNOTATE", ("Applied " + std::to_string(applied) +
                                                 " annotations from JSON " + filename)
                                                    .c_str());

            return applied > 0;
        }

    } // namespace annotate
} } // namespace sc_hls::msg_lib
