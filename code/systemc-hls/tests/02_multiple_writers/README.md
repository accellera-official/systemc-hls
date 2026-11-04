# Negative Test: Multiple Writers Detection

## Purpose

This test verifies that the sc_hls library correctly detects and reports an error
when two processes attempt to write to the same channel — a structural violation that
would cause undefined behavior in synthesized RTL.

## Test Description

The test creates a scenario with two writers on the same `msg_channel`:

1. A `Producer` submodule has a `msg_out` port bound to `chan`
2. The parent module also has a `SC_CTHREAD` that calls `chan.reset_push()`

When the parent process calls `chan.reset_push()`, the channel_manager detects that
the underlying signal is already driven by the `Producer`'s port and reports a
`SC_HLS-102` error.

### Expected Behavior

When the second writer calls `reset_push()` on a channel that already has a registered
writer, the sc_hls library should:
1. Detect the signal conflict
2. Report a `SC_HLS-102` error
3. Stop the simulation

### Test Success Criteria

- **PASS**: Simulation stops with a `SC_HLS-102` error indicating multiple writers
- **FAIL**: Simulation runs without detecting the conflict

## Files

- `dut.h`: `Producer` submodule that writes to a `msg_out` port
- `top.h`: Top-level module that also writes to the same channel (the violation)
- `testbench.cpp`: Test harness that expects the error to occur
- `Makefile`: Build configuration

## Running the Test

```bash
make run
```

## Expected Output

```
=== Multiple Writers Detection Test ===
Expected: SC_HLS-102 error when multiple writers detected

Error: SC_HLS-102: Multiple writers detected on same signal.
  First writer registered by process: ...
  Second writer registered by process: ...
  Each signal can only have one writer.
```

The test runner checks that the `SC_HLS-102` error was reported.