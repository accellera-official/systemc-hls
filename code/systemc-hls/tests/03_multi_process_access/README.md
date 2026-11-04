# Negative Test: Multi-Process Access Detection

## Purpose

This test suite verifies that the sc_hls library correctly enforces the
single-process ownership rule: each port must be accessed exclusively from the
process that called `reset_pop()` or `reset_push()` on it. Accessing a port from
a different process is a structural error that would cause non-deterministic
behavior in synthesized RTL.

## Test Description

The test is compiled into 7 separate binaries, each exercising a different scenario
via a preprocessor define. Five scenarios test illegal cross-process access (expected
to produce `SC_HLS-102` errors), and two test that repeated reset calls from the
same process are legal (expected to succeed cleanly).

### Sub-Tests

| Binary | Define | Scenario | Expected Result |
|--------|--------|----------|-----------------|
| `test_out_push` | `TEST_OUT_PUSH` | `push()` called from wrong process | `SC_HLS-102` error |
| `test_out_pushnb` | `TEST_OUT_PUSHNB` | `push_nb()` called from wrong process | `SC_HLS-102` error |
| `test_in_pop` | `TEST_IN_POP` | `pop()` called from wrong process | `SC_HLS-102` error |
| `test_in_popnb` | `TEST_IN_POPNB` | `pop_nb()` called from wrong process | `SC_HLS-102` error |
| `test_in_peeknb` | `TEST_IN_PEEKNB` | `peek_nb()` called from wrong process | `SC_HLS-102` error |
| `test_multi_reset_out` | `TEST_MULTI_RESET_OUT` | Same process calls `reset_push()` 5× | No error, clean exit |
| `test_multi_reset_in` | `TEST_MULTI_RESET_IN` | Same process calls `reset_pop()` 5× | No error, clean exit |

### Single-Process Ownership Rule

When a process calls `reset_pop()` or `reset_push()`, the channel_manager records that
process as the owner of the port. Any subsequent API call (`pop`, `push`, `pop_nb`,
`push_nb`, `peek_nb`) from a different process triggers a `SC_HLS-102` error.

Calling `reset_pop()` or `reset_push()` multiple times from the **same** process is
explicitly allowed (e.g., for multi-cycle reset sequences) and must not produce an error.

### Test Success Criteria

- **PASS (error tests)**: Simulation stops with `SC_HLS-102` error
- **PASS (reset tests)**: Simulation completes without any error
- **FAIL**: Expected error not produced, or unexpected error produced

## Files

- `dut.h`: DUT with configurable violation scenarios (selected by preprocessor define)
- `top.h`: Top-level module connecting DUT and testbench processes
- `testbench.cpp`: Test harness with per-scenario expected outcome
- `Makefile`: Builds all 7 binaries and runs each

## Running the Tests

```bash
make run
```

This builds and runs all 7 sub-tests. The test runner checks each binary's exit
code and error output to determine pass/fail.

## Expected Output (per sub-test)

For error-detection tests:
```
=== Test: msg_out_if push() from wrong process ===
Expected: SC_HLS-102 error
Error: SC_HLS-102: push() called from wrong process.
  Port: ...
  Expected process: ...
  Actual process: ...
  Each port must be accessed from only one process.
```

For reset-allowed tests:
```
=== Test: Multi-cycle reset for msg_out_if (same process calls reset_push 5x) ===
Expected: no error, clean exit
Simulation PASSED