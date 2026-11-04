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

# 11_rand_stall — extra targets for stall and reproducible-seed runs.
# The default `make run` invokes ./sim_sc 0 (no stalls); ./sim_sc 1 enables
# random stalls; SC_HLS_RAND_SEED makes a stall run reproducible.

include ../common/Makefile.inc

.PHONY: run-stall run-seed

run-stall: $(TARGET) ; ./$(TARGET) 1
run-seed:  $(TARGET) ; SC_HLS_RAND_SEED=12345 ./$(TARGET) 1