# systemc-hls Release Notes

## 0.1.0 — public review (2026-11-17)

First public release of systemc-hls, the reference library for the draft
*SystemC High-level Synthesis Standard* of the Accellera SystemC Synthesis
Working Group (SWG). This is a **public-review pre-release**
(`SC_HLS_VERSION_PRERELEASE` = `"pub_rev"`); the API may change in response to
review feedback.

### What's included

- **Single header-only include** `<systemc-hls>`, namespace `sc_hls::msg_lib`.
- **Message passing with flow control** (dat/vld/rdy): `msg_in`, `msg_out`,
  `msg_channel`, with blocking and non-blocking `push`/`pop`/`peek_nb`.
- **Message passing without back-pressure** (dat/vld): `msg_dv_in`,
  `msg_dv_out`, `msg_dv_channel`, with a configurable overrun policy
  (`dv_overrun_policy`, SC_HLS-401; experimental, not part of the draft
  standard).
- **Sync (rendezvous)**: `sync_in`, `sync_out`, `sync_channel`.
- **FIFO**: `msg_fifo_module` (raw enq/deq ports) and `msg_fifo_channel`.
- **View selection by tag**: `sim_view_tag` (cycle-accurate),
  `tlm_view_tag` (fast functional, simulation-only), `default_view_tag`, and a
  vendor extension point for synthesis views.
- **Always-available features** (dormant until configured): VCD tracing,
  transaction logging (with a user-defined emitter extension point), random
  stall injection, latency back-annotation (text and JSON).
- **Direct pin access** via `enable_port_access()` for ports and portless
  channels.
- **Clock handling**: automatic `sc_clock` discovery, `add_clk()`, and
  `add_clock_alias()` for non-`sc_clock` clocks.
- **Built-in safety checks**: SC_HLS-101 (missing reset), SC_HLS-102
  (multiple writers / wrong-process access), SC_HLS-ANNOTATE (annotation file
  issues) and SC_HLS (inactive annotation at end of simulation).
- **17 examples** (`examples/`, run with `examples/run_all.py`) and **7 test
  suites** (`tests/`, run with `tests/run_all.py`).

### Tested configurations

- SystemC 2.3.4 and 3.0.1
- GCC 11.4
- Verilator 5.051 (example 17, mixed-language co-simulation)

### Known limitations and open topics

- The `dv_overrun_policy` API is experimental and not part of the draft
  standard.
- `tlm_view_tag` is simulation-only.
- Example 17: the SystemC hierarchical path prefix of an `sc_module`
  instantiated inside a Verilog top has not been verified yet.
- A common base class for modular ports is an open working-group topic.
