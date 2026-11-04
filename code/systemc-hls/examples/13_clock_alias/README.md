# 13_clock_alias - Aliasing a Non-`sc_clock` Clock

Demonstrates `sc_hls::add_clock_alias()`: running a design cycle-accurately when
its clock is a hand-toggled `sc_signal<bool>` rather than an `sc_clock`.

## Why this is needed

The sc_hls clock manager discovers clocks only by `dynamic_cast<sc_clock*>`
while walking the design hierarchy, and its pre/post scheduler needs a clock's
`period()` to place `pre()` one epsilon before, and `post()` one epsilon after,
each active edge (this is what makes simulation timing match HLS-generated RTL).

A plain `sc_signal<bool>` toggled by a thread is therefore invisible to the
library. A design driven only by such a signal fails at the first port access
with:

```
Error: SC_HLS: No clocks found - ensure at least one sc_clock exists in design
```

`add_clock_alias()` bridges the gap.

## Key Concepts

- **`add_clock_alias(companion_clock, toggled_signal)`** — create a companion
  `sc_clock` whose period and phase match the toggled signal, and alias the
  signal's posedge event to it. Processes statically sensitive to the toggled
  signal (here the DUT's `proc`, plus `stim`/`resp`) resolve into the companion
  clock's timing domain and are pre/post scheduled on its period.
- **Type-safe API** — the function takes the `sc_clock` and the
  `sc_signal_in_if<bool>` objects (not raw `sc_event`s) and extracts
  `.posedge_event()` internally, which prevents wrong-order / wrong-edge
  mistakes.

## Design

```
        +-----------+  in_chan   +-----+  out_chan  +-----------+
  stim  |  push 0..9| ---------> | dut | ---------> |  pop+check|  resp
        +-----------+            +-----+            +-----------+
                                    ^
                          clk (hand-toggled sc_signal<bool>)
                          aliased to  clk_alias (sc_clock, 2 ns)
```

`clkgen()` toggles `clk` with a posedge at t = 0, 2, 4, ... ns. The companion
`sc_clock clk_alias` has the same 2 ns period and phase. The DUT forwards each
of the 10 pushed values; `resp` checks them in order.

## Cycle-Accuracy Caveat

The pre/post scheduler free-runs on the companion `sc_clock`'s period and phase
— it is **not** synchronized to the toggled signal's actual edges. Correctness
requires the companion clock to match the signal in both period and phase. The
library does not currently validate this; the cross-check below confirms it.

## Files

- `dut.h` — msg pass-through `dut`, sensitive to `clk.pos()`; unaware its clock
  is aliased.
- `testbench.cpp` — `testbench` module (hand-toggled `clk`, companion
  `clk_alias`, the `add_clock_alias(clk_alias, clk)` call, stim/resp/reset) and
  `sc_main`.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced.

## Cycle-accuracy cross-check (optional)

Load `waves.vcd` and confirm every `clk` posedge coincides with a `clk_alias`
posedge (identical timestamps), and that the channel handshake (`vld`/`rdy`/
`dat`) transitions land on those edges — i.e. the aliased signal behaves exactly
as a native `sc_clock` would.

## Related Examples

- `01_xput_accurate` — the same msg protocol driven by a normal `sc_clock`.
- `08_multi_clk` — multiple `sc_clock` domains in one design.
