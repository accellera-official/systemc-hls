# 03_nonblocking - Non-Blocking Operations

Demonstrates the non-blocking `pop_nb()` and `push_nb()` methods of the msg
protocol. Unlike blocking `pop()`/`push()`, non-blocking variants return
immediately, returning a `bool` to indicate whether the transfer succeeded.

## Key Concepts

- **`pop_nb(m)`** — returns `true` if data was available (the value is written
  into `m`), `false` otherwise.
- **`push_nb(m)`** — returns `true` if the data was accepted, `false` if the
  output buffer was full.
- **Polling** — a single thread can check multiple ports per cycle and react
  to whichever has data.
- **No blocking wait** — the thread continues immediately regardless of port
  state.

## When to Use This Pattern

- A thread needs to poll multiple input ports and process whichever has data.
- Implementing priority arbitration between competing data sources.
- Performing opportunistic transfers without committing to a wait.
- Building state machines that react differently based on port availability.

## Design

```
                +-----+
in1 --[pop_nb]-->|     |--[push]--> out1   (forwarded if got1)
                | dut |
in2 --[pop_nb]-->|     |--[push]--> out2   (forwarded if got2)
                +-----+
```

The DUT polls both inputs each cycle. Whichever port(s) have data are
forwarded to the corresponding output port. If neither has data, the thread
waits one cycle and tries again.

## Files

- `dut.h` — `dut` module using `pop_nb()` for multi-port polling.
- `testbench.cpp` — `sc_main` and inline `testbench` with independent stim/resp threads.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced.