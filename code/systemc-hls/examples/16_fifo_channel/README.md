# 16_fifo_channel - FIFO Channel Wrapper

Demonstrates `msg_fifo_channel<T, N>`: a FIFO wrapped as a pair of ready-to-use
channels (`in` / `out`). Two threads inside a single `dut` module communicate
directly through the channel — one pushes into `fifo_chan.in`, the other pops
from `fifo_chan.out` — without ever binding or managing the FIFO's raw
`enq`/`deq` ports. The FIFO buffers up to `N` elements, and the vld/rdy
back-pressure path keeps overflow / underflow safe.

## Key Concepts

- **`sc_hls::msg_lib::msg_fifo_channel<T, N>`** — wraps a
  `msg_fifo_module<T, N>` plus two `msg_channel<T>`s (`in`, `out`), binding the
  FIFO's `enq`/`deq` internally. User code only ever touches `in` / `out`.
- Contrast with `04_fifo`, which uses `msg_fifo_module` directly and wires its
  `enq`/`deq` to separate channels in the design.
- vld/rdy back-pressure on both channels provides automatic flow control.

## Design

```
testbench → in1 ─▶ [producer]──push──▶ fifo_chan.in ─▶ msg_fifo(8) ─▶ fifo_chan.out ──pop──▶[consumer] ─▶ out1 → testbench
                              ⬑──────────────────── dut ────────────────────────⬐
```

`producer` reads from `in1` and pushes to `fifo_chan.in`. `consumer` pops from
`fifo_chan.out` and writes to `out1`. The two threads are decoupled by the
FIFO; if the producer is faster, data accumulates up to depth=8.

## Files

- `dut.h` — `dut` containing two threads and a `msg_fifo_channel<T, 8>`.
- `testbench.cpp` — `sc_main` and inline `testbench` with `stim`/`resp` threads.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
The `resp` thread logs values `0x0`..`0x13` (0..19) in order.
A `waves.vcd` file is also produced (showing the `in`/`out` channel
`vld`/`rdy`/`dat` signals and the internal FIFO).

## Related Examples

- `04_fifo` — the same buffered decoupling using `msg_fifo_module` directly,
  with producer/consumer as separate modules.
