## ****************************************************************************
##
##  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
##  more contributor license agreements.  See the NOTICE file distributed
##  with this work for additional information regarding copyright ownership.
##  Accellera licenses this file to you under the Apache License, Version 2.0
##  (the "License"); you may not use this file except in compliance with the
##  License.  You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
##  implied.  See the License for the specific language governing
##  permissions and limitations under the License.
##
## ****************************************************************************

# 10_logging — build/run targets.
#
# The log FORMAT is selected at compile time via the SC_HLS_EMITTER macro, so
# each format variant is a separate clean build:
#   make run        -> built-in text emitter          -> example_log.txt
#   make run-text   -> alias for the built-in emitter  -> example_log.txt
#   make run-jsonl  -> user json_emitter_tag           -> example_log.jsonl
#   make run-csv    -> user csv_emitter_tag            -> example_log.csv

include ../common/Makefile.inc

.PHONY: run-text run-jsonl run-csv distclean

# Built-in text format is the default `make run` (defined in Makefile.inc).
run-text:
	$(MAKE) clean
	$(MAKE) CXXFLAGS='$(CXXFLAGS) -DLOG_FILENAME=\"example_log.txt\"' run

run-jsonl:
	$(MAKE) clean
	$(MAKE) CXXFLAGS='$(CXXFLAGS) -DSC_HLS_EMITTER=json_emitter_tag -DLOG_FILENAME=\"example_log.jsonl\"' run

run-csv:
	$(MAKE) clean
	$(MAKE) CXXFLAGS='$(CXXFLAGS) -DSC_HLS_EMITTER=csv_emitter_tag -DLOG_FILENAME=\"example_log.csv\"' run

# Override clean to also remove logging artifacts produced by the example.
distclean: clean
	rm -f example_log.txt example_log.jsonl example_log.csv
