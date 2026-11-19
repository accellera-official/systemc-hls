---
style: chapter
---

# 7. Communication: signals, ports, channels and memories

This section specifies how processes can communicate, either within a module or between modules.

Within a module, processes *<u>shall</u>* only communicate with each other through
- data members of type `sc_signal`,
- data members of a struct or class type that contains data members of type `sc_signal`, provided that the processes communicate only by reading and writing those signals, directly or through member functions of the struct or class, or
- *array variables* mapped to *memories* for synthesis.

Between modules, communication *<u>shall</u>* only be through:
- `sc_in` and `sc_out` ports which are to be bound to ports in the parent module or to `sc_signal`s, or
- *array variables* mapped to *memories* for synthesis.

Non-constant variables of other types *<u>shall</u>* be read and written by a single process.

Other cases of access to non-constant module data members from outside the module are ***Not Supported***.

## 7.1 Basic signals

The `sc_signal` primitive has ***Restricted Support***:
- with the `WRITER_POLICY` defaulted or explicitly set to `SC_ONE_WRITER` is ***Supported***
- with the `SC_MANY_WRITERS` policy is ***Not Supported***.

Furthermore, only the following member functions are ***Supported***.

- `void write( const T& )`
- `const T& read( ) const`
- `operator const T& () const`
- `sc_signal()`
- `sc_signal( const char* )`
- `operator= ( const T& )`
- `operator= ( const sc_signal<T,WRITER_POLICY>& )`

The following resolved signal types are ***Not Supported***:
- `sc_signal_resolved`
- `sc_signal_rv`.

Use of `sc_dt::sc_logic` as the template parameter `T` for `sc_signal<T>` is ***Not Supported***.

Inheriting from a basic signal type is ***Not Supported***.

## 7.2 Basic ports

The pre-defined specialized port classes are ***Supported*** so blocks in a SystemC hierarchy can
communicate through convenient access to member functions of the pre-defined SystemC primitive
channels.

Use of `sc_in<T>` and `sc_out<T>` for `T` being any synthesizable type (see Section [8](08-types.md#8-types)) is ***Supported***.

Use of `sc_in_clk` is ***Supported***.

Furthermore, for `sc_in<T>`, only the following member functions are ***Supported***.

- `sc_in()`
- `sc_in(const char*)`
- `const T& read( ) const`
- `operator const T& () const`
- `void bind( const sc_signal_in_if<T>& )`
- `void operator() ( const sc_signal_in_if<T> & )`
- `void bind( sc_port< sc_signal_in_if<T>, 1> & )`
- `void operator() ( sc_port< sc_signal_in_if<T>, 1> & )`
- `void bind( sc_port< sc_signal_inout_if<T>, 1> & )`
- `void operator() ( sc_port< sc_signal_inout_if<T>, 1> & )`

For `sc_in<bool>`, in addition to the above, the following further member functions are
***Supported***.

- `sc_event_finder& pos() const`
- `sc_event_finder& neg() const`

Furthermore, for `sc_out<T>`, only the following member functions are ***Supported***.

- `sc_out()`
- `sc_out(const char*)`
- `const T& read( ) const`
- `operator const T& () const`
- `void write( const T& )`
- `operator= ( const T& )`
- `operator= ( const sc_signal_in_if<T>& )`
- `operator= ( const sc_port< sc_signal_in_if<T>, 1> & )`
- `operator= ( const sc_port< sc_signal_inout_if<T>, 1> & )`
- `operator= ( const sc_out<T> & )`
- `void bind( sc_signal_inout_if<T>& )`
- `void operator() ( sc_signal_inout_if<T>& )`
- `void bind( sc_port_b< sc_signal_inout_if<T> >& )`
- `void operator() ( sc_port_b< sc_signal_inout_if<T> >& )`

For `sc_out<bool>`, in addition to the above, the following further member functions are
***Supported***.

- `sc_event_finder& pos() const`
- `sc_event_finder& neg() const`

Use of `sc_dt::sc_logic` as the template parameter `T` for `sc_in<T>` and `sc_out<T>` is
***Not Supported***.

Use of bidirectional port `sc_inout<T>` is ***Not Supported***.

The following resolved port types are ***Not Supported***:
- `sc_in_resolved`
- `sc_inout_resolved`
- `sc_out_resolved`
- `sc_in_rv`
- `sc_inout_rv`
- `sc_out_rv`

Inheriting from a basic port type is ***Not Supported***.

Calling the binding function of a basic port — when that port is a data member of a module — from the constructor of the module’s parent is ***Supported***.

## 7.3 Interface, export and port

The SystemC core language classes — `sc_interface`, `sc_export`, and `sc_port` — are designed to support interface method call channel semantics. This allows ports to invoke member functions of the channel they are bound to.

However, such interface method calls can cross module boundaries without well-defined rules for translation to pin-level interfaces, which complicates synthesis.

Within a design, for any `sc_module`, the following rules apply:
- Direct or indirect inheritance from `sc_interface` is ***Ignored***.
- Direct or indirect inheritance from `sc_port` or `sc_export` is ***Not Supported***.
- Direct or indirect instantiation of data members of type `sc_port` or `sc_export` — except for the types explicitly allowed in Section [7.2](#72-basic-ports) — is ***Not Supported***.

## 7.4 Modular ports

The purpose of a modular port is to encapsulate related ports and signals into a single structure, and to optionally provide a convenient API that encapsulates a particular I/O protocol, and hierarchical bind functions.

Modular ports are ***Supported with Restrictions***.

### 7.4.1 Syntactic requirements

The syntactic requirements for a modular port are:
- A modular port *<u>shall</u>* be a struct or a class.
- A modular port *<u>may</u>* be derived from `sc_module`.

*Example: output port, without base class:*
```cpp
template<typename T>
struct msg_out   // no base class
{
    sc_in<bool>     rdy;
    sc_out<bool>    vld;
    sc_out<T>       dat;

    // Default constructor, required for C arrays of modular ports.
    // Delegating constructor, to avoid a stale name pointer.
    msg_out() : msg_out(std::string(sc_gen_unique_name("msg_out")).c_str()) {}

    msg_out(const char* n)
        : rdy((std::string(n)+"_rdy").c_str())
        , vld((std::string(n)+"_vld").c_str())
        , dat((std::string(n)+"_dat").c_str())
    {}

    // Example of I/O member function:
    void reset() {
        vld.write(0);
    }

    // Example of I/O member function:
    void put(const T& value) {
        // I/O code with protocol details goes here
        ...
    }

    // Hierarchical bind function:
    template<typename CHAN>
    void operator()(CHAN& c) { bind(c); }

    // Hierarchical bind function:
    template <typename CHAN>
    void bind(CHAN& c) {
        rdy(c.rdy);
        vld(c.vld);
        dat(c.dat);
    }
};
```

*Example: input port, deriving from `sc_module`*
```cpp
template<typename T>
SC_MODULE(msg_in)   // Deriving from SystemC module
{
    sc_out<bool>    SC_NAMED(rdy);
    sc_in<bool>     SC_NAMED(vld);
    sc_in<T>        SC_NAMED(dat);

    SC_CTOR(msg_in) {};

    // Default constructor, required for C arrays of modular ports:
    msg_in() : msg_in(sc_gen_unique_name("msg_in")) {}

    // Example of I/O member function:
    void reset() {
        // protocol reset code here
        rdy.write(0);
    }

    // Example of I/O member function:
    T get() {
        // I/O code with protocol details goes here
        ...
    }

    // Hierarchical bind function:
    template<typename CHAN>
    void operator()(CHAN& c) { bind(c); }

    // Hierarchical bind function:
    template <typename CHAN>
    void bind(CHAN& c) {
        rdy(c.rdy);
        vld(c.vld);
        dat(c.dat);
    }
};
```

Modular ports are instantiated in parent modules, using the same syntax as for a basic port.

*Example: a modular port instantiation:*
```cpp
SC_MODULE(ParentModule) {
    sc_in<bool>             SC_NAMED(clk);
    sc_in<bool>             SC_NAMED(rst);
    msg_in<sc_uint<32>>     SC_NAMED(bus_in);
    msg_out<sc_uint<32>>    SC_NAMED(bus_out);
    ...

    SC_CTOR(ParentModule) {
        ...
    }
    ...
};
```

### 7.4.2 Synthesis rules

Basic ports in the modular ports can be treated in two ways, depending on where they are connected.
If they are connected to signals outside the parent module, a corresponding port will appear in the interface of the generated RTL module.
Otherwise, if connected to ports of the parent module, e.g. reset or clock ports, then there will be no additional port created.

The following rules describe the expected interface in the generated RTL for modular ports:

- If a basic port in a modular port is not connected to a basic port in the parent module, the *<u>synthesis tool</u>* *<u>shall</u>* inline it in the parent module, and
- for such inlined port, the *<u>synthesis tool</u>* *<u>shall</u>* name it in this form: `"<modular port name>_<basic port name>"`, where:
  - `<modular port name>` is the C++ identifier of the data member that declares the modular port in the parent module, and
  - `<basic port name>` is the C++ identifier of the data member that declares the basic port in the modular port.

The *<u>synthesis tool</u>* *<u>shall</u>* derive these names from the C++ declarations, independently of the names passed to the constructors of the modular port or of its basic ports.

The example below demonstrates the above port instantiation and naming rules:

```systemverilog
module ParentModule (
    input wire clk,
    input wire rst,
    output wire bus_in_rdy,
    input wire bus_in_vld,
    input wire [31:0] bus_in_dat,
    input wire bus_out_rdy,
    output wire bus_out_vld,
    output wire [31:0] bus_out_dat
    ...
);
```


## 7.5 Memories

To implement a memory in a design, the standard approach is to declare an array and apply a synthesis directive to map that array to a memory resource.

Modeling a memory using an array of type `std::array` or a C-style array is ***Supported with Restrictions***.

Such an array *<u>shall not</u>* be allocated dynamically (e.g., using `new`) within the *design*, and the synthesis tool *<u>shall</u>* infer the number and type of read and write ports by analyzing the bodies of the processes that access the array.

The mechanism for designating an array to be implemented as a memory is **tool-specific** and provided through synthesis directives.

### 7.5.1 Array size and value type

Arrays used to model memories are ***Supported*** for any number of dimensions. The size of each dimension *<u>shall</u>* be a constant expression greater than zero.

For the array element (value) type, *Basic array value types* are defined as any of the following:
- Fundamental integer and Boolean types:
  `bool`, `char`, `short`, `unsigned short`, `int`, `unsigned int`, `long`, `unsigned long`, `long long`, `unsigned long long`
- SystemC bit-accurate and fixed-point types (with valid template parameters):
  `sc_bv<N>`, `sc_uint<W>`, `sc_biguint<W>`, `sc_int<W>`, `sc_bigint<W>`, `sc_fixed<W, I>`, `sc_ufixed<W, I>`

Memory arrays whose element type is a *Basic array value type* are ***Supported***.

For the purposes of this standard, a **Basic-layout class** is a class/struct whose non-static data members are limited to:
- Scalars of *Basic array value type*,
- Arrays of *Basic array value type* whose extents are compile-time constants,
- Other **Basic-layout classes** (recursively defined as above).

Memory arrays whose element type is a **Basic-layout class** are ***Supported***.

All other element (value) types are ***Not Supported***.

### 7.5.2 Memories local to a module

Mapping arrays declared as module data members, or as automatic objects created once per process activation (e.g., before the main `while` loop of an `SC_THREAD`), to a memory is ***Supported***.

Mapping arrays whose lifetime is limited to a single iteration (e.g., declared inside a `while(1)` loop) to a memory is ***Not Supported***.

If multiple processes share a read or write port, the synthesis tool *<u>shall</u>* insert the required memory port multiplexing automatically.

Because the order in which process instances are run within the evaluation phase is implementation-defined, access to a memory shared by different processes, also across modules (see Section [7.5.3](#753-memory-shared-between-modules)), *<u>should</u>* be explicitly synchronized to avoid non-deterministic simulation behavior (see SystemC LRM 4.3.2.3).

***
*Example — Memory used within a module*
```cpp
struct MyClass {
    std::array<sc_uint<32>, 64> array_a;
    // ...
};

SC_MODULE(MyBlock) {
    // ...
    std::array<sc_uint<32>, 128> array_b; // map to memory: SUPPORTED
    // ...

    SC_CTOR(MyBlock) {
        SC_THREAD(proc);
        // ...
    }

    void proc() {
        sc_uint<32> array_c[128]; // map to memory: SUPPORTED (allocated once per thread start)
        MyClass     myobj_1;      // map to memory: SUPPORTED for array_a (constructed once here)
        // ...
        wait();
        while (1) {
            // map to memory: NOT SUPPORTED for array_a (constructed each iteration)
            MyClass myobj_2;
            // ...
        }
    }
};
```
***


### 7.5.3 Memory shared between modules

Sharing a memory between modules is ***Supported with Restrictions*** and is allowed only when using `std::array`.

The array *<u>shall</u>* be specified as follows:
- Modules (or classes instantiated within modules) *<u>shall</u>* maintain references to the array that models the memory.
- The array instance *<u>shall</u>* be passed to each submodule through its constructor, and the constructor *<u>shall</u>* either bind the parameter to a reference data member or forward it to a subobject that maintains the reference.

If multiple modules share a read or write port, the synthesis tool *<u>shall</u>* automatically insert the required memory port multiplexing.

***
*Example: memory shared between modules*
```cpp
// Memory: 32-bit words, 128 entries
using mem_t = std::array<sc_dt::sc_uint<32>, 128>;

// -----------------------
// Reader (read-only user)
// -----------------------
SC_MODULE(LeafReader) {
    const mem_t& mem;   // read-only reference to shared memory

    LeafReader(const sc_module_name& n, const mem_t& mem_in)
        : sc_module(n)
        , mem(mem_in) // bind shared memory reference
    { /* ... */
    }

    void proc() {
        // ... read from mem ...
        // auto v = mem[0];
    }
};

// -----------------------
// Writer (write-capable user)
// -----------------------
SC_MODULE(LeafWriter) {
    mem_t& mem; // non-const reference for write access

    LeafWriter(const sc_module_name& n, mem_t& mem_in)
        : sc_module(n)
        , mem(mem_in) // bind shared memory reference
    { /* ... */ }

    void proc() {
        // ... write to mem ...
        // mem[0] = 0x12345678;
    }
};

// ----------------------------------------------------
// Wrapper that forwards the memory reference to a subobject
// (illustrates: "constructor passes memory to a subobject that maintains the reference")
// ----------------------------------------------------
struct Worker {
    mem_t& mem; // holds the reference
    explicit Worker(mem_t& m) : mem(m) {}

    void do_something() {
        // ... read/write mem ...
        // mem[1] = 0xDEADBEEF;
    }
};

SC_MODULE(LeafWrapper) {
    Worker worker1; // subobject that retains the shared memory reference
    Worker worker2; // subobject that retains the shared memory reference

    LeafWrapper(const sc_module_name& n, mem_t& mem_in)
        : sc_module(n)
        , worker1(mem_in) // forward reference to subobject, single memory port to be inferred
        , worker2(mem_in) // forward reference to subobject, single memory port to be inferred
    { /* ... */ }

    void proc() {
        worker1.do_something();
        worker2.do_something();
    }
};

// -----------------------
// Parent that owns memory
// -----------------------
SC_MODULE(ParentMod) {
    mem_t       mem;       // SINGLE shared memory instance (statically allocated in the design)
    LeafWriter  writer;    // write-capable user
    LeafReader  reader1;   // read-only user
    LeafReader  reader2;   // another read-only user (demonstrates shared read port)
    LeafWrapper wrapper;   // demonstrates forwarding to a subobject

    SC_CTOR(ParentMod)
        : mem{}                       // default-initialize memory array
        , writer("Writer", mem)       // pass memory by reference to submodules
        , reader1("Reader1", mem)     // read-only reference
        , reader2("Reader2", mem)     // another read-only reference (tools may share a read port)
        , wrapper("Wrapper", mem)     // constructor forwards to subobject that holds the reference
    {
        // Synthesis tools:
        //  - infer read/write ports from accesses in submodule processes,
        //  - insert automatic sharing mux if multiple modules share a read or write port.
    }
};
```

## 7.6 Arrays of signals and ports

Arrays of signals and ports are ***Supported with Restrictions***, either through the use of C arrays or through the use of the `sc_vector` class.

### 7.6.1 Syntactic requirements

The following constructs can be used in arrays:
- Arrays of `sc_signal` are ***Supported***.
- Arrays of basic ports are ***Supported***.
- Arrays of modular ports are ***Supported***.

**Note:** A C array of modular ports requires the modular port to have a default constructor (ISOC++ 11.6). An `sc_vector` of modular ports requires the modular port to be derived from `sc_object`, e.g. from `sc_module`, and to be constructible from a `const char*` name (SystemC LRM 8.5.3). A modular port without a base class can therefore only be used in a C array.

Features of `sc_vector` are outlined below:
- Specification of size through constructor argument is ***Supported***.
- Specification of size through `init()` function in the parent module constructor is ***Supported***.
- The `size()` member function is ***Supported***.
- `operator[]` and `at()` methods are ***Supported***.
- `sc_vector` of `sc_vector` is ***Supported***.

Examples of supported port arrays:
```cpp
SC_MODULE(MyModule) {
    sc_in<int>      in[10];
    sc_out<int>     out[10];
    sc_signal<int>  sig[10];
    sc_vector<sc_in<int>>           SC_NAMED(vec_in);
    sc_vector<sc_out<int>>          vec_out{"vec_out", 10};
    sc_vector<sc_signal<int>>       vec_sig{"vec_sig", 10};
    sc_vector<msg_in<sc_uint<32>>>  bus_in{"bus_in",10};
    msg_out<sc_uint<32>>            bus_out[10];

    SC_CTOR(MyModule) {
        vec_in.init(10);
    }
    ...
};
```

### 7.6.2 Synthesis rules

For an array of modular ports, either a C array or an `sc_vector`, the *<u>synthesis tool</u>* *<u>shall</u>* apply the rules of Section [7.4.2](#742-synthesis-rules) to each element, with `<modular port name>` replaced by `<array name>_<index>`, where:
- `<array name>` is the C++ identifier of the data member that declares the array, and
- `<index>` is the index of the element, in decimal.

For a multi-dimensional C array, or an `sc_vector` of `sc_vector`, one `_<index>` *<u>shall</u>* be appended per dimension, outermost dimension first.

The *<u>synthesis tool</u>* *<u>shall</u>* derive these names from the C++ declarations, independently of the names of the elements in simulation.

**Note:** In simulation, elements default-constructed with `sc_gen_unique_name` get names such as `msg_out_2_rdy`, which reflect neither the array name nor the element index.

The example below shows the RTL interface generated for the modular port arrays `bus_in` and `bus_out` of the example above:

```systemverilog
module MyModule (
    ...
    output wire bus_in_0_rdy,
    input wire bus_in_0_vld,
    input wire [31:0] bus_in_0_dat,
    ...
    input wire bus_out_9_rdy,
    output wire bus_out_9_vld,
    output wire [31:0] bus_out_9_dat
    ...
);
```

## 7.7 Unsupported pre-defined SystemC channels

The following pre-defined SystemC channels are ***Not Supported***:
- `sc_buffer`
- `sc_clock`
- `sc_mutex`
- `sc_semaphore`
- `sc_fifo`
- `sc_event_queue`

The core language class `sc_event` is also ***Not Supported***.


**Note:** While `sc_clock` channel is not supported, clock ports are supported as specified in Section [7.2](#72-basic-ports). An `sc_clock` can be used in the testbench to drive these clock ports (see [18.10](18-systemc-hls-library.md#1810-clock-management)).



