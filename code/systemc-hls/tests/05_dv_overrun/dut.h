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
 * dut.h — msg_dv_in consumer that intentionally lags the producer.
 *
 * The consumer pops a value, then performs two extra wait() cycles before
 * pushing the result. With a producer that drives vld=1 every cycle, the
 * input buffer fills, vld arrives again before the next pop(), and the msg_dv_in
 * reader reports an overrun (SC_HLS-401) at whatever severity the
 * configured dv_overrun_policy dictates.
 */

#pragma once

#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(dut) {
public:
    using T = unsigned;

    sc_in<bool>  SC_NAMED(clk);
    sc_in<bool>  SC_NAMED(rst_bar);

    msg_dv_in<T>      SC_NAMED(in1);
    msg_dv_out<T>     SC_NAMED(out1);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void main() {
        out1.reset_push();
        in1.reset_pop();
        wait();

        while (1) {
            T x = in1.pop();
            // Two explicit waits guarantee the consumer is genuinely slow
            // (II=3). pop() and push() are zero-time once the protocol is
            // satisfied, so without these the loop would race ahead.
            wait();
            wait();
            out1.push(x);
        }
    }
};