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

#pragma once

#include "dut.h"

#if defined(TEST_MSG_OUT) || defined(TEST_MSG_IN)
SC_MODULE(Top) {
    TestModule SC_NAMED(test);
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_channel<sc_uint<16>> SC_NAMED(out_chan);

    SC_CTOR(Top) : clk("clk", 1, SC_NS) {
        test.clk(clk);
        test.rst_bar(rst_bar);
        test.in(in_chan);
        test.out(out_chan);

        SC_CTHREAD(driver, clk);
        async_reset_signal_is(rst_bar, false);

        SC_CTHREAD(reset_proc, clk);
    }

    void driver() {
        in_chan.reset_push();
        wait();
        in_chan.push(100);
        wait();
    }

    void reset_proc() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};
#endif

#if defined(TEST_MSG_DV_OUT) || defined(TEST_MSG_DV_IN)
SC_MODULE(Top) {
    TestModule SC_NAMED(test);
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);
    msg_dv_channel<sc_uint<16>> SC_NAMED(in_chan);
    msg_dv_channel<sc_uint<16>> SC_NAMED(out_chan);

    SC_CTOR(Top) : clk("clk", 1, SC_NS) {
        test.clk(clk);
        test.rst_bar(rst_bar);
        test.in(in_chan);
        test.out(out_chan);

        SC_CTHREAD(driver, clk);
        async_reset_signal_is(rst_bar, false);

        SC_CTHREAD(reset_proc, clk);
    }

    void driver() {
        in_chan.reset_push();
        wait();
        in_chan.push(100);
        wait();
    }

    void reset_proc() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};
#endif

#if defined(TEST_PORTLESS_PUSH) || defined(TEST_PORTLESS_POP)
SC_MODULE(Top) {
    TestModule SC_NAMED(test);
    sc_clock clk;
    sc_signal<bool> SC_NAMED(rst_bar);

    SC_CTOR(Top) : clk("clk", 1, SC_NS) {
        test.clk(clk);
        test.rst_bar(rst_bar);

        SC_CTHREAD(reset_proc, clk);
    }

    void reset_proc() {
        rst_bar.write(0);
        wait(5);
        rst_bar.write(1);
        wait();
    }
};
#endif