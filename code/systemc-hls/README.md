# systemc-hls 0.1.0 (public review)

Reference library for the draft *SystemC High-level Synthesis Standard*
(Accellera SystemC Synthesis Working Group).

A communication library for SystemC HLS designs that enables throughput-accurate modeling using explicit handshake protocols.

The library is systemc-hls (`sc_hls`). Include it with `#include <systemc-hls>`. All
message-passing types (ports, channels, interfaces, view tags, logging,
annotation, random-stall) live in namespace `sc_hls::msg_lib`; library-wide
utilities (e.g. `add_clk`) are at the top-level `sc_hls` namespace. The
sub-namespaces `sc_hls::axi_lib` and `sc_hls::mem_lib` are reserved for future
use. User code typically opens both namespaces:

```cpp
#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;
```

## Overview

The sc_hls library provides a set of communication primitives for SystemC-based hardware designs. It supports multiple protocols and enables cycle-accurate simulation while maintaining synthesizability for high-level synthesis (HLS) tools.

## Key Features

- **Multiple Protocols**:
  - Message passing with flow control (`msg_*`, dat/vld/rdy): full flow control with back-pressure
  - Message passing without back-pressure (`msg_dv_*`, dat/vld): valid-only
  - Sync: Rendezvous synchronization without data transfer

- **Throughput-Accurate Simulation**: Pre/Post scheduling ensures timing matches synthesized RTL

- **Tool-Agnostic**: No vendor-specific pragmas or dependencies

- **Modular Design**: Clean separation of protocols, channels, and ports

- **Always-Available Features** (dormant until configured):
  - Transaction logging
  - Random stall injection for verification
  - Back-annotation for performance modeling
  - VCD tracing support

- **Built-in Safety Checks**:
  - `SC_HLS-101`: Missing reset detection (fatal error)
  - `SC_HLS-102`: Multiple writers / wrong process access detection
  - `SC_HLS-ANNOTATE`: Inactive annotation detection at end of simulation

## Quick Start

### Basic Usage

```cpp
#include <systemc-hls>

using namespace sc_hls;
using namespace sc_hls::msg_lib;

SC_MODULE(Producer) {
    sc_in<bool> clk;
    sc_in<bool> rst;
    msg_out<uint32_t> out;  // Uses default_view_tag (sim_view_tag)

    SC_CTOR(Producer) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst, false);
    }

    void run() {
        out.reset_push();
        wait();

        while (true) {
            uint32_t data = compute_data();
            out.push(data);
        }
    }
};

SC_MODULE(Consumer) {
    sc_in<bool> clk;
    sc_in<bool> rst;
    msg_in<uint32_t> in;  // Uses default_view_tag (sim_view_tag)

    SC_CTOR(Consumer) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst, false);
    }

    void run() {
        in.reset_pop();
        wait();

        while (true) {
            uint32_t data = in.pop();
            process_data(data);
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;

    Producer prod("prod");
    Consumer cons("cons");
    msg_channel<uint32_t> chan("chan");  // Uses default_view_tag (sim_view_tag)

    prod.clk(clk);
    prod.rst(rst);
    prod.out(chan);

    cons.clk(clk);
    cons.rst(rst);
    cons.in(chan);

    // Run simulation
    rst = 0;
    sc_start(1, SC_NS);
    rst = 1;
    sc_start(100, SC_NS);

    return 0;
}
```

## View Tags

The library uses template type tags to select between different channel implementations:

- **`sim_view_tag`** - Standard cycle-accurate simulation view
  - Signal-based communication with full msg/msg_dv protocol support
  - Supports random stall injection and back-annotation
  - Default view for synthesis and simulation

- **`tlm_view_tag`** - Fast TLM-based simulation view (simulation-only)
  - Uses `tlm::tlm_fifo` internally for fast functional simulation
  - Useful for software development, testing, and early system exploration
  - Not available in synthesis context

- **`default_view_tag`** - Automatically selects the appropriate default
  - Resolves to `sim_view_tag` in both simulation and synthesis contexts
  - Used when no explicit view tag is specified

### Using View Tags

```cpp
// Explicit sim_view_tag (cycle-accurate)
msg_in<uint32_t, sim_view_tag> in;
msg_out<uint32_t, sim_view_tag> out;
msg_channel<uint32_t, sim_view_tag> chan;

// Explicit tlm_view_tag (fast simulation)
msg_in<uint32_t, tlm_view_tag> in;
msg_out<uint32_t, tlm_view_tag> out;
msg_channel<uint32_t, tlm_view_tag> chan;

// Default (uses default_view_tag = sim_view_tag)
msg_in<uint32_t> in;
msg_out<uint32_t> out;
msg_channel<uint32_t> chan;
```

### Vendor Extension Point

HLS tool vendors can define their own view tags to provide tool-specific channel implementations for synthesis. This enables portable designs that work with any vendor's HLS tool.

**How vendors define custom tags:**

```cpp
// In vendor header (e.g., vendor_channels.h)
namespace sc_hls { namespace msg_lib {
    struct vendor_synth_tag {};  // Vendor-specific tag
}

// Then provide partial specializations for ports/channels
template <typename Message>
class msg_in<Message, vendor_synth_tag> {
    // Vendor-specific implementation
};
```

**How users select vendor tags:**

```cpp
// Option 1: Explicit tag in code
msg_in<uint32_t, vendor_synth_tag> in;

// Option 2: Set as default for synthesis context
// In vendor header:
#ifdef __SYNTHESIS__
#define SC_HLS_DEFAULT_SYNTH_VIEW_TAG vendor_synth_tag
#endif
#include <systemc-hls>

// Then user code uses default:
msg_in<uint32_t> in;  // Uses vendor_synth_tag in synthesis
```

**Parameterizable designs** allow the same code to work with any view:

```cpp
template <typename ViewTag = default_view_tag>
SC_MODULE(MyDesign) {
    msg_in<uint32_t, ViewTag> in;
    msg_out<uint32_t, ViewTag> out;
    // ...
};

// Instantiate for simulation
MyDesign<sim_view_tag> sim_design("sim");

// Instantiate for vendor synthesis
MyDesign<vendor_synth_tag> synth_design("synth");
```

## Protocols

### Message passing with flow control (dat/vld/rdy)

Full vld/rdy handshake with back-pressure support:
- `msg_channel<T, ViewTag = default_view_tag>` - Channel
- `msg_in<T, ViewTag = default_view_tag>` - Input port
- `msg_out<T, ViewTag = default_view_tag>` - Output port

**API**:
- `push(data)` - Blocking write
- `push_nb(data)` - Non-blocking write (returns bool)
- `pop()` - Blocking read
- `pop_nb(data)` - Non-blocking read (returns bool)
- `peek_nb(data)` - Non-blocking peek (returns bool)
- `reset_push()` - Reset writer
- `reset_pop()` - Reset reader

### Message passing without back-pressure (dat/vld)

Valid-only signaling without back-pressure; the consumer must accept data
every cycle when valid is asserted:
- `msg_dv_channel<T, ViewTag = default_view_tag>` - Channel
- `msg_dv_in<T, ViewTag = default_view_tag>` - Input port
- `msg_dv_out<T, ViewTag = default_view_tag>` - Output port

**API**: Same as `msg_in` / `msg_out`. A new beat arriving while the reader's
buffer is still occupied overwrites it; such overruns are reported (SC_HLS-401)
at a severity selected by the experimental `dv_overrun_policy` (default: Mute).

### Sync (Rendezvous)

Synchronization without data transfer:
- `sync_channel<ViewTag = default_view_tag>` - Channel
- `sync_in<ViewTag = default_view_tag>` - Input port
- `sync_out<ViewTag = default_view_tag>` - Output port

**API**:
- `sync_in::sync()` / `sync_out::sync()` - Wait for rendezvous (ports)
- `sync_channel::sync_read()` / `sync_channel::sync_write()` - rendezvous on a portless channel
- `reset_pop()` - Reset
- `reset_push()` - Reset

## File Organization

```
include/
├── systemc-hls              # Main header (include this; extensionless)
├── sc_hls_view_tag.h        # View tags (sim_view_tag, tlm_view_tag, default_view_tag)
├── sc_hls_interfaces.h      # Interface definitions
├── sc_hls_msg.h             # msg_in / msg_out / msg_channel (declarations)
├── sc_hls_msg_dv.h          # msg_dv_in / msg_dv_out / msg_dv_channel (declarations)
├── sc_hls_sync.h            # sync_in / sync_out / sync_channel (declarations)
├── sc_hls_fifo_module.h     # FIFO implementation (raw enq/deq ports)
├── sc_hls_fifo_channel.h    # FIFO wrapped as in/out channels
├── sc_hls_tracing.h         # VCD tracing
├── sc_hls_logging.h         # Transaction logging
├── sc_hls_rand_stall.h      # Random stall injection
├── sc_hls_annotate.h        # Back-annotation
├── sim_views/               # Simulation-view implementations (not visible under __SYNTHESIS__)
│   ├── sc_hls_msg_sim_view.h
│   ├── sc_hls_msg_dv_sim_view.h
│   └── sc_hls_sync_sim_view.h
└── internal/                # Implementation details (do not include directly)
    ├── sc_hls_msg_protocol.h
    ├── sc_hls_msg_dv_protocol.h
    ├── sc_hls_manager.h
    ├── sc_hls_sim_helper.h
    ├── sc_hls_synth_util.h
    ├── sc_hls_tracing_sfinae.h
    └── sc_hls_ver.h
```

## Building

The library is header-only. Simply include `<systemc-hls>` and ensure SystemC is in your include path:

```bash
g++ -std=c++11 -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 \
    -o sim testbench.cpp -lsystemc
```

For the bundled examples, each one is a thin Makefile that includes the
shared rules in `examples/common/Makefile.inc`:

```make
include ../common/Makefile.inc
```

That single line gives every example a consistent `make`, `make run`, and
`make clean` target. See [`examples/common/Makefile.inc`](examples/common/Makefile.inc)
for the full build rules and overridable variables (`SYSTEMC_HOME`,
`TARGET`, `SRCS`, etc.).

## Requirements

- SystemC 2.3.4 or later
- C++11 compiler

## Always-Available Features

All features are always compiled in but remain dormant until explicitly configured. No compile-time flags are required.

### Transaction Logging

```cpp
#include <systemc-hls>

// Enable logging at runtime
sc_hls::msg_lib::channel_logs logs;
logs.enable("transactions");
logs.log_hierarchy(top);
```

The library ships a single built-in output format (pipe-delimited text). The
format is a compile-time extension point: define a tag type, specialize
`sc_hls::msg_lib::transaction_emitter<Message, Tag>`, and select it with
`-DSC_HLS_EMITTER=<tag>`. See `examples/10_logging` for user-written JSON and
CSV emitters. The working group does not standardize on any structured format.

### Random Stall Injection

```cpp
#include <systemc-hls>

// Enable random stalls on all channels under a hierarchy (verbose = true
// prints the per-channel stall probabilities)
sc_hls::msg_lib::rand_stall::set(top, /*verbose=*/true);

// The random seed can be set with an environment variable
// SC_HLS_RAND_SEED=12345 ./sim
```

### Back-Annotation

```cpp
#include <systemc-hls>

// Load annotations at runtime (text or JSON format)
sc_hls::msg_lib::annotate::load_annotations("latencies.txt");
sc_hls::msg_lib::annotate::load_annotations_json("latencies.json");
```

Annotations are validated at end of simulation - any loaded but unused annotations trigger a warning.

## Examples

See the `examples/` directory for 17 complete, runnable examples. They share
a common testbench skeleton:

- A single top-level class `SC_MODULE(testbench)` instantiates the DUT (`dut_`)
  and any helper modules.
- `sc_main` constructs `auto tb_top = std::make_shared<testbench>("tb_top");`
  so the SystemC/VCD hierarchy root is `tb_top`.
- Tracing uses `trace_hierarchy(tb_top.get(), tf)` writing `waves.vcd`
  (except 12, which is TLM-only and has no signals to trace).
- Pass/fail is reported with the canonical `Simulation PASSED` /
  `Simulation FAILED` sentinel.

| Example | Description |
|---------|-------------|
| `01_xput_accurate/` | Throughput-accurate message passing with structured `driver`/`monitor` testbench |
| `02_dat_vld/` | Message passing without back-pressure (`msg_dv_*`) — valid only |
| `03_nonblocking/` | Non-blocking operations (`push_nb`, `pop_nb`) |
| `04_fifo/` | `msg_fifo_module<T,N>` for buffered intra-module communication |
| `05_portless/` | Internal (portless) `msg_channel` between threads of one module |
| `06_direct_input_sync/` | Mixed protocols: `msg_*` + `sync_channel` + direct `sc_in` |
| `07_ping_pong_mem/` | Ping-pong shared memory via `sync_channel` rendezvous |
| `08_multi_clk/` | Multi-clock-domain crossing + dynamic reset |
| `09_latency_annotate/` | HLS pipeline latency back-annotation (programmatic / text / JSON) |
| `10_logging/` | Transaction logging (built-in text) + custom-format extension (JSON / CSV) |
| `11_rand_stall/` | Random stall injection for back-pressure verification |
| `12_xput_accurate_tlm/` | TLM (`tlm::tlm_fifo`) view for fast functional simulation |
| `13_clock_alias/` | Aliasing a non-`sc_clock` clock with `add_clock_alias` |
| `14_enable_port_access/` | Driving `msg_in`/`msg_out` pins directly (`enable_port_access`) |
| `15_enable_port_access_portless/` | Manual handshake on a portless channel |
| `16_fifo_channel/` | `msg_fifo_channel` FIFO wrapper |
| `17_mixed_language/` | Wrappers for Verilog / SystemC co-simulation |

Most examples (02–11) use the simple **inline** testbench pattern: `stim` /
`resp` / `reset` SC_THREADs declared directly inside `SC_MODULE(testbench)`.
Examples **01** and **12** use the **structured** pattern: a separate
`top.h` declares `driver` and `monitor` modules instantiated as `drv_` /
`mon_` inside `testbench`.

Build and run any single example:

```bash
cd examples/03_nonblocking
make clean && make run
```

Run all 17 examples in parallel:

```bash
cd examples
python3 run_all.py
```

## Tests

See the `tests/` directory for safety check validation:

| Test | Description |
|------|-------------|
| `01_missing_reset/` | Validates SC_HLS-101 (missing reset detection) |
| `02_multiple_writers/` | Validates SC_HLS-102 (multiple writers detection) |
| `03_multi_process_access/` | Validates SC_HLS-102 (wrong process access) |
| `04_incorrect_annotation/` | Validates SC_HLS-ANNOTATE (unused annotation warning) |
| `05_dv_overrun/` | Validates SC_HLS-401 (`msg_dv_in` overrun) and the `dv_overrun_policy` settings |
| `06_generated_clock/` | Negative test: manually generated clock without an `sc_clock` |
| `07_enable_port_access/` | Negative test: `enable_port_access` misuse |

Run all tests:
```bash
cd tests
python3 run_all.py
```

## Release Notes

See [RELEASE_NOTES.md](RELEASE_NOTES.md).

## Contributing

This library is being developed as part of the Accellera SystemC Synthesis Working
Group standardization effort. See [CONTRIBUTING.md](CONTRIBUTING.md).

## Support

For questions and issues, please open an issue on the public repository
https://github.com/accellera-official/systemc-hls (see [CONTRIBUTING.md](CONTRIBUTING.md)).
