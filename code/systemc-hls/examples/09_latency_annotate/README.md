# 09_latency_annotate - HLS Pipeline Latency Back-Annotation

Demonstrates **back-annotation** of HLS pipeline latency on a message-passing design
with a divergence/reconvergence topology. The example also illustrates the
three supported ways to apply annotations: programmatically, from a text
file, or from a JSON file.

## Key Concepts

- **Back-annotation** — adding a latency/capacity model to a `msg_out`
  port so that simulation reflects the timing of the corresponding HLS
  pipeline stage.
- **Programmatic annotation** — `port.set_latency_capacity(latency, capacity)`
  called from `sc_main` after the testbench is built.
- **Text-file annotation** — `sc_hls::msg_lib::annotate::load_annotations(path)`
  parses a simple `<channel_path> <latency> [capacity]` format.
- **JSON-file annotation** — `sc_hls::msg_lib::annotate::load_annotations_json(path)`
  parses `{ "channels": { "<path>": { "latency": N, "capacity": M } } }`.
- Annotations are applied to the **port that calls `reset_push`** (here
  `block0_inst.dout` and `block1_inst.dout`). Applying them to the bound
  `msg_channel` would be inert because the Pre()/Post() schedule lives on
  the port that owns the push side.
- Back-annotation is always compiled in — there is no compile flag.

## Design

```
                    ┌──────────────── dut_top ──────────────────┐
                    │                                           │
din ──[msg_in]──▶ divergence ──▶ block0 ──▶ bout0 (3-cycle latency)
                    │                                          │
                    │              ▶ block1 ──▶ bout1 (3-cycle latency)
                    │                                  │
                    │                              reconvergence ──[msg_out]──▶ dout
                    └───────────────────────────────────────────┘
```

The `divergence` module fans out, the two `block`s perform the (latency-modelled)
work in parallel, and the `reconvergence` module sums the results.

## Files

- `dut.h` — five sub-modules (`divergence`, `block0`, `block1`,
  `reconvergence`) plus the wrapper `dut_top` that wires them.
- `testbench.cpp` — `sc_main` and inline `testbench`. Selects the annotation
  method via the optional CLI argument.
- `latencies.txt` — text-format annotation source.
- `latencies.json` — JSON-format annotation source.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

The default `make run` invokes `./sim_sc` with no arguments → no annotation:

```bash
make clean && make run
```

The four annotation methods are reachable via CLI args:

```bash
./sim_sc 0     # no annotations (default)
./sim_sc 1     # programmatic (set_latency_capacity calls)
./sim_sc 2     # text-file annotation (latencies.txt)
./sim_sc 3     # JSON-file annotation (latencies.json)
```

Channel paths in the annotation files use the canonical hierarchy:
`tb_top.dut_.bout0`, `tb_top.dut_.block0_inst.dout`, etc.

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
A `waves.vcd` file is also produced.

With latency=3 on `bout0` and `bout1`:
- Data pushed at cycle N appears at the reconvergence inputs at cycle N+3.
- Both inputs arrive at the same cycle (the two paths are kept in lock-step).

## Related Examples

- `01_xput_accurate` — single-pass message passing, no annotation.
- `04_fifo` — buffered intra-module communication.