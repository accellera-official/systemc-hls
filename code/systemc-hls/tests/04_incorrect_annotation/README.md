# Negative Test: Incorrect Annotation Detection

## Purpose

This test verifies that the sc_hls library correctly detects and reports an error
when `set_latency_capacity()` is called on a `msg_channel` instead of on the `msg_out`
port that is bound to it. Annotating the wrong object results in an inactive annotation
that has no effect on simulation — a silent misconfiguration that must be caught.

## Test Description

Back-annotation models HLS pipeline latency by delaying data on a `msg_out` port.
The annotation must be applied to the `msg_out` port (the object that calls
`reset_push()` and drives the Pre/Post scheduling), not to the `msg_channel` it is
bound to.

### The Violation

```cpp
// CORRECT: annotate the msg_out port
top_inst.block_inst.dout.set_latency_capacity(3, 3);

// INCORRECT (what this test does): annotate the msg_channel
top_inst.bout.set_latency_capacity(3, 3);
```

When `set_latency_capacity()` is called on `bout` (a `msg_channel`), the annotation
is registered in the channel_manager's annotation table. However, `bout` never calls
`reset_push()` — only the bound `msg_out` port does. At end of simulation, the
channel_manager's `validate_annotations()` detects that the annotation was registered
but never activated, and reports a `SC_HLS-ANNOTATE` error.

### Expected Behavior

When `sc_stop()` is called, the channel_manager should:
1. Scan the annotation table for entries where `was_reset == false`
2. Report a `SC_HLS-ANNOTATE` error for each inactive annotation
3. Provide a hint identifying the correct object to annotate

### Test Success Criteria

- **PASS**: `SC_HLS-ANNOTATE` error is reported at end of simulation
- **FAIL**: No error is reported (annotation silently ignored)

## Files

- `dut.h`: Design with a `msg_out` port (`block_inst.dout`) bound to a `msg_channel` (`bout`)
- `testbench.cpp`: Test harness that annotates `bout` (incorrect) and checks for the error
- `Makefile`: Build configuration

## Running the Test

```bash
make run
```

## Expected Output

```
SC_HLS-ANNOTATE: Annotation on inactive port/channel: top_inst.bout
  (latency=3, capacity=3)
  Annotations must be applied to ports that call reset_push(),
  not to channels they are bound to.
  Hint: If 'top_inst.bout' is a msg_channel, annotate the
  msg_out port that is bound to it instead.

PASS: SC_HLS-ANNOTATE error correctly detected for inactive annotation