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

# Makefile for multi-process access detection test
#
# Error tests: expect SC_HLS-102 to be reported
# Success tests: expect clean exit (no error)

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

# Error tests: two processes in same module, wrong process calls API
ERROR_TESTS = test_out_push test_out_pushnb test_in_pop test_in_popnb test_in_peeknb

# Success tests: same process calls Reset multiple times (multi-cycle reset)
SUCCESS_TESTS = test_multi_reset_out test_multi_reset_in

ALL_TESTS = $(ERROR_TESTS) $(SUCCESS_TESTS)

all: $(ALL_TESTS)

test_out_push: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_OUT_PUSH $(INCLUDES) -o $@ $< $(LDFLAGS)

test_out_pushnb: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_OUT_PUSHNB $(INCLUDES) -o $@ $< $(LDFLAGS)

test_in_pop: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_IN_POP $(INCLUDES) -o $@ $< $(LDFLAGS)

test_in_popnb: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_IN_POPNB $(INCLUDES) -o $@ $< $(LDFLAGS)

test_in_peeknb: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_IN_PEEKNB $(INCLUDES) -o $@ $< $(LDFLAGS)

test_multi_reset_out: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_MULTI_RESET_OUT $(INCLUDES) -o $@ $< $(LDFLAGS)

test_multi_reset_in: testbench.cpp dut.h top.h
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -DTEST_MULTI_RESET_IN $(INCLUDES) -o $@ $< $(LDFLAGS)

# Test target:
#   Error tests  -> PASS if SC_HLS-102 is in output
#   Success tests -> PASS if exit code is 0
test: all
	@pass=0; fail=0; \
	echo ""; \
	echo "=== Error Tests (expect SC_HLS-102) ==="; \
	for t in $(ERROR_TESTS); do \
		output=$$(./$$t 2>&1); \
		if echo "$$output" | grep -q "Error: SC_HLS-102"; then \
			echo "PASS: $$t"; \
			pass=$$((pass+1)); \
		else \
			echo "FAIL: $$t (SC_HLS-102 NOT detected)"; \
			echo "--- Output ---"; \
			echo "$$output"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo ""; \
	echo "=== Success Tests (expect clean exit, no error) ==="; \
	for t in $(SUCCESS_TESTS); do \
		if ./$$t > /dev/null 2>&1; then \
			echo "PASS: $$t"; \
			pass=$$((pass+1)); \
		else \
			echo "FAIL: $$t (unexpected error or non-zero exit)"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo ""; \
	echo "Results: $$pass passed, $$fail failed"; \
	if [ $$fail -gt 0 ]; then exit 1; fi

clean:
	rm -f $(ALL_TESTS) *.vcd

.PHONY: all test clean
