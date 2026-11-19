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

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

#ifdef TEST_MSG_OUT
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(in);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(TestModule) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        in.reset_pop();
        // out.reset_push() intentionally omitted
        wait();
        auto val = in.pop();
        out.push(val);
    }
};
#endif

#ifdef TEST_MSG_IN
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_in<sc_uint<16>> SC_NAMED(in);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(TestModule) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        // in.reset_pop() intentionally omitted
        out.reset_push();
        wait();
        auto val = in.pop();
        out.push(val);
    }
};
#endif

#ifdef TEST_MSG_DV_OUT
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_dv_in<sc_uint<16>> SC_NAMED(in);
    msg_dv_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(TestModule) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        in.reset_pop();
        // out.reset_push() intentionally omitted
        wait();
        auto val = in.pop();
        out.push(val);
    }
};
#endif

#ifdef TEST_MSG_DV_IN
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_dv_in<sc_uint<16>> SC_NAMED(in);
    msg_dv_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(TestModule) {
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void proc() {
        // in.reset_pop() intentionally omitted
        out.reset_push();
        wait();
        auto val = in.pop();
        out.push(val);
    }
};
#endif

#ifdef TEST_PORTLESS_PUSH
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(chan);

    SC_CTOR(TestModule) {
        SC_THREAD(writer_proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(reader_proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void writer_proc() {
        // chan.reset_push() intentionally omitted
        wait();
        chan.push(42);
    }

    void reader_proc() {
        chan.reset_pop();
        wait();
        while (1) {
            auto val = chan.pop();
            (void)val;
            wait();
        }
    }
};
#endif

#ifdef TEST_PORTLESS_POP
SC_MODULE(TestModule) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);
    msg_channel<sc_uint<16>> SC_NAMED(chan);

    SC_CTOR(TestModule) {
        SC_THREAD(writer_proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(reader_proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void writer_proc() {
        chan.reset_push();
        wait();
        chan.push(42);
    }

    void reader_proc() {
        // chan.reset_pop() intentionally omitted
        wait();
        auto val = chan.pop();
        (void)val;
    }
};
#endif