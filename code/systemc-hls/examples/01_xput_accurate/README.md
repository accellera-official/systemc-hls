# 01_xput_accurate - Throughput-Accurate Message Passing

Demonstrates the core msg protocol (dat/vld/rdy) and throughput-accurate
modeling capability of the sc_hls library. This is the first example, and
also the one that introduces the **structured testbench** pattern: a separate
`driver` and `monitor` module instead of inline stim/resp threads.

## Key Concepts

- **`msg_in<T>` / `msg_out<T>`** — message ports with full valid/ready handshake.
- **Throughput-accurate** — simulation timing matches HLS-generated RTL, so the
  example is suitable for early performance validation.
- **Structured testbench** — `top.h` declares helper modules `driver` and
  `monitor` instantiated as `drv_` / `mon_` inside `testbench`. Most other
  examples (02–11) use the simpler inline `stim`/`resp` pattern.

## Design

```
        +-----------+        +-----+        +-----------+
in1 --> |           | -----> |     | -----> |           | --> out1
in2 --> |  driver   | -----> | dut | -----> |  monitor  | --> out2
in3 --> |   (drv_)  | -----> |(dut_)|       |   (mon_)  |
        +-----------+        +-----+        +-----------+

DUT:  out1 = in1 * in2
      out2 = (in1 * in2) * in3
```

Three 16-bit inputs, two 32-bit outputs. The monitor compares against a
pre-computed expected vector and calls `sc_stop()` on completion.

## Files

- `dut.h` — `dut` module (the DUT).
- `top.h` — `driver`, `monitor`, and `testbench` modules.
- `testbench.cpp` — `sc_main`.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced.

## Related Examples

- `02_dat_vld` — same message passing but without ready (msg_dv, no back-pressure).
- `12_xput_accurate_tlm` — the same pattern templated on `ViewTag` to swap
  cycle-accurate signals for fast TLM FIFOs.