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
sc_hls_ver.h

Macros to define SystemC HLS version in BOTH the simulation and vendor synthesis views.

*/

#pragma once

#define SYSTEMC_HLS_0_1_0

#define SYSTEMC_HLS_VERSION       20261119
#define SC_HLS_VERSION_ORIGINATOR "Accellera"
#define SC_HLS_VERSION_MAJOR      0
#define SC_HLS_VERSION_MINOR      1
#define SC_HLS_VERSION_PATCH      0
#define SC_HLS_IS_PRERELEASE      1

#define SC_HLS_COPYRIGHT                      \
  "Copyright (c) 2026 by all Contributors,\n" \
  "ALL RIGHTS RESERVED\n"


#define SC_HLS_VERSION_RELEASE_DATE \
  SC_HLS_STRINGIFY_HELPER_( SYSTEMC_HLS_VERSION )

#if ( SC_HLS_IS_PRERELEASE == 1 )
#  define SC_HLS_VERSION_PRERELEASE "pub_rev"
#  define SC_HLS_VERSION \
    SC_HLS_STRINGIFY_HELPER_( SC_HLS_VERSION_MAJOR.SC_HLS_VERSION_MINOR.SC_HLS_VERSION_PATCH ) \
    "_" SC_HLS_VERSION_PRERELEASE "_" SC_HLS_VERSION_RELEASE_DATE \
    "-" SC_HLS_VERSION_ORIGINATOR
#else
#  define SC_HLS_VERSION_PRERELEASE "" // nothing
#  define SC_HLS_VERSION \
    SC_HLS_STRINGIFY_HELPER_( SC_HLS_VERSION_MAJOR.SC_HLS_VERSION_MINOR.SC_HLS_VERSION_PATCH ) \
    "-" SC_HLS_VERSION_ORIGINATOR
#endif
