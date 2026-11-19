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

// Case 1 DUT: 1-deep registered msg passthrough, written in Verilog.
//
// Port names follow the §7.4.2 modular-port naming rule (<port>_rdy,
// <port>_vld, <port>_dat), i.e. what an HLS tool emits for
//     msg_in<uint32_t>  in1;
//     msg_out<uint32_t> out1;
// in1_rdy/out1_vld are driven from registers, so they change only at the
// clock edge. A transfer happens on a posedge where vld && rdy.
// in1_rdy = !have, so the block cannot accept and emit in the same cycle:
// throughput is one item every 2 cycles (II=2), as in examples/14.
`timescale 1ns/1ps
module vpass (
    input             clk,
    input             rst_n,
    output            in1_rdy,
    input             in1_vld,
    input      [31:0] in1_dat,
    input             out1_rdy,
    output            out1_vld,
    output     [31:0] out1_dat
);
    reg        have;
    reg [31:0] hold;

    assign in1_rdy  = !have;
    assign out1_vld = have;
    assign out1_dat = hold;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            have <= 1'b0;
            hold <= 32'd0;
        end else if (have) begin
            if (out1_rdy) have <= 1'b0;    // output beat consumed
        end else if (in1_vld) begin
            hold <= in1_dat;               // input beat accepted
            have <= 1'b1;
        end
    end
endmodule
