# 12_xput_accurate_tlm - TLM Fast Simulation View

Demonstrates the **TLM (Transaction Level Modeling) view** of the Channels
library. Selecting `tlm_view_tag` for the channels makes them use
`tlm::tlm_fifo` internally instead of cycle-accurate signals, giving a
significantly faster functional simulation at the cost of cycle accuracy.

The DUT and testbench in this example are identical in structure to
`01_xput_accurate`, but templated on `ViewTag` so the same source can be
instantiated either as cycle-accurate (`sim_view_tag`, the default) or
fast-functional (`tlm_view_tag`).

## Key Concepts

- **`msg_in<T, ViewTag>` / `msg_out<T, ViewTag>`** — message ports parameterized
  on the view.
- **`tlm_view_tag`** — selects `tlm::tlm_fifo`-backed channels.
- **`sim_view_tag`** — selects signal-based, cycle-accurate channels.
- **`msg_channel<T, tlm_view_tag>(name, depth)`** — TLM channel with a
  configurable FIFO depth (here, 20).
- **Structured testbench** — like `01_xput_accurate`, this example uses
  separate `driver` and `monitor` modules instantiated as `drv_` / `mon_`
  inside `testbench`. Templating the helpers on `ViewTag` keeps the
  testbench compact.

## TLM vs Cycle-Accurate

| Aspect          | TLM View                 | Cycle-Accurate View         |
|-----------------|--------------------------|------------------------------|
| Speed           | Very fast                | Slower                       |
| Timing          | Functional only          | Cycle-accurate               |
| Channel impl.   | `tlm::tlm_fifo`          | `vld`/`rdy`/`dat` signals    |
| VCD tracing     | Not meaningful           | Full signal visibility       |
| Use case        | Early functional dev     | Performance / RTL correlation |

## Design

```
        +-----------+        +-----+        +-----------+
in1 --> |           | -----> |     | -----> |           | --> out1
in2 --> |  driver   | -----> | dut | -----> |  monitor  | --> out2
in3 --> |   (drv_)  | -----> |(dut_)|       |   (mon_)  |
        +-----------+        +-----+        +-----------+

DUT:  out1 = in1 * in2
      out2 = (in1 * in2) * in3

All channels: msg_channel<T, tlm_view_tag>(name, /*depth=*/20)
```

The testbench sends 40 input values (vs 10 in 01_xput_accurate); the FIFO
depth of 20 lets the driver run ahead of the DUT.

## Files

- `dut.h` — `dut<ViewTag>` (templated DUT module).
- `top.h` — `driver<ViewTag>`, `monitor<ViewTag>`, and `testbench` wiring
  it all together with `tlm_view_tag`.
- `testbench.cpp` — `sc_main`. No VCD: TLM has no signals to trace.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The simulation completes successfully and prints `Simulation PASSED`.
**No `waves.vcd` is produced** — TLM channels carry transactions, not
signals.

## When to Use TLM View

- Early functional development.
- Software-level system exploration where cycle-accurate timing is not needed.
- Regression suites that prioritize speed over timing fidelity.

For performance validation, RTL correlation, or detailed waveform debugging,
keep the default `sim_view_tag` (as in `01_xput_accurate`).

## Related Examples

- `01_xput_accurate` — the same design with cycle-accurate signals.
- `04_fifo` — explicit `msg_fifo` (different from a TLM-fifo-backed channel).

## Auxiliary Files

- `compute_expected.py` — small Python script that generated the expected
  output vector hard-coded in `monitor::proc()`. Documented here for
  reproducibility; not used at build/run time.