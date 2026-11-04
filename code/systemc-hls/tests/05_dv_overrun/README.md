# Test 05: msg_dv_in Overrun Policy

## Purpose

Exercise the `sc_hls::msg_lib::dv_overrun_policy` machinery added to the `msg_dv_in`
reader. The msg_dv protocol (dat/vld) has no `rdy` back-pressure, so when the consumer
is slower than the producer, the reader's input buffer is overwritten with
each newly-arriving valid beat. This is bus-faithful (a real dat/vld wire would
lose the previous beat) and is legitimate for some designs (e.g. decimators)
but is also a common bug for tight 1:1 consumers.

The `dv_overrun_policy` lets the user choose what severity each overrun event
is reported at:

- `dv_overrun_policy::Error`
- `dv_overrun_policy::Warning`
- `dv_overrun_policy::Mute`    (library default, overruns are not reported)

The overrun policy is experimental: it is not part of the draft standard.

Configuration has two layers:

- **Global default**, set via `sc_hls::msg_lib::set_default_dv_overrun_policy(...)`.
- **Per-port override**, set via `port.set_overrun_policy(...)`. A port
  uses its local override if set; otherwise it follows the global default.

This suite verifies the entire matrix.

## Test Setup

All sub-tests share the same `dut` and `Top` (defined in `dut.h` /
`common.h`):

- The producer (`stim` thread in `Top`) calls `push(i); wait();` for 30
  iterations — vld is high every cycle.
- The dut's consumer thread does:

  ```cpp
  T x = in1.pop();
  wait();   // busy 1
  wait();   // busy 2
  out1.push(x);
  ```

  i.e. II=3. Two explicit `wait()` calls are required because `pop()` and
  `push()` are zero-time once the protocol is satisfied — without them the
  loop would race ahead and not produce any overruns.

The combination guarantees overrun events on the input channel.

A custom `sc_report_handler` (in `common.h`) captures every
`SC_HLS-401` report by severity, while suppressing the abort/throw
actions that `SC_ERROR` would normally trigger, so the simulation runs to
completion in every sub-test.

## Sub-tests

| Sub-test                                  | Global default | Per-port  | Pass condition                                      |
|-------------------------------------------|----------------|-----------|-----------------------------------------------------|
| `test_dv_overrun_default_error`           | Error          | -         | ≥1 SC_ERROR `SC_HLS-401`                       |
| `test_dv_overrun_global_mute_local_error` | Mute           | Error     | ≥1 SC_ERROR `SC_HLS-401` (local beats global)  |
| `test_dv_overrun_local_warning`           | (lib default = Mute) | Warning   | ≥1 SC_WARNING and 0 SC_ERROR `SC_HLS-401`      |
| `test_dv_overrun_global_mute`             | Mute           | -         | 0 `SC_HLS-401` reports of any severity         |

## Files

- `dut.h` — slow `msg_dv_in` consumer with two explicit `wait()`s.
- `common.h` — captured-error harness, `Top` (clock + producer + DUT + drain).
- `testbench_default_error.cpp`
- `testbench_global_mute_local_error.cpp`
- `testbench_local_warning.cpp`
- `testbench_global_mute.cpp`
- `Makefile` — builds and runs all four binaries.

## Running

```bash
make clean && make test
```

Expected output:

```
PASS: test_dv_overrun_default_error
PASS: test_dv_overrun_global_mute_local_error
PASS: test_dv_overrun_local_warning
PASS: test_dv_overrun_global_mute

Results: 4 passed, 0 failed
```

The suite is also auto-discovered by `tests/run_all.py`.