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

# optional: set externally to switch e.g. linux64, mingw-w64, etc.
TARGET_ARCH   ?= linux64

# optional: set externally to provide additional linker flags
#   e.g -fuse-ld=lld
EXTRA_LDFLAGS ?=

# optional: set externally to pickup precompiled SystemC
SYSTEMC_HOME ?= $(HOME)/systemc_install/systemc-3.0.1

# optional: set externally to adapt to custom SystemC layout
#   e.g. systemc
#         \ inc
#         \ lib
#            \ libsystemc_ar.a
SYSTEMC_INC_DIR ?= $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR ?= $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)
SYSTEMC_LIB     ?= -lsystemc

CXX ?= g++
CXXFLAGS = -Wall -DSC_ALLOW_DEPRECATED_IEEE_API -DSC_INCLUDE_DYNAMIC_PROCESSES
INCLUDES = -I$(SYSTEMC_INC_DIR) -I../../include -I../../examples/common
LDFLAGS = -L$(SYSTEMC_LIB_DIR) $(SYSTEMC_LIB) -lpthread -Wl,-rpath,$(SYSTEMC_LIB_DIR)

TESTS = test_generated_clock

all: $(TESTS)

test_generated_clock: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

test: all
	@pass=0; fail=0; \
	for t in $(TESTS); do \
		if ./$$t > /dev/null 2>&1; then \
			echo "PASS: $$t"; \
			pass=$$((pass+1)); \
		else \
			echo "FAIL: $$t"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo ""; \
	echo "Results: $$pass passed, $$fail failed"

clean:
	rm -f $(TESTS) *.vcd *.d *.o

.PHONY: all test clean
