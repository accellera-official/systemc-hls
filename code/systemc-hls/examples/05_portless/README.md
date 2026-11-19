# 05_portless - Internal (Portless) Channels

Demonstrates `msg_channel` used as a **portless** channel — declared as a
member of a module rather than at its boundary, and bound directly by two
threads inside the same module. This is a common pattern for pipelining
inside a single module.

## Key Concepts

- **`msg_channel<T>`** can be used both as a port-binding object (between
  modules) and as a portless internal channel (within one module).
- The same channel object is `reset_push`-ed by one thread and `reset_pop`-ed
  by another.
- No port binding is required — the channel is just a member variable.

## When to Use This Pattern

- Two threads within the same module need to communicate.
- Modeling pipeline stages inside a single module boundary.
- The channel does not need to be visible at the module's port interface.

Use port channels (`msg_in`/`msg_out`) when communication crosses a module
boundary instead.

## Design

```
                +------------------- dut -------------------+
                |                                           |
in1 ──[msg_in]──▶| main1 ──▶ msg_channel<uint32> chan1 ──▶ main2 |──[msg_out]──▶ out1
                |                                           |
                +-------------------------------------------+
```

`main1` reads from the external `in1` port and pushes to the internal
`chan1`. `main2` pops from `chan1` and writes to the external `out1` port.
`trace_hierarchy(...)` automatically picks up the internal `chan1` so its
`vld`/`rdy`/`dat` signals appear in `waves.vcd`.

## Files

- `dut.h` — `dut` module with two threads and an internal `msg_channel`.
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

- `04_fifo` — buffered version (`msg_fifo`) of intra-module communication.
- `01_xput_accurate` — msg_in/msg_out ports across module boundaries.