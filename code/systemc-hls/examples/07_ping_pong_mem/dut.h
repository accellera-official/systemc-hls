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

/*
 * Ping-Pong Memory Example - sync_channel for Memory Synchronization
 *
 * This example demonstrates the use of sync_channel for explicit synchronization
 * between threads that share memory. The ping-pong pattern allows one thread
 * to write to memory while another reads from a different section, then they
 * swap roles.
 *
 * Key features:
 *   - sync_channel: rendezvous synchronization between threads
 *   - sync() on the writer side: signals completion of a write phase
 *   - sync() on the reader side: waits for write completion before reading
 *   - Ping-pong buffering: alternates between two halves of a shared memory
 *
 * Architecture:
 *   thread1: writes to mem[0..7] or mem[8..15], then signals via sync1.sync()
 *   thread2: waits via sync1.sync(), then reads from the half that was just written
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>> SC_NAMED(in1);
    msg_out<sc_uint<16>> SC_NAMED(out1);

    sync_channel<> SC_NAMED(sync1); // memory synchronization between threads

    sc_uint<16> mem[128];

    SC_CTOR(dut) {
        SC_THREAD(thread1);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
        SC_THREAD(thread2);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset_thread1() {
        in1.reset_pop();
        sync1.reset_push();
        wait();
    }

    void thread1() {
        bool ping_pong = false;
        reset_thread1();
        while (1) {
            for (int i = 0; i < 8; i++) {
                mem[i + (8 * ping_pong)] = in1.pop();
            }
            sync1.sync_write(mem);
            ping_pong = !ping_pong;
        }
    }

    void reset_thread2() {
        out1.reset_push();
        sync1.reset_pop();
        wait();
    }

    void thread2() {
        bool ping_pong = false;
        reset_thread2();
        while (1) {
            sync1.sync_read(mem);
            for (int i = 0; i < 8; i++) {
                out1.push(mem[i + (8 * ping_pong)]);
            }
            ping_pong = !ping_pong;
        }
    }
};