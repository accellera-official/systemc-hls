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

# Shared build rules for all systemc-hls examples.
#
# Each example's Makefile only needs:
#     include ../common/Makefile.inc
#
# An example may override TARGET, SRCS, or add extra phony targets above the
# include line. The runner (run_all.py) invokes "make clean && make run".

# optional: set externally to switch e.g. linux64, mingw-w64, etc.
TARGET_ARCH   ?= linux64

# optional: set externally to provide additional linker flags
#   e.g -fuse-ld=lld
EXTRA_LDFLAGS ?=

# optional: set externally to pickup precompiled SystemC
SYSTEMC_HOME  ?= /usr/local/systemc-2.3.4

# optional: set externally to adapt to custom SystemC layout
#   e.g. systemc
#         \ inc
#         \ lib
#            \ libsystemc_ar.a
SYSTEMC_INC_DIR ?= $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR ?= $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)
SYSTEMC_LIB     ?= -lsystemc

CHANNELS_HOME ?= ../..

CXX           ?= g++
CXXWARN       ?= -Wall
CXXDEFS       ?= -DSC_ALLOW_DEPRECATED_IEEE_API
CXXDEFS       += -DSC_INCLUDE_DYNAMIC_PROCESSES

CXXFLAGS      += $(CXXSTD) $(CXXWARN) $(CXXDEFS)
CXXFLAGS      += -I$(SYSTEMC_INC_DIR)
CXXFLAGS      += -I$(CHANNELS_HOME)/include
CXXFLAGS      += -I../common
CXXFLAGS      += -MMD -MP

LDFLAGS       += -L$(SYSTEMC_LIB_DIR)
LDLIBS         = $(SYSTEMC_LIB) -lpthread
LDFLAGS       += -Wl,-rpath,$(SYSTEMC_LIB_DIR)

TARGET        ?= sim_sc
SRCS          ?= testbench.cpp
OBJS          := $(SRCS:.cpp=.o)
DEPS          := $(SRCS:.cpp=.d)

.PHONY: all run clean
.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(EXTRA_LDFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) *.vcd

-include $(DEPS)
