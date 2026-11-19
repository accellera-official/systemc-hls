---
style: chapter
---

# 18. The SystemC HLS Library

This section specifies the SystemC HLS library. The library currently includes
a communication library, a set of handshake-based ports and channels for
throughput-accurate, synthesizable communication between processes and modules.
Its future versions may include other SystemC and C++ components.

The purpose of the communication library is to provide an API and the supporting simulation
infrastructure for modeling *<u>designs</u>* with concurrent I/O operations, such
that the same concurrent transactions observed in the SystemC model are also
observed in the generated RTL. A *transaction* is an observable event at a module
interface, using variations of the valid, data, and ready protocols.

The library is a defined API and behavioral contract. A *<u>synthesis tool</u>*
*<u>may</u>* provide its own implementation; any implementation that conforms to
the class names, member signatures, and behavior specified in this chapter is a
conforming implementation. The proof-of-concept reference implementation released with
this standard is one such implementation; its internal structure is not normative.

## 18.1 Overview

All library types are made available through a single include, and reside in the
`sc_hls::msg_lib` namespace. The library-wide utility `add_clock_alias` is
additionally available in the enclosing `sc_hls` namespace; it is available in the
simulation view only.

```cpp
#include <systemc-hls>
using namespace sc_hls;
using namespace sc_hls::msg_lib;
```

The library provides two message passing protocols. Each protocol defines a
*producer* port, a *consumer* port, and a *portless* channel usable directly
between two processes of the same module.

| Protocol | Producer port | Consumer port | Channel | Signals |
|----------|---------------|---------------|---------|---------|
| Message passing with flow control | `msg_out` | `msg_in` | `msg_channel` | `vld`, `rdy`, `dat` |
| Message passing without back-pressure | `msg_dv_out` | `msg_dv_in` | `msg_dv_channel` | `vld`, `dat` |

The protocols differ in when a transfer occurs:

- **Message passing with flow control** (dat/vld/rdy): data is transferred on a clock edge when `vld` = 1 and `rdy` = 1. The producer asserts `vld` when it has data; the consumer asserts `rdy` when it can accept. Either side *<u>may</u>* stall the other; flow control enables the design to use back-pressure.
- **Message passing without back-pressure** (dat/vld): data is transferred on a clock edge when `vld` = 1. There is no `rdy`; the consumer *<u>shall</u>* accept the beat in that cycle and cannot stall the producer.

The library also provides a rendezvous synchronization between two processes,
with no data transfer: `sync_out` drives `vld` and samples `rdy`, `sync_in`
drives `rdy` and samples `vld`, and `sync_channel` is the portless form. Neither
side has precedence; a rendezvous completes on a clock edge on which both `vld`
and `rdy` are asserted (see [18.5](#185-rendezvous-synchronization)).

The library models concurrent I/O within a process. The transactions of
distinct message passing endpoints *<u>may</u>* overlap in time — several transfers
*<u>may</u>* be in progress in the same cycle. In simulation, the partial order
in which the endpoint calls of a single process appear in the source code
*<u>shall</u>* be preserved: a `pop` or `push` that follows another in program
order *<u>shall</u>* not start before it.

A *<u>synthesis tool</u>* *<u>may</u>* overlap or reorder the transactions of a
process, for example by pipelining a loop so that the transactions of successive
iterations overlap, provided that for each endpoint the sequence of transferred
values is the same as in program order and the data dependencies of the process
are preserved. Such a transformation can change the timing of the transactions
relative to the simulation view; latency and capacity back-annotation
([18.8.3](#1883-latency-and-capacity-back-annotation)) can be used to model it in
simulation.

A rendezvous *<u>shall</u>* not overlap with any other transaction of the
same process: a `sync` (or `sync_read` / `sync_write`) *<u>shall</u>* act as a
barrier, so that every transaction preceding it in program order completes before
it, and no transaction following it in program order begins until it has
completed. This rule applies in both simulation and synthesis: a
*<u>synthesis tool</u>* *<u>shall</u>* not move a transaction across a
rendezvous.

The message passing APIs are defined by two SystemC interfaces:

```cpp
template <typename Message> class msg_in_if : public sc_interface {
public:
    virtual void    reset_pop() = 0;
    virtual Message pop() = 0;
    virtual bool    pop_nb(Message& m) = 0;
    virtual bool    peek_nb(Message& m) = 0;
};

template <typename Message> class msg_out_if : public sc_interface {
public:
    virtual void reset_push() = 0;
    virtual void push(const Message& m) = 0;
    virtual bool push_nb(const Message& m) = 0;
};
```

For message passing, a producer port *<u>shall</u>* implement the writer interface (`msg_out_if`), a consumer port *<u>shall</u>* implement the reader interface (`msg_in_if`), and a channel *<u>shall</u>* implement both.

## 18.2 Simulation and Synthesis Views

The library defines two views of each port and channel:

- a **simulation view**, used to simulate the SystemC model of the *<u>design</u>* with the
  concurrent transactions, and
- a **synthesis view**, used as input to an HLS *<u>synthesis tool</u>* for
  generating RTL.

Each port and channel takes a trailing view-selection template parameter for the view tag:
```cpp
template <typename Message, typename ViewTag = default_view_tag> class msg_in;
```

The `sync_in`, `sync_out`, and `sync_channel` classes carry no `Message` and are
parameterized on `ViewTag` only.

The view is selected through `default_view_tag`, which is resolved at compile
time according to whether the `__SYNTHESIS__` macro is defined:

- When `__SYNTHESIS__` is not defined (simulation), `default_view_tag`
  *<u>shall</u>* resolve to the throughput-accurate simulation view `sim_view_tag`.
- When `__SYNTHESIS__` is defined (synthesis), `default_view_tag` *<u>shall</u>*
  resolve to the tag named by the macro `SC_HLS_DEFAULT_SYNTH_VIEW_TAG`. A
  *<u>synthesis tool</u>* selects its own synthesis view by defining this macro
  to its view tag; if `__SYNTHESIS__` is defined and `SC_HLS_DEFAULT_SYNTH_VIEW_TAG`
  is not, the program is ill-formed.

A *<u>synthesis tool</u>* *<u>shall</u>* define both `__SYNTHESIS__` and `SC_SYNTHESIS`
(see Section [4.2](04-translation-units.md#42-pre-processing-directives)).

A *<u>design</u>* that names no explicit `ViewTag` therefore compiles to the
simulation view for simulation and to the *<u>synthesis tool</u>*'s view for
synthesis, from a single source.

A third view, `tlm_view_tag`, selects a fast functional simulation view backed by
TLM FIFOs; it is described in [18.9](#189-tlm-fast-simulation-view).

## 18.3 Message Passing with Flow Control

Message passing with flow control provides reliable message passing; flow
control enables the design to use back-pressure. A message passed with this
protocol *<u>shall</u>* not be dropped: a data value is
transferred on signal `dat` on a clock edge only when both `vld` and `rdy` are
asserted, and either endpoint *<u>may</u>* stall the other until it is ready.

```cpp
template <typename Message, typename ViewTag = default_view_tag> class msg_out;    // producer
template <typename Message, typename ViewTag = default_view_tag> class msg_in;     // consumer
template <typename Message, typename ViewTag = default_view_tag> class msg_channel; // portless
```

A `msg_in` consumer port implements `msg_in_if` (`reset_pop`, `pop`, `pop_nb`,
`peek_nb`); a `msg_out` producer port implements `msg_out_if` (`reset_push`, `push`, `push_nb`); a
`msg_channel` implements both.

The ports *<u>shall</u>* implement the following behavior:
- `reset_pop` / `reset_push` *<u>shall</u>* be called in the reset state before any transfer.
- `pop()` *<u>shall</u>* not return until a value has been received.
- `push()` *<u>shall</u>* not return until the value has been accepted.
- The non-blocking `pop_nb`, `peek_nb`, and `push_nb` *<u>shall</u>* return within the current cycle and report success through their return value.

Each port and channel *<u>shall</u>* provide a member type alias `message_type` equal to its `Message` parameter.

The `msg_in` and `msg_out` ports are modular ports as specified in
[7.4](07-channels.md#74-modular-ports). In the generated RTL, their `rdy`, `vld`,
and `dat` ports *<u>shall</u>* be named as specified in
[7.4.2](07-channels.md#742-synthesis-rules), for example `in1_rdy`, `in1_vld`,
`in1_dat` for a port `in1`.

A `msg_channel` *<u>shall</u>* additionally support portless access: it
*<u>shall</u>* expose the `msg_in_if` and `msg_out_if` interface functions
directly on the channel object, with the same required behavior specified above,
so that two processes of the same module can communicate through it without ports.

In addition to implementing the `msg_in_if` / `msg_out_if` interfaces, a port or
channel *<u>shall</u>* provide the binding functions `bind()` and `operator()`,
used to bind it to the ports or channels it connects to; `operator()`
*<u>shall</u>* have the same effect as `bind()`. The example below shows the
reader side of a `msg_in` consumer port: it implements `msg_in_if` and provides
the binding functions over its `vld`, `rdy`, and `dat` signals. The class layout
is implementation-dependent; only the interface methods, the `message_type` alias, the three signal-level
ports and the presence of the binding functions are normative.

***
*Example: a consumer port implementing `msg_in_if` with binding functions*
```cpp
template <typename Message>   // ViewTag parameter omitted for brevity
class msg_in
    : public msg_in_if<Message>, public sc_module // class layout is implementation-dependent
{
public:
    using message_type = Message;

    sc_out<bool>    SC_NAMED(rdy);
    sc_in<bool>     SC_NAMED(vld);
    sc_in<Message>  SC_NAMED(dat);

    // msg_in_if implementation:
    void    reset_pop() override;
    Message pop() override;
    bool    pop_nb(Message& m) override;
    bool    peek_nb(Message& m) override;

    // Binding functions:
    template <typename C> void bind(C& rhs)       { rdy(rhs.rdy); vld(rhs.vld); dat(rhs.dat); }
    template <typename C> void operator()(C& rhs) { bind(rhs); }
};
```

***
*Example: a message passing passthrough module using ports*
```cpp
SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<32>>  SC_NAMED(in1);
    msg_out<sc_uint<32>> SC_NAMED(out1);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        out1.reset_push();
        wait();
    }

    void main() {
        reset();
        while (1) {
            auto v = in1.pop();
            out1.push(v);
        }
    }
};
```
***
*Example: two processes of one module communicating through an internal channel*
```cpp
SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<uint32_t>      SC_NAMED(in1);
    msg_out<uint32_t>     SC_NAMED(out1);
    msg_channel<uint32_t> SC_NAMED(chan1);   // internal, portless

    SC_CTOR(dut) {
        SC_THREAD(main1);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
        SC_THREAD(main2);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void main1() {
        in1.reset_pop();
        chan1.reset_push();
        wait();
        while (1) { chan1.push(in1.pop()); }
    }

    void main2() {
        chan1.reset_pop();
        out1.reset_push();
        wait();
        while (1) { out1.push(chan1.pop()); }
    }
};
```
***

## 18.4 Message Passing without Back-Pressure

The dat/vld message passing protocol provides valid-only signaling with no `rdy`
back-pressure. The producer drives data whenever it has data available and
*<u>shall</u>* not be stalled by the consumer. The consumer *<u>shall</u>* accept a beat in the cycle in
which `vld` is asserted.

```cpp
template <typename Message, typename ViewTag = default_view_tag> class msg_dv_out;     // producer
template <typename Message, typename ViewTag = default_view_tag> class msg_dv_in;      // consumer
template <typename Message, typename ViewTag = default_view_tag> class msg_dv_channel; // portless
```

A `msg_dv_in` consumer port implements `msg_in_if` (`reset_pop`, `pop`, `pop_nb`,
`peek_nb`); a `msg_dv_out` producer port implements `msg_out_if` (`reset_push`, `push`, `push_nb`); a
`msg_dv_channel` implements both. Each port and channel provides the
`message_type` alias. As for `msg_in` / `msg_out`, a dat/vld port or channel *<u>shall</u>* also provide
the binding functions (`bind()` and `operator()`) over its `vld` and `dat`
signals, and a `msg_dv_channel` *<u>shall</u>* support portless access by exposing
the interface functions directly on the channel object.

The `msg_dv_in` and `msg_dv_out` ports are modular ports as specified in
[7.4](07-channels.md#74-modular-ports). In the generated RTL, their `vld` and
`dat` ports *<u>shall</u>* be named as specified in
[7.4.2](07-channels.md#742-synthesis-rules), for example `in1_vld`, `in1_dat`
for a port `in1`.

The consumer *<u>shall</u>* be able to buffer one unconsumed beat.
If a new valid beat arrives while the buffer is still occupied, the buffered beat is overwritten and the previous value is lost; this is faithful to a valid-only wire.

The ports *<u>shall</u>* implement the following behavior:
- `reset_pop` / `reset_push` *<u>shall</u>* be called in the reset state before any transfer.
- `push()` *<u>shall</u>* not return until the producer's beat has been driven; the consumer cannot apply back-pressure.
- `pop()` *<u>shall</u>* not return until a beat is available in the consumer's buffer.
- The non-blocking `pop_nb`, `peek_nb`, and `push_nb` *<u>shall</u>* return within the current cycle and report success through their return value; `push_nb` reports `false` when the producer's previous beat has not yet been driven.

***
*Example: a dat/vld producer-consumer module*
```cpp
SC_MODULE(dut) {
    using T = unsigned;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_dv_in<T>  SC_NAMED(in1);
    msg_dv_out<T> SC_NAMED(out1);

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in1.reset_pop();
        out1.reset_push();
        wait();
    }

    void main() {
        reset();
        while (1) {
            auto x = in1.pop();
            out1.push(x);
        }
    }
};
```
***

## 18.5 Rendezvous Synchronization

The rendezvous protocol synchronizes two processes without transferring data. A
rendezvous completes on a clock edge in which both `vld` and `rdy` are asserted.

```cpp
template <typename ViewTag = default_view_tag> class sync_out;     // drives vld
template <typename ViewTag = default_view_tag> class sync_in;      // drives rdy
template <typename ViewTag = default_view_tag> class sync_channel; // portless
```

A `sync_in` port provides `reset_pop` and `sync`; a `sync_out` port
provides `reset_push` and `sync`. A portless `sync_channel` provides `reset_pop`
and `sync_read` for its reader process, and `reset_push` and `sync_write` for its
writer process. These classes carry no `Message`. A sync port or channel
*<u>shall</u>* also provide the binding functions (`bind()` and `operator()`)
over its `vld` and `rdy` signals, and a `sync_channel` *<u>shall</u>* support
portless access by exposing the rendezvous functions directly on the channel
object.

The `sync_in` and `sync_out` ports are modular ports as specified in
[7.4](07-channels.md#74-modular-ports). In the generated RTL, their `vld` and
`rdy` ports *<u>shall</u>* be named as specified in
[7.4.2](07-channels.md#742-synthesis-rules), for example `sync_port_vld`,
`sync_port_rdy` for a port `sync_port`.

The ports *<u>shall</u>* implement the following behavior:
- `reset_pop` / `reset_push` *<u>shall</u>* be called in the reset state before any rendezvous.
- `sync()` (and, on a channel, `sync_read()` / `sync_write()`) *<u>shall</u>* block until the rendezvous completes, that is, until a clock edge on which both `vld` and `rdy` are asserted, and *<u>shall</u>* then return on both sides in that cycle.
- No data is transferred.

***
*Example: a synchronization barrier before processing*
```cpp
SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    sync_in<>            SC_NAMED(sync_port);
    msg_out<uint32_t>    SC_NAMED(out1);

    uint32_t compute();  // user function, not part of the library

    SC_CTOR(dut) {
        SC_THREAD(main);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        sync_port.reset_pop();
        out1.reset_push();
        wait();
    }

    void main() {
        reset();
        while (1) {
            sync_port.sync();   // wait for the other side to rendezvous
            out1.push(compute());
        }
    }
};
```
***

## 18.6 FIFO

A conforming implementation *<u>shall</u>* provide buffered message passing FIFO channels of configurable depth in two forms.

These FIFO channels are one possible alternative to `sc_fifo`, which is
***Not Supported*** ([7.7](07-channels.md#77-unsupported-pre-defined-systemc-channels)).

Both forms *<u>shall</u>* buffer up to `Depth` elements and preserve order, and
their back-pressure *<u>shall</u>* prevent overflow and underflow. Both provide
the message passing API (through forwarding functions or ports) and
*<u>shall</u>* conform to the same required behavior specified in [18.3](#183-message-passing-with-flow-control).

`msg_fifo_module` presents the FIFO as a module with clock, reset, and `msg_in` / `msg_out`
enqueue/dequeue ports to be bound to channels:

```cpp
template <typename Message, unsigned int Depth, typename ViewTag = default_view_tag>
class msg_fifo_module : public sc_module {
public:
    sc_in_clk                 SC_NAMED(clk);
    sc_in<bool>               SC_NAMED(rst);
    msg_in<Message, ViewTag>  SC_NAMED(enq);   // enqueue side
    msg_out<Message, ViewTag> SC_NAMED(deq);   // dequeue side
};
```

***
*Example: buffering two modules with a depth-8 `msg_fifo_module`*
```cpp
SC_MODULE(dut) {
    using T = sc_uint<32>;
    static constexpr int N = 8;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_fifo_module<T, N> SC_NAMED(fifo1);
    msg_channel<T>        SC_NAMED(fifo_in);
    msg_channel<T>        SC_NAMED(fifo_out);
    // ... producer and consumer submodules ...

    SC_CTOR(dut) {
        fifo1.clk(clk);
        fifo1.rst(rst_bar);
        fifo1.enq(fifo_in);    // producer pushes into fifo_in
        fifo1.deq(fifo_out);   // consumer pops from fifo_out
        // ... bind producer.to_fifo(fifo_in), consumer.from_fifo(fifo_out) ...
    }
};
```
***

`msg_fifo_channel` wraps a `msg_fifo_module` and presents it as two channels
`in` and `out`, and forwards the message passing API:

```cpp
template <typename Message, unsigned int Depth, typename ViewTag = default_view_tag>
class msg_fifo_channel : public sc_module {
public:
    sc_in_clk                     SC_NAMED(clk);
    sc_in<bool>                   SC_NAMED(rst);
    msg_channel<Message, ViewTag> SC_NAMED(in);    // producer pushes here
    msg_channel<Message, ViewTag> SC_NAMED(out);   // consumer pops here

    // Producer side (forwards to in)
    void reset_push() { in.reset_push(); }
    void push(const Message &m) { in.push(m); }
    bool push_nb(const Message &m) { return in.push_nb(m); }

    // Consumer side (forwards to out)
    void reset_pop() { out.reset_pop(); }
    Message pop() { return out.pop(); }
    bool pop_nb(Message &m) { return out.pop_nb(m); }
    bool peek_nb(Message &m) { return out.peek_nb(m); }

private:
    msg_fifo_module<Message, Depth, ViewTag> fifo;
};
```

***
*Example: two processes communicating through a `msg_fifo_channel`*
```cpp
SC_MODULE(dut) {
    using T = sc_uint<32>;
    static constexpr int N = 8;

    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<T>  SC_NAMED(in1);
    msg_out<T> SC_NAMED(out1);

    msg_fifo_channel<T, N> SC_NAMED(fifo_chan);   // FIFO as in/out channels

    SC_CTOR(dut) {
        fifo_chan.clk(clk);
        fifo_chan.rst(rst_bar);
        SC_THREAD(producer);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
        SC_THREAD(consumer);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void producer() {
        in1.reset_pop();
        fifo_chan.in.reset_push();
        wait();
        while (1) { fifo_chan.in.push(in1.pop()); }
    }

    void consumer() {
        fifo_chan.out.reset_pop();
        out1.reset_push();
        wait();
        while (1) { out1.push(fifo_chan.out.pop()); }
    }
};
```
***

## 18.7 Direct Port Access

A conforming implementation *<u>shall</u>* support direct port access as follows.
A port or channel endpoint *<u>may</u>* opt out of the blocking/non-blocking API
and instead drive and sample its ports (`vld`, `rdy`, `dat`) directly. This is
enabled with:

```cpp
void enable_port_access();          // on msg_in / msg_out / msg_dv_in / msg_dv_out
void enable_port_access_reader();   // on a channel, for its reader side
void enable_port_access_writer();   // on a channel, for its writer side
```

`enable_port_access` *<u>shall</u>* be called during elaboration (in the module
constructor); calling it later *<u>shall</u>* be reported as an error. When
enabled on an endpoint:

- `push`, `pop`, `push_nb`, `pop_nb`, and `peek_nb` on that endpoint are
  ***Not Supported*** and *<u>shall</u>* be reported as an error, and
- `reset_pop` / `reset_push` on that endpoint are ***Not Supported*** and
  *<u>shall</u>* be reported as an error; the process instead drives the idle port
  values (for example `rdy.write(false)` / `vld.write(false)`) in its reset
  state.

The `vld`, `rdy`, and `dat` members are basic ports; the member functions
available on them are those specified in [7.2](07-channels.md#72-basic-ports).

***
*Example: driving the `msg_in` / `msg_out` ports directly*
```cpp
SC_MODULE(dut) {
    sc_in<bool> SC_NAMED(clk);
    sc_in<bool> SC_NAMED(rst_bar);

    msg_in<sc_uint<16>>  SC_NAMED(in);
    msg_out<sc_uint<16>> SC_NAMED(out);

    SC_CTOR(dut) {
        in.enable_port_access();
        out.enable_port_access();
        SC_THREAD(proc);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
    }

    void reset() {
        in.rdy.write(false);    // drive idle ports; reset_pop()/reset_push() forbidden
        out.vld.write(false);
        wait();
    }

    void proc() {
        reset();
        while (1) {
            // read in.vld / in.dat, drive in.rdy, out.vld, out.dat ...
            wait();
        }
    }
};
```
***

## 18.8 Simulation and Verification Features

This subsection specifies simulation and verification features. A conforming
implementation *<u>shall</u>* support the syntax presented below and provide the
described behavior. These features are available only in the simulation view; they are not available
under synthesis.

### 18.8.1 Tracing

An implementation *<u>shall</u>* provide `trace_hierarchy`, which traces every
library channel in an object hierarchy to a SystemC trace file:

```cpp
void trace_hierarchy(sc_object* obj, sc_trace_file* tf);
void trace_hierarchy(sc_object& obj, sc_trace_file* tf);
```

When invoked after the top-level module has been constructed, it *<u>shall</u>*
trace, for each library channel in the hierarchy, the `vld`, `rdy`, and `dat`
signals of a `msg_channel`; the `vld` and `dat` signals of a `msg_dv_channel`; and
the `vld` and `rdy` signals of a `sync_channel`.

```cpp
sc_trace_file* tf = sc_create_vcd_trace_file("waves");
trace_hierarchy(&top, tf);
```

### 18.8.2 Transaction Recording

An implementation *<u>shall</u>* provide `channel_logs`, which records every
message accepted by a `push` or a successful `push_nb`, and every message returned
by a `pop` or a successful `pop_nb`, on every message passing port and channel in a
hierarchy (`peek_nb` and rendezvous operations are not recorded):

```cpp
struct emitter_options {
    bool log_direction = true;
    bool unbuffered    = false;
};

class channel_logs {
public:
    int  enable(const std::string& fname = "channel_logs.txt",
                const emitter_options& opts = emitter_options());
    void log_hierarchy(sc_object& obj);
};
```

Function `enable` *<u>shall</u>* select the output file name and recording options, and
`log_hierarchy` *<u>shall</u>* attach every channel in the subtree to the log. A
`channel_logs` object *<u>shall</u>* outlive `sc_start()`.

The `emitter_options` argument of `enable` is optional; when it is omitted, the
default options are used.

The output is textual; each message is written using its `operator<<()`.

***
*Example: transaction recording with the default options*
```cpp
channel_logs logs;
logs.enable("run.txt");
logs.log_hierarchy(top);
```

***
*Example: transaction recording with explicit options*
```cpp
channel_logs    logs;
emitter_options opts;
opts.unbuffered = true;     // flush the log after each write
logs.enable("run.txt", opts);
logs.log_hierarchy(top);
```
***

### 18.8.3 Latency and Capacity Back-Annotation

A writer endpoint (a `msg_out` or `msg_dv_out` port, or a channel written
directly by a process) *<u>shall</u>* support back-annotation of its latency
(delay in clock cycles) and capacity (items in flight), used to improve
pre-synthesis timing accuracy. A writer endpoint *<u>shall</u>* provide:

```cpp
void set_latency_capacity(int latency, int capacity);   // on a writer endpoint
```

which *<u>shall</u>* configure the writer endpoint to model the given latency and
capacity for subsequent transfers.

An implementation *<u>shall</u>* also support loading annotations for a hierarchy
from a file, and writing a template of the current annotations to a file:

```cpp
namespace annotate {
    bool load_annotations(const std::string& filename);       // text
    bool load_annotations_json(const std::string& filename);  // JSON
    void write_annotations(const std::string& filename);      // emit a template
}
```

```cpp
top.dut_.block0_inst.dout.set_latency_capacity(3, 3);         // programmatic
sc_hls::msg_lib::annotate::load_annotations("latencies.txt"); // from file
```

### 18.8.4 Random Stall Injection

An implementation *<u>shall</u>* provide `rand_stall::set`, which injects
randomized back-pressure stalls into every message passing channel with flow control in a hierarchy, to
stress-test synchronization:

```cpp
struct rand_stall {
    static void set(sc_object& obj, bool verbose = false);
};
```

Random stall injection is ***Supported*** for message passing with flow control
only; it is ***Not Supported*** for message passing without back-pressure, which
has no back-pressure path. The stall sequence
*<u>shall</u>* be seeded from the `SC_HLS_RAND_SEED` environment variable, so a
given seed reproduces the same sequence.

```cpp
sc_hls::msg_lib::rand_stall::set(top, /*verbose=*/true);
```

## 18.9 TLM Fast-Simulation View

An implementation *<u>shall</u>* support the `tlm_view_tag` view, which selects a
fast functional simulation view in which each channel is backed by a
`tlm::tlm_fifo` rather than by `vld`/`rdy`/`dat` signals. It trades
throughput accuracy for simulation speed and is intended for fast functional
verification and early system-level exploration. This view is ***Not Supported***
under synthesis: `tlm_view_tag` is available only when `__SYNTHESIS__` is not
defined.

A channel in this view *<u>shall</u>* expose the same `msg_in_if` / `msg_out_if`
reader and writer API (`pop`/`pop_nb`/`peek_nb` and `push`/`push_nb`, with
`reset_pop` / `reset_push`) as the simulation view `sim_view_tag`, so process code is
unchanged between the two views. The channel constructor *<u>shall</u>* take the
FIFO depth as an argument, which defaults to 16:

```cpp
template <typename Message>
class msg_channel<Message, tlm_view_tag> {
public:
    msg_channel(const sc_module_name& nm, unsigned int depth = 16);
};
```

Because the view is a trailing template parameter, a *<u>design</u>* templated on
`ViewTag` can be instantiated in either view from a single source.

***
*Example: instantiating a design and its channels in the TLM view*
```cpp
dut<tlm_view_tag> SC_NAMED(dut_);

msg_channel<sc_uint<16>, tlm_view_tag> in1{"in1", 20};   // FIFO depth 20
msg_channel<sc_uint<32>, tlm_view_tag> out1{"out1", 20};
```
***

## 18.10 Clock Management

A conforming implementation *<u>shall</u>* automatically discover the `sc_clock`
objects used by a design.

If a design's clock is not an `sc_clock` (for example a hand-toggled
`sc_signal<bool>`), a companion `sc_clock` of matching period and phase
*<u>may</u>* be aliased to the signal so the library can schedule against it:

```cpp
void add_clock_alias(sc_clock& real_clk,
                     const sc_core::sc_signal_in_if<bool>& alias_sig);
```

```cpp
sc_clock clk_alias("clk_alias", 10, SC_NS);
add_clock_alias(clk_alias, clk);   // clk is a toggled sc_signal<bool>
```

## 18.11 Diagnostics

A conforming implementation *<u>shall</u>* report at least the following conditions:

| Report | Severity | Condition |
|--------|----------|-----------|
| `SC_HLS-101` | Fatal | An endpoint operation (`push`, `pop`, `push_nb`, `pop_nb`, `peek_nb`) was invoked without its reset call (`reset_pop` / `reset_push`) having been invoked in the reset state. |
| `SC_HLS-102` | Error | Two writer endpoints drive the same signal, or an endpoint operation is invoked from a process other than the one that invoked its reset call. |
| `SC_HLS-401` | Implementation-defined; not reported by default | A `msg_dv` consumer receives a valid beat while the previous beat has not been consumed. |
| `SC_HLS` | Error | An operation forbidden in direct port-access mode (see [18.7](#187-direct-port-access)); `enable_port_access` called after elaboration; no `sc_clock` found, or the clock of a process cannot be resolved; a back-annotation on an endpoint whose reset call was never invoked. |
| `SC_HLS-ANNOTATE` | Warning | An annotation file cannot be opened, is malformed, or names a channel that is not in the hierarchy. |
