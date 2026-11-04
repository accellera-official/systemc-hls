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

# Mixed-language wrapper example: two self-contained cases.
#   case1_vdut_sctb  - Verilog DUT / SystemC testbench
#   case2_scdut_vtop - SystemC DUT / Verilog top (clock generated in Verilog)
#
# Both cases are built with Verilator (--sc --timing). If Verilator is not
# installed the example is skipped (reported, exit 0) so the examples suite
# does not fail on machines without it.
#
# The runner (run_all.py) invokes "make clean && make run".

VERILATOR ?= verilator
HAVE_VERILATOR := $(shell command -v $(VERILATOR) 2>/dev/null)

CASES = case1_vdut_sctb case2_scdut_vtop

.PHONY: all run case1 case2 run clean
.DEFAULT_GOAL := all

ifeq ($(HAVE_VERILATOR),)
all run case1 case2:
	@echo "17_mixed_language: SKIPPED ($(VERILATOR) not found)"
else
all:
	$(MAKE) -C case1_vdut_sctb all
	$(MAKE) -C case2_scdut_vtop all

run: case1 case2

case1:
	$(MAKE) -C case1_vdut_sctb run

case2:
	$(MAKE) -C case2_scdut_vtop run
endif

clean:
	$(MAKE) -C case1_vdut_sctb clean
	$(MAKE) -C case2_scdut_vtop clean
