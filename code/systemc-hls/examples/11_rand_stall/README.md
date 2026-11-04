# 11_rand_stall - Random Stall Injection

Demonstrates random stall injection on msg channels for verification.
Random stalls simulate back-pressure from downstream consumers by randomly
deasserting the `rdy` signal on msg channels, exercising the DUT's flow-control
handling under realistic pressure conditions.

## Key Concepts

- **`sc_hls::msg_lib::rand_stall::set(testbench, verbose)`** — enables random stall
  injection on every msg channel under the given hierarchy. With `verbose=true`
  the configured stall probability is printed for each channel.
- **`SC_HLS_RAND_SEED` env var** — when set, the stall sequence becomes
  reproducible (the same seed yields the same pattern of `rdy` deassertions).
- **Always compiled in** — random stall injection is dormant until activated;
  no compile flag is needed.

## Why Random Stalls Matter

In synthesised RTL, downstream consumers may not always be ready to accept
data. Random stall injection exercises the DUT's `vld`/`rdy` handshake under
back-pressure, catching bugs that only manifest when the consumer stalls.
Without stalls, a simulation may pass even if the DUT incorrectly assumes
the consumer is always ready.

## Design

```
in1 ──[msg_in]──┐
               │     ┌──────┐    ┌─[msg_out]─▶ out1   (= in1 + in2)
in2 ──[msg_in]──┼──▶  │ dut  │ ──┤
               │     └──────┘    └─[msg_out]─▶ out2   (= in1 + in3)
in3 ──[msg_in]──┘
```

## Files

- `dut.h` — `dut` module with three inputs and two outputs.
- `testbench.cpp` — `sc_main` and inline `testbench` with `stim`/`resp`/`reset`
  threads. Parses the optional CLI arg to toggle stall injection.
- `Makefile` — build configuration (uses `../common/Makefile.inc`),
  plus `run-stall` and `run-seed` convenience targets.

## Building and Running

The default `make run` invokes `./sim_sc 0` (no stalls):

```bash
make clean && make run
```

Other modes:

```bash
./sim_sc 1                        # random stalls (different each run)
SC_HLS_RAND_SEED=42 ./sim_sc 1  # reproducible stall pattern
make run-stall                    # equivalent to ./sim_sc 1
make run-seed                     # equivalent to SC_HLS_RAND_SEED=12345 ./sim_sc 1
```

## Expected Output

```
=== Random Stalls disabled ===
Stimulus started
Stimulus complete
All responses received
Simulation PASSED
```

With stalls enabled, the simulation takes more cycles but still passes:

```
=== Enabling Random Stalls ===
  in1: stall probability = 0.3
  in2: stall probability = 0.3
  in3: stall probability = 0.3
==============================
... (same Stimulus / responses)
Simulation PASSED
```

A `waves.vcd` file is also produced.

## Related Examples

- `01_xput_accurate` — same msg pattern without stalls.
- `10_logging` — capture every transaction, including stalled ones.