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

// Case 2 Verilog top: owns the clock and reset, and acts as the testbench
// for the SystemC block (dut_wrap). Models a large Verilog design / TB with
// one SystemC subsystem in it.
//
// Under Verilator the SystemC block cannot be instantiated inside Verilog,
// so vtop exposes the connection as ports and sc_main.cpp wires vtop and
// dut_wrap side by side. Under commercial simulators, dut_wrap is instantiated here
// instead.
//
// R4: everything driven towards the SystemC block comes from posedge
// registers, so it never changes in the (edge-0.02ns, edge] window.
// timeprecision (1ps) must equal the SystemC time resolution.
`timescale 1ns/1ps
module vtop #(parameter N = 10) (
    output reg        clk,
    output reg        rst_n,
    input             in1_rdy,
    output            in1_vld,
    output     [31:0] in1_dat,
    output            out1_rdy,
    input             out1_vld,
    input      [31:0] out1_dat,
    output            done,
    output            ok
);
    // Clock: period 2ns, first posedge at 0ns. The sc_hls scheduler assumes
    // posedges at k*period (see dut_wrap.h), so the phase is 0.
    initial clk = 1'b1;
    always #1 clk = ~clk;

    // Reset: active low for the first 5 posedges.
    initial begin
        rst_n = 1'b0;
        repeat (5) @(posedge clk);
        rst_n <= 1'b1;
    end

    function [31:0] ref_value(input [31:0] i);
        ref_value = i * 7 + 3;
    endfunction

    reg        active;
    reg [31:0] cyc;
    reg [31:0] sent;
    reg [31:0] received;
    reg        err;

    always @(posedge clk or negedge rst_n)
        if (!rst_n) begin active <= 1'b0; cyc <= 0; end
        else        begin active <= 1'b1; cyc <= cyc + 1; end

    // Producer: sends ref_value(0..N-1) on in1.
    assign in1_vld = active && (sent < N);
    assign in1_dat = ref_value(sent);

    always @(posedge clk or negedge rst_n)
        if (!rst_n) sent <= 0;
        else if (in1_vld && in1_rdy) sent <= sent + 1;

    // Consumer: checks out1 in order; stalls one cycle in four.
    assign out1_rdy = active && (cyc[1:0] != 2'd3);

    always @(posedge clk or negedge rst_n)
        if (!rst_n) begin
            received <= 0;
            err <= 1'b0;
        end else if (out1_vld && out1_rdy) begin
            if (out1_dat !== ref_value(received)) begin
                $display("vtop: mismatch at item %0d: ref=%0d act=%0d",
                         received, ref_value(received), out1_dat);
                err <= 1'b1;
            end
            received <= received + 1;
        end

    assign done = (received == N);
    assign ok   = done && !err;

    always @(posedge done)
        $display("vtop: %0d items received, %s", received, err ? "FAIL" : "PASS");
endmodule
