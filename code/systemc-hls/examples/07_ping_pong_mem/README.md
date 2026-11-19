# 07_ping_pong_mem - Ping-Pong Shared Memory via sync_channel

Demonstrates a classic ping-pong (double-buffer) memory pattern between two
threads that share a memory buffer. A `sync_channel` provides the rendezvous
that hands off ownership of one half of the memory from a producer thread
to a consumer thread.

## Key Concepts

- **`sync_channel<>`** — a vld/rdy-only handshake with no data payload, used
  here purely as a barrier between the producer and consumer.
- **`sync_out(mem)` / `sync_in(mem)`** — the producer signals completion of
  a write phase; the consumer waits for that signal before reading. The
  `mem` argument is a memory handle that the producer can update before
  releasing the barrier; it is observable by the consumer after `sync_in()`.
- **Ping-pong buffering** — `mem[0..7]` and `mem[8..15]` alternate between
  being filled and drained, allowing the two threads to operate concurrently.

## Design

```
                ┌────────────────────── dut ────────────────────┐
                │                                              │
in1 ─[msg_in]──▶ │ thread1: write 8 values → mem[half] →────────┐│
                │                                          sync││
                │ thread2: ◀────────── mem[other half] ◀───────┘│
                │                                          ▼   │
                │                                      out1 ──┐ │
                └──────────────────────────────────────────────┘
                                                            ▼
                                                          [msg_out]
```

`thread1` writes 8 values into one half of `mem`, then calls `sync1.sync_out(mem)`.
`thread2` waits via `sync1.sync_in(mem)`, then reads 8 values from the
just-finished half. They alternate halves on each iteration.

## Files

- `dut.h` — `dut` module with two threads, an internal `sync_channel`, and
  a 128-entry shared `mem`.
- `testbench.cpp` — `sc_main` and inline `testbench` with `stim`/`resp` threads.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced (showing the internal `sync1` channel).

## Related Examples

- `06_direct_input_sync` — `sync_in` for synchronisation across modules.
- `04_fifo` — alternate intra-module buffering using `msg_fifo`.