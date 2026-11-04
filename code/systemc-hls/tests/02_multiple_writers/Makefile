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

# Makefile for multiple writers detection test

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

TARGET = sim_sc
SRCS = testbench.cpp
OBJS = $(SRCS:.cpp=.o)

# Compiler settings
CXX ?= g++
CXXFLAGS = -Wall -DSC_ALLOW_DEPRECATED_IEEE_API -DSC_INCLUDE_DYNAMIC_PROCESSES
CXXFLAGS += -I$(SYSTEMC_INC_DIR) -I../../include -I../../examples/common
CXXFLAGS += -MMD -MP

LDFLAGS = -L$(SYSTEMC_LIB_DIR) $(SYSTEMC_LIB) -lpthread -Wl,-rpath,$(SYSTEMC_LIB_DIR)

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d) *.vcd

# Run the simulation (exits non-zero when error is detected - that is expected)
run: $(TARGET)
	-./$(TARGET)

# Test target: PASS if SC_HLS-102 is reported, FAIL otherwise
test: $(TARGET)
	@output=$$(./$(TARGET) 2>&1); \
	if echo "$$output" | grep -q "Error: SC_HLS-102"; then \
		echo "PASS: SC_HLS-102 error correctly detected"; \
		exit 0; \
	else \
		echo "FAIL: SC_HLS-102 error was NOT detected"; \
		echo "--- Output ---"; \
		echo "$$output"; \
		exit 1; \
	fi

.PHONY: all clean run test

-include $(OBJS:.o=.d)
