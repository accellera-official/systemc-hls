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

# Case 1: Verilog DUT (vpass.v) / SystemC testbench (testbench.cpp).
# Built with Verilator in SystemC mode; requires SYSTEMC_HOME.

TARGET_ARCH     ?= linux64
SYSTEMC_LIB_DIR ?= $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)
CHANNELS_HOME   ?= ../../..
VERILATOR       ?= verilator

VFLAGS  = --sc --exe --build --timing -Wno-fatal -j 0 --Mdir obj_dir --top-module vpass
VFLAGS += -CFLAGS "-std=c++17 -DSC_ALLOW_DEPRECATED_IEEE_API -DSC_INCLUDE_DYNAMIC_PROCESSES"
VFLAGS += -CFLAGS "-I$(SYSTEMC_HOME)/include -I$(abspath $(CHANNELS_HOME))/include -I$(abspath ../../common) -I$(CURDIR)"
VFLAGS += -LDFLAGS "-L$(SYSTEMC_LIB_DIR) -lsystemc -Wl,-rpath,$(SYSTEMC_LIB_DIR)"

.PHONY: all run clean
.DEFAULT_GOAL := all

all: obj_dir/Vvpass

obj_dir/Vvpass: vpass.v testbench.cpp vpass_wrap.h
	$(VERILATOR) $(VFLAGS) vpass.v testbench.cpp -o Vvpass

run: obj_dir/Vvpass
	./obj_dir/Vvpass

clean:
	rm -rf obj_dir *.vcd
