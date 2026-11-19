# Negative Test: Missing sc_clock (Manually Generated Clock)

## Purpose

This test verifies that the sc_hls library correctly detects a design that
drives its clock by hand — toggling a plain `sc_signal<bool>` in a thread
— instead of using `sc_clock`. This is a real-world mistake: a testbench
author writes their own clock generator process such as:

```cpp
while (1) {
    wait(1, SC_NS); clk_sig = 0;
    wait(1, SC_NS); clk_sig = 1;
}
```

and connects it to a DUT whose processes are correctly sensitive to
`clk_sig.posedge_event()`.

### Why this fails

`sc_hls`'s `clock_manager::find_clocks()` (in
`include/internal/sc_hls_manager.h`) discovers clocks by walking the design
hierarchy and testing `dynamic_cast<sc_clock*>(obj)` on each object. A
hand-toggled `sc_signal<bool>` is not an `sc_clock`, so it is never added to
`clock_vector` — regardless of how many processes are correctly sensitive
to its `posedge_event()`.

At first port access (`reset_pop()`/`reset_push()`), the library lazily
starts its channel manager, which requires at least one registered clock.
Finding none, it reports:

```
Error: SC_HLS: No clocks found - ensure at least one sc_clock exists in design
```

via `SC_REPORT_ERROR("SC_HLS", ...)`. This is a generic (un-numbered)
`SC_HLS` message type, distinct from the numbered `SC_HLS-101` /
`SC_HLS-102` / `SC_HLS-401` codes used elsewhere in the library. With the
default `SC_ERROR` action of `SC_DISPLAY | SC_THROW`, the error unwinds as
a C++ exception rather than aborting the process outright.

### Test Success Criteria

- **PASS**: An `SC_ERROR` with msg_type `SC_HLS` and message containing
  "No clocks found" is reported.
- **FAIL**: The simulation runs without detecting the missing clock.

## Files

- `dut.h` — plain `msg_in`/`msg_out` pass-through DUT with correct reset behavior; the
  clock misconfiguration is entirely in `top.h`, not here.
- `top.h` — `Top` module with a hand-toggled `sc_signal<bool> clk_sig`
  (no `sc_clock` anywhere in the design) driving `dut`, `driver`, and
  `reset_proc`.
- `testbench.cpp` — test harness expecting the `SC_HLS` "No clocks found"
  error.
- `Makefile` — build configuration.

## Running the Test

```bash
make clean
make test
```

## Expected Output

```
PASS: test_generated_clock

Results: 1 passed, 0 failed
```

The suite is also auto-discovered by `tests/run_all.py`.
