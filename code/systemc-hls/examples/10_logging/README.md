# 10_logging - Transaction Logging

Demonstrates the transaction-logging facility of the sc_hls library **and** how
a library user extends it with a custom output format. Every `push` / `pop` on
every channel under the testbench hierarchy is captured to a file.

The library ships **one** built-in format (a pipe-delimited text line). It does
not standardize on any structured format; instead the format is a compile-time
**extension point**. This example ships two user-defined formats — JSON Lines
and CSV — as a demonstration of that extension mechanism.

## Basic logging (built-in format)

```cpp
sc_hls::msg_lib::channel_logs logs;
sc_hls::msg_lib::emitter_options opts;   // { log_direction, unbuffered }
logs.enable("example_log.txt", opts);    // one file, caller picks the name/extension
logs.log_hierarchy(top);                 // auto-discovers every channel under `top`
```

Each line is `<port> | <dir> | <data> | <time>`:

```
tb_top.in_chan | PUSH | 0 | 60 ns
tb_top.dut_.in1 | POP | 0 | 70 ns
```

## Extending the format (user emitters)

The active emitter is selected at **compile time** by the `SC_HLS_EMITTER`
macro. A user format is a specialization of
`sc_hls::msg_lib::transaction_emitter<Message, Tag>` on a user-defined tag type:

```cpp
struct json_emitter_tag {};   // declared BEFORE <systemc-hls>

template <typename Message>
struct sc_hls::msg_lib::transaction_emitter<Message, json_emitter_tag> {
    static void emit(std::ostream* os, const Message& m, int id,
                     const std::string& port_name, const std::string& direction,
                     const sc_hls::msg_lib::emitter_options& opts) {
        /* write one line however you like */
    }
};
```

Then build the design with `-DSC_HLS_EMITTER=json_emitter_tag`. See
`custom_emitters.h` for the full JSON and CSV emitters used here.

**Note:** the tag type must be declared *before* `<systemc-hls>` is included —
`SC_HLS_EMITTER` is substituted into the library template at definition time, so
the name must already be visible. `custom_emitters.h` shows the required order.

Because the emitter is chosen at compile time, there is no runtime cost on the
transaction path and no per-transaction indirection. The trade-off is that the
format is fixed per build: switching formats is a rebuild, not a runtime flag.

## Design

```
in_chan ──[msg_in]──▶ dut ──[msg_out]──▶ out_chan
                       └── result = val * val
```

The DUT pops a 16-bit value, squares it, and pushes the 32-bit result. The
testbench pushes 0..19; the monitor pops, checks `val == i*i`, and calls
`sc_stop()` after the 20th value.

## Files

- `dut.h` — simple squaring DUT.
- `custom_emitters.h` — **user** code: `json_emitter_tag` / `csv_emitter_tag`
  and their `transaction_emitter` specializations (not part of the library).
- `testbench.cpp` — `sc_main` enables `channel_logs` and runs the simulation.
  The output filename comes from the `LOG_FILENAME` macro set by the Makefile.
- `Makefile` — `run` / `run-text` (built-in), plus `run-jsonl` / `run-csv`
  which clean-rebuild with the corresponding `SC_HLS_EMITTER` tag.

## Building and Running

Each format is a separate build (the emitter is compile-time):

```bash
make run          # built-in text  -> example_log.txt
make run-text     # same as `make run`
make run-jsonl    # user JSON Lines -> example_log.jsonl
make run-csv      # user CSV        -> example_log.csv
make distclean    # remove binaries + all example_log.* artifacts
```

## Expected Output

The simulation prints `Simulation PASSED` and names the log file written. A
`waves.vcd` file is also produced.

## Related Examples

- `01_xput_accurate` — same DUT shape, no logging.
- `11_rand_stall` — exercise flow-control with random stalls.
