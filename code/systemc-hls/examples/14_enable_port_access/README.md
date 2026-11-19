# 14_enable_port_access - Driving msg Pins Directly

Demonstrates `enable_port_access()` on `msg_in` / `msg_out`: a process that
maintains the msg (dat/vld/rdy) handshake by toggling the `vld` / `rdy` /
`dat` pins **directly**, instead of calling `push()` / `pop()`.

## Why

Most designs use the high-level `push()` / `pop()` API and let the library's
channel manager drive the wire protocol. Some customers instead want direct
pin-level control — e.g. to model a specific RTL handshake by hand. This is the
sc_hls analogue of mini_matchlib's `disable_spawn`.

## What `enable_port_access()` does

Call `enable_port_access()` only during elaboration (in the module constructor);
calling it later reports an `SC_HLS` error.

- **`push()` / `pop()` / `push_nb()` / `pop_nb()` / `peek_nb()` are forbidden** —
  calling them reports an `SC_HLS` error. The process must drive the pins itself.
- **The channel manager does not drive the pins** — the endpoint is not
  registered, so its internal `pre()` / `post()` scheduling never runs.
- **The reset functions are forbidden** — calling `reset_pop()` / `reset_push()`
  on a port-access port reports an `SC_HLS` error. The user process owns the
  pins, including their idle values. Per the repo convention, the user's
  code drives the idle pins directly (`in.rdy = 0` / `out.vld = 0`)
  instead of calling the reset functions.


## Design

```
        +--------+ in_chan  +-------------------+ out_chan +---------+
 driver | push() |  msg     | dut (manual pins) |   msg    | pop()   | monitor
 (auto) |  0..9  | -------> | enable_port_access| -------> |  check  | (auto)
        +--------+          +-------------------+          +---------+
```

Only the **DUT** uses `enable_port_access()`. Its single `proc` is a registered
1-deep passthrough that maintains both handshakes by hand:

- input side (DUT is consumer): drives `in.rdy` high when it has room; a beat is
  accepted on a cycle where `in.vld && in.rdy`.
- output side (DUT is producer): drives `out.vld` while holding a value; a beat
  is consumed on a cycle where `out.vld && out.rdy`.

The `driver` and `monitor` at the far ends use the ordinary `push()` / `pop()`
API, so the example isolates the manual side. Because the DUT holds one value at
a time, throughput is one beat every two cycles (II=2) — the correct behavior
for a 1-deep registered passthrough.

## Files

- `dut.h` — DUT with `in.enable_port_access()` / `out.enable_port_access()` and
  the manual handshake process.
- `testbench.cpp` — `driver`, `monitor`, `testbench`, and `sc_main`.
- `Makefile` — build configuration (uses `../common/Makefile.inc`).

## Building and Running

```bash
make clean && make run
```

## Expected Output

The monitor receives `0..9` in order and the simulation prints
`Simulation PASSED`. A `waves.vcd` is produced (traces `clk` and the channels).

## Cycle-accuracy cross-check (optional)

Load `waves.vcd`, set the clock to `clk`, and confirm:
- every `in_chan.rdy` / `out_chan.vld` transition lands on a clock edge
  (registered outputs), and
- `out_chan.dat` presents `0,1,2,...,9` with `out_chan.vld` asserted exactly on
  the cycle each value is valid — i.e. the hand-driven handshake is cycle-
  accurate, not off by a cycle.

## Related Examples

- `01_xput_accurate` — the same msg protocol via the normal `push()` / `pop()`
  API (channel manager drives the pins).
- `15_enable_port_access_portless` — `enable_port_access` on a portless
  `msg_channel`, using the reader/writer-qualified calls.
