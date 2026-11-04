# Negative Test: enable_port_access misuse

Verifies the guard rails around `enable_port_access()` on `msg_in` / `msg_out`:
the forbidden data-transfer API, the forbidden reset calls, and the
elaboration-phase restriction.

## Sub-tests

- **test_forbidden_pop** — a `msg_in` reader has `enable_port_access()` (in the
  constructor) and then calls `pop()`. Expect an `SC_HLS` error whose message
  contains "forbidden".
- **test_forbidden_push** — a `msg_out` writer has `enable_port_access()` and then
  calls `push()`. Expect an `SC_HLS` "forbidden" error.
- **test_reset_forbidden** — a writer has `enable_port_access()` and then calls
  `reset_push()`. Reset is forbidden under port access, so expect an `SC_HLS`
  "forbidden" error.
- **test_enable_after_elab** — a writer calls `enable_port_access()` from inside
  a process (after elaboration, during `SC_RUNNING`). Expect an `SC_HLS` error
  whose message contains "must be called during elaboration".

Each sub-test installs a capturing `sc_report_handler` (see `common.h`) that
records `SC_ERROR` / `SC_FATAL` reports without aborting, runs the simulation,
and returns PASS (exit 0) only if the expected `SC_HLS` message was captured.

## Files

- `common.h` — capturing report handler + `saw_sc_hls_error_containing()`.
- `testbench_forbidden_pop.cpp`, `testbench_forbidden_push.cpp`,
  `testbench_reset_forbidden.cpp`, `testbench_enable_after_elab.cpp` — the four
  sub-tests.
- `Makefile` — builds and runs all sub-tests via the `test` target.

## Running

```bash
make clean
make test
```

## Expected Output

```
PASS: test_forbidden_pop
PASS: test_forbidden_push
PASS: test_reset_forbidden
PASS: test_enable_after_elab

Results: 4 passed, 0 failed
```

The suite is also auto-discovered by `tests/run_all.py`.

## Related

- `examples/14_enable_port_access` — correct use on plain ports.
- `examples/15_enable_port_access_portless` — correct use on a portless channel
  (reader/writer split).
