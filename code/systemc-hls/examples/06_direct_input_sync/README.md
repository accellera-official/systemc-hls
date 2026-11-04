# 06_direct_input_sync - Mixed Protocols (msg + Sync + Direct Inputs)

Demonstrates a design that combines three different communication
mechanisms in a single module:

- **`msg_in` / `msg_out`** for back-pressured streaming data.
- **`sync_in` / `sync_channel`** for explicit rendezvous synchronization.
- **`sc_in`** direct inputs for configuration / control values.

This pattern fits designs with explicit synchronization points between
processing phases and a per-iteration configuration update.

## Key Concepts

- **`sync_in<>`** — a rendezvous handshake (vld/rdy only, no data) used to
  release the DUT's main loop once per iteration.
- **`sc_vector<msg_in<...>>`** — multiple sample-data input ports declared
  via `sc_vector` to scale with `num_samples`.
- **`sc_vector<sc_in<...>>`** — multiple direct (signal-level) configuration
  input ports.
- The DUT runs nested `for` loops parameterised by the direct inputs and
  conditionally pushes to its msg_out output.

## Design

```
                          ┌───────────────────────── dut ─────────────────────────┐
sync_chan  ─[sync_in]────▶ sync barrier                                              │
sample_in[0..2] ─[msg_in]▶ sample_in[]                                               │
direct_inputs[0..7] ─────▶ direct_inputs[]   →   compute   ─[msg_out]── out1    ────▶│
                          └────────────────────────────────────────────────────────┘
```

For each iteration, the testbench:
1. Waits for `sync_chan.rdy == 1`,
2. Updates `direct_inputs[2..7]`,
3. Asserts `sync_chan.vld` to release the DUT's loop,
4. Pushes `num_samples × direct_inputs[0] × direct_inputs[1]` values into
   the sample channels.

The DUT computes a sum-of-products of the samples, multiplies by 10, and
pushes the result on `out1` only when it exceeds the threshold in
`direct_inputs[7]`.

## Files

- `dut.h` — `dut` module with mixed-protocol ports.
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

- `07_ping_pong_mem` — `sync_channel` paired with shared memory.
- `01_xput_accurate` — msg-only protocol without sync or direct inputs.