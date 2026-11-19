# 08_multi_clk - Multi-Clock Domain Crossing

Demonstrates a design that runs across two clock domains. The DUT operates
on a slow clock (`clk5`, 5 ns period); the testbench's response thread
runs on a fast clock (`clk1`, 1 ns period). msg channels handle the
clock-domain crossing automatically.

This example also exercises a **dynamic reset**: the testbench drops `rst_bar`
mid-simulation for 10 ns to verify that the DUT recovers correctly.

## Key Concepts

- **Multiple `sc_clock` instances** with different periods bound to different
  threads via `sensitive << clkN.posedge_event()`.
- **Automatic CDC** — msg channels synchronise across clock domains without
  requiring explicit double-flop or async-FIFO logic at the example level.
- **Dynamic reset** — `rst_bar` is asserted at start, deasserted, then
  re-asserted at ~25 ns for 10 ns to test mid-run reset handling.

## Design

```
testbench (clk1, 1 ns)        dut (clk5, 5 ns)
  ┌────────────────┐           ┌──────────┐
  │ stim @ clk5 ───┼─[msg_in]──▶│  proc    │──[msg_out]──┐
  │ resp @ clk1 ◀──┼───────────────────────────────────┘
  │ reset @ clk1   │           └──────────┘
  └────────────────┘
```

The DUT pops a value, adds `0x100`, and pushes it back. The two-stage clock
crossing happens entirely inside the msg channels.

## Files

- `dut.h` — `dut` module clocked on `clk5`.
- `testbench.cpp` — `sc_main` and inline `testbench` with two `sc_clock`s
  and a dynamic-reset thread.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced; both `clk1` and `clk5` will be visible
along with the dynamic reset pulse.

## Related Examples

- `01_xput_accurate` — single-clock msg design.
- `04_fifo` — buffering between processes (single clock).