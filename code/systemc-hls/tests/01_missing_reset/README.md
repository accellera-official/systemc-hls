# Negative Test: Missing Reset Call

## Purpose

This test verifies that the sc_hls library safety assertion mechanism correctly detects when a port is used without being properly reset.

## Test Description

The test is based on example 01 (xput_accurate) but with an intentional bug: the `out2.reset_push()` call is omitted in the DUT's reset function.

### Expected Behavior

When the DUT attempts to use `out2.push()` without first calling `out2.reset_push()`, the sc_hls library should:
1. Detect the missing reset call
2. Generate an error message
3. Stop the simulation

### Test Success Criteria

- **PASS**: Simulation stops with an error indicating the missing reset
- **FAIL**: Simulation runs without detecting the error

## Files

- `dut.h` - DUT with intentionally missing `out2.reset_push()` call
- `top.h` - Top-level module connecting driver, DUT, and monitor
- `testbench.cpp` - Test harness that expects an error to occur
- `Makefile` - Build configuration

## Running the Test

```bash
make clean
make run
```

## Expected Output

The test should output:
```
Negative test PASSED - Error was correctly detected
```

And the simulation should report an error about the missing reset call on `out2`.

## Implementation Details

The bug is in `dut.h` at line 40:
```cpp
void reset()
{
    in1.reset_pop();
    in2.reset_pop();
    in3.reset_pop();
    out1.reset_push();
    // INTENTIONALLY MISSING: out2.reset_push();
    wait();
}
```

When `proc()` later calls `out2.push(v2)`, the safety assertion should trigger because `out2` was never reset.