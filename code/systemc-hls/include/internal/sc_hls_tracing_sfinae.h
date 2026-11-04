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
sc_hls_tracing_sfinae.h

SFINAE Type Traits for Tracing (Internal)

This file contains SFINAE (Substitution Failure Is Not An Error) helper
templates used to detect channel types for automatic tracing. These are
implementation details that users should not need to interact with directly.

Version: 3.0
*/

#pragma once

#include <type_traits>
#include <utility>

namespace sc_hls { namespace msg_lib {

    // ========================================================================
    // SFINAE helpers for automatic tracing
    // ========================================================================

    template <typename ...> using void_type = void;

    // SFINAE helper to detect if a type has vld, rdy, and dat members (msg)
    template <typename T, typename = void> struct has_vld_rdy_dat : std::false_type {};

    template <typename T>
    struct has_vld_rdy_dat<
        T, void_type<decltype(std::declval<T>().vld), decltype(std::declval<T>().rdy),
                       decltype(std::declval<T>().dat)>> : std::true_type {};

    // SFINAE helper to detect if a type has only vld and rdy members (Sync)
    template <typename T, typename = void> struct has_vld_rdy : std::false_type {};

    template <typename T>
    struct has_vld_rdy<
        T, void_type<decltype(std::declval<T>().vld), decltype(std::declval<T>().rdy)>>
        : std::true_type {};

    // SFINAE helper to detect if a type has only vld and dat members (msg_dv)
    template <typename T, typename = void> struct has_vld_dat : std::false_type {};

    template <typename T>
    struct has_vld_dat<
        T, void_type<decltype(std::declval<T>().vld), decltype(std::declval<T>().dat)>>
        : std::true_type {};

} } // namespace sc_hls::msg_lib
