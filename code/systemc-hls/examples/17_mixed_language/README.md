# 17_mixed_language - Wrappers for Verilog / SystemC co-simulation

Two self-contained cases show how a design written with the sc_hls library
meets RTL at a pins-only boundary. The pin names follow the §7.4.2
modular-port naming rule (`<port>_rdy`, `<port>_vld`, `<port>_dat`).

| Case | DUT | Testbench / top | Clock owner | Directory |
|---|---|---|---|---|
| 1 | Verilog (`vpass.v`) | SystemC (sc_hls driver/monitor) | SystemC `sc_clock` | `case1_vdut_sctb/` |
| 2 | SystemC (`dut.h`, `msg_in`/`msg_out`) | Verilog (`vtop.v`) | Verilog `always #1` | `case2_scdut_vtop/` |

Both cases use the same DUT: a 1-deep registered msg passthrough carrying
`uint32_t` values, with `in1` as input and `out1` as output.

## Running

```
make run        # both cases (Verilator --sc --timing)
make case1
make case2
```

It needs Verilator 5 with `--timing` support and `SYSTEMC_HOME`. If no
`verilator` is on `PATH`, the example prints `SKIPPED` and exits 0.

## Case 1: Verilog DUT, SystemC testbench

`vpass_wrap.h` is the wrapper a flow would generate around RTL: an
`sc_module` with only `sc_in`/`sc_out` pins, named after the RTL ports,
holding the Verilated model. The testbench binds the `rdy`/`vld`/`dat`
signals of its `msg_channel`s directly to these pins. Only the testbench end
of each channel is reset (`reset_push()` / `reset_pop()`), so the library
drives only that end and the Verilog drives the other.

Checked results:
- The monitor checks all 10 values in order and also counts them for
  completeness.
- The VCD shows one transfer every 2 cycles on both `in1` and `out1`
  (II=2), because the RTL deasserts `in1_rdy` while it holds a value. This
  matches examples/14.

## Case 2: SystemC DUT inside a Verilog top

The Verilog top owns the clock and reset. This models a large Verilog
design or TB that contains one SystemC subsystem. `dut_wrap.h` is the
wrapper around the SystemC block:

- **Pins only.** The DUT's `msg_in`/`msg_out` pins bind port-to-port to the
  wrapper ports. There is no `msg_channel`: the Verilog wiring is the
  channel.
- **Companion clock.** The sc_hls scheduler needs an `sc_clock`: it
  discovers clocks with `dynamic_cast<sc_clock*>` and places `pre()`/`post()`
  from the clock period. The Verilog clock is only a `bool` signal, so the
  wrapper holds a companion `sc_clock` and calls
  `add_clock_alias(companion, clk)` (see examples/13). The scheduler
  free-runs on the companion, so its period and first posedge are wrapper
  constructor parameters and must match the Verilog clock.
- **Clock checks.** `clk_check()` reports `SC_HLS-WRAP clock mismatch` at
  any Verilog posedge that is not a companion posedge. In
  `end_of_elaboration()`, a first posedge that is not a multiple of the
  period is rejected (see the limitation below).
- **Input timing.** The Verilog side must not change DUT inputs within
  0.02 ns before a clock edge (2×epsilon of the scheduler). Driving them
  from posedge registers is enough. The Verilog `timeprecision` (1ps) must
  equal the SystemC time resolution; Verilator aborts on a mismatch.

Verilator cannot instantiate an `sc_module` inside Verilog, so
`sc_main.cpp` instantiates `Vvtop` and `dut_wrap` side by side and connects
them with signals. Under commercial simulators, `dut_wrap` would be instantiated inside
`vtop.v` instead. That flow is TODO.

Checked results:
- The Verilog consumer checks all 10 values in order and stalls one cycle
  in four. It drives `done`/`ok`, which `sc_main` checks together with the
  SC error count.
- Negative checks were run by hand and are not committed. A companion with
  `first_posedge = 1ns` triggers the phase error plus a mismatch at every
  edge. A companion with `period = 4ns` triggers a mismatch at every other
  Verilog edge.

### Limitation found: the scheduler ignores the `sc_clock` start time

`channel_manager::run_for_clock()` (`include/internal/sc_hls_manager.h`)
waits one epsilon at time 0 and then loops on the period. It never reads
the clock's start time, so `pre()`/`post()` are always placed around
`k*period`. A pure-SystemC check with examples/14's testbench at a 2 ns
period gives:

| `sc_clock` start time | Result | Items received |
|---|---|---|
| 0 ns | PASS | every 2 ns (1 per cycle) |
| 1 ns | PASS | every 4 ns (1 per 2 cycles) |

So a clock with a non-zero phase is silently not throughput-accurate. In
the mixed-language case it was worse: data was corrupted (item 0 repeated).
This is why `dut_wrap` rejects such a phase. The library itself is not
changed in this example.

## Names (open item)

Log, annotation and rand-stall names use SystemC hierarchical paths: here
`wrap.dut_.in1` in case 2 and `tb_top.dut_.*` in case 1. The SystemC
path prefix of an `sc_module` instantiated inside a Verilog top has not been
verified yet.
