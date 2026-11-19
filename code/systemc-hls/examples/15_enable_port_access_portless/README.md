# 15_enable_port_access_portless - Manual Handshake on a Portless Channel

Demonstrates `enable_port_access_reader()` / `enable_port_access_writer()` on a
**portless `msg_channel`**: two processes in one module communicate through an
internal channel whose vld/rdy handshake is driven by hand on both sides.

## Why the reader/writer split

A `msg_channel` inherits *both* the reader and writer protocols, so it has two
`enable_port_access()` methods (one per side). To disambiguate, the channel
exposes:

- `enable_port_access_writer()` — the producer side drives `vld` / `dat`,
  reads `rdy`.
- `enable_port_access_reader()` — the consumer side drives `rdy`, reads
  `vld` / `dat`.

(A plain port — `msg_in` / `msg_out` — has only one side, so it just uses
`enable_port_access()`; see example 14.)

## Design

```
   in (msg_in, normal) --> proc1 --> chan (portless, manual) --> proc2 --> out (msg_out, normal)
                                     writer side ^   ^ reader side
```

One module, two clocked processes, one internal portless `msg_channel chan`:

- **proc1** owns the channel **writer** side. It `pop()`s from `in` with the
  normal API, then drives `chan.vld` / `chan.dat` and reads `chan.rdy` directly.
  Its `reset1()` method uses the normal `in.reset_pop()` for the input port, and
  drives the manual channel writer idle (`chan.vld = 0`) directly — the
  channel's `reset_push()` is forbidden under port access.
- **proc2** owns the channel **reader** side. It drives `chan.rdy` and reads
  `chan.vld` / `chan.dat` directly, then `push()`es to `out` with the normal
  API. Its `reset2()` method drives the manual channel reader idle
  (`chan.rdy = 0`) directly — the channel's `reset_pop()` is forbidden under
  port access — and uses the normal `out.reset_push()` for the output port.

Only the portless channel is manual; the module's `in` / `out` ports use the
ordinary `pop()` / `push()` API, so the example isolates the channel-side
reader/writer split. `enable_port_access_reader/writer()` may only be called
during elaboration (in the constructor). Each stage holds one value in a
register, and the two independent holding registers let the pipeline stream at
one beat per cycle (II=1).

## Files

- `dut.h` — the module with `chan.enable_port_access_writer()` /
  `chan.enable_port_access_reader()` and the two manual-handshake processes.
- `testbench.cpp` — `driver`, `monitor`, `testbench`, and `sc_main`.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The monitor receives `0..9` in order and the simulation prints
`Simulation PASSED`. A `waves.vcd` is produced (traces `clk` and the channels,
including the internal `dut_.chan`).

## Cycle-accuracy cross-check (optional)

Load `waves.vcd`, set the clock to `clk`, and inspect the internal channel
`tb_top.dut_.chan`: `chan.dat` carries `0,1,2,...,9` with `chan.vld` and
`chan.rdy` asserted on the streaming cycles, all transitions on clock edges —
i.e. the hand-driven reader/writer handshake is cycle-accurate, exactly as an
auto-scheduled channel would be.

## Related Examples

- `05_portless` — the same portless-channel topology using the normal
  `push()` / `pop()` API (channel manager drives the pins).
- `14_enable_port_access` — `enable_port_access()` on plain `msg_in` / `msg_out`
  ports.
