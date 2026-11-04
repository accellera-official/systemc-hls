# 04_fifo - FIFO Channel

Demonstrates `msg_fifo<T, N>`: a FIFO channel that decouples a producer thread
from a consumer thread inside a single module. The FIFO buffers up to `N`
elements, allowing the two threads to operate at different rates while the
vld/rdy back-pressure path keeps overflow / underflow safe.

## Key Concepts

- **`sc_hls::msg_lib::msg_fifo<T, N>`** — a FIFO of depth `N` carrying values of type `T`.
  Exposes two ports: `enq` (push side) and `deq` (pop side).
- Two threads (`main1` and `main2`) inside the same `dut` module communicate
  through the FIFO.
- vld/rdy back-pressure on `enq` and `deq` provides automatic flow control.

## Design

```
testbench → in1 ─▶ [main1]──enq──▶ msg_fifo(8) ──deq──▶[main2] ─▶ out1 → testbench
                          ⬑─────── dut ───────────⬐
```

`main1` reads from `in1` and pushes to the FIFO. `main2` pops from the FIFO
and writes to `out1`. The two threads are decoupled by the FIFO; if the
producer is faster, data accumulates up to depth=8.

## Files

- `dut.h` — `dut` containing two threads and a `msg_fifo<T, 8>`.
- `testbench.cpp` — `sc_main` and inline `testbench` with `stim`/`resp` threads.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced (showing the FIFO `vld`/`rdy`/`dat`
signals on both `enq` and `deq` sides).

## Related Examples

- `05_portless` — internal `msg_channel` (no buffering, just decoupling).
- `01_xput_accurate` — basic msg protocol without buffering.