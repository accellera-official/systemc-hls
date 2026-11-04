# 02_dat_vld - Message Passing without Back-Pressure (msg_dv)

Demonstrates the msg_dv protocol (dat/vld): a one-way handshake using only
`vld` and `dat` signals. There is no `rdy`/back-pressure path from the
receiver to the sender, so the producer cannot be stalled.

## Key Concepts

- **`msg_dv_in<T>` / `msg_dv_out<T>`** — input/output ports with valid-only protocol.
- **Producer-driven** — data flows whenever the producer asserts `vld`.
- **No back-pressure** — the consumer cannot stall the producer; if a sample
  is missed, it is dropped.

## When to Use This Pattern

Use msg_dv when:
- The producer is naturally rate-limited (sensor outputs, streaming sources).
- The consumer is always ready to accept data.
- A simpler protocol than msg (dat/vld/rdy) is acceptable.

Use message passing with flow control (`msg_in`/`msg_out`) when the receiver needs to apply back-pressure.

## Design

```
in1 ──[msg_dv_in]──▶ dut ──[msg_dv_out]──▶ out1
```

The DUT pops each input value and pushes the same value to the output. Two
loops exercise different scheduling: the first runs at II=1, the second at
II=4 with intervening waits, demonstrating that msg_dv does not require the
producer and consumer to advance in lockstep.

## Files

- `dut.h` — `dut` module using `msg_dv_in`/`msg_dv_out` ports.
- `testbench.cpp` — `sc_main` and inline `testbench` with `stim`/`resp` threads.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced.

## Related Examples

- `01_xput_accurate` — full msg protocol with back-pressure.
- `03_nonblocking` — msg_in/msg_out with `pop_nb`/`push_nb` for opportunistic transfers.