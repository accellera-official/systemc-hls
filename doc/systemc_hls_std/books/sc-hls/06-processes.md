---
style: chapter
---

# 6. Processes

The use of the *SC_THREAD*, *SC_CTHREAD*, and *SC_METHOD* constructs to create processes is
***Supported with Restrictions***.

The use of *sc_spawn()* to create processes is ***Not Supported***.

None of the uses of *sc_process_handle* are synthesizable, hence such usage is ***Not Supported***.

Consequently, the use of *suspend()*, *resume()*, *enable()*, *disable()*, *kill()*, *reset()*, and
*throw_it()* is also ***Not Supported***.

## 6.1 SC_METHOD

In a SystemC *<u>design</u>*, the body of an *SC_METHOD* is executed whenever its sensitivity
condition is met. The logic inferred by a *<u>synthesis tool</u>* for an *SC_METHOD* depends on both
the sensitivity condition and the form of the *SC_METHOD* body.

As a *<u>synthesis refinement</u>*, all paths through an *SC_METHOD* *<u>shall</u>* either be
combinational or sequential. An *SC_METHOD* with a mix of combinational and sequential paths is
***Not Supported***.

For *SC_METHOD*, *reset_signal_is* and *async_reset_signal_is* are ***Not Supported***. Modeling of
reset behavior is done using the coding styles described below.

### 6.1.1 Combinational SC_METHOD

An *SC_METHOD* can be used to describe combinational logic, with the *SC_METHOD* sensitive to any
change in the signals in its sensitivity list.

As a *<u>synthesis refinement</u>*, the sensitivity list here *<u>shall</u>* be static and include
all signals that are read in the body of the method. This is done to avoid accidental latching,
which would lead to synthesis and simulation mismatches.

*Example of combinational SC_METHOD:*
```cpp
SC_MODULE(design) {
    sc_in <int> SC_NAMED(a);
    sc_in <int> SC_NAMED(b);
    sc_in <int> SC_NAMED(c);
    sc_out<int> SC_NAMED(f);

    SC_CTOR(design) {
        SC_METHOD(comb);
        sensitive << a << b << c;
    }

    void comb() {
        f.write(a.read() + b.read() + c.read());
    }
};
```

### 6.1.2 Sequential SC_METHOD

The body of a sequential *SC_METHOD* *<u>shall</u>* consist of two mutually exclusive branches
based on the clock and reset conditions.

#### 6.1.2.1 Sequential SC_METHOD with synchronous reset

A sequential *SC_METHOD* with synchronous reset *<u>shall</u>* be sensitive to only a positive or
negative edge of a clock signal.

*Example of sequential SC_METHOD with synchronous reset:*

```cpp
SC_MODULE(design) {
    sc_in <bool>    SC_NAMED(clk);
    sc_in <bool>    SC_NAMED(rst);
    ...

    SC_CTOR(design) {
        SC_METHOD(dff);
        sensitive << clk.pos();
    }

    void dff() {
        if (rst.read() == 0) {
            ... // reset signals
        } else {
            ... // assign signals
        }
    }
};
```

#### 6.1.2.2 Sequential SC_METHOD with asynchronous reset

A sequential *SC_METHOD* with asynchronous reset *<u>shall</u>* be sensitive to only a positive or
negative edge of a clock signal and a positive or negative edge of a reset signal.

*Example of sequential SC_METHOD with asynchronous reset:*

```cpp
SC_MODULE(design) {
    sc_in <bool>    SC_NAMED(clk);
    sc_in <bool>    SC_NAMED(rst);
    ...

    SC_CTOR(design) {
        SC_METHOD(dff);
        sensitive << clk.pos() << rst.neg();
    }

    void dff() {
        if (rst.read() == 0) {
            ... // reset signals
        } else {
            ... // assign signals
        }
    }
};
```

## 6.2 SC_THREAD and SC_CTHREAD

SystemC allows the usage of the *SC_THREAD* and *SC_CTHREAD* macros to create unspawned processes
which run from the start of simulation and are expected to run until the end of simulation (see
Section [6.2.2](06-processes.md#622-thread-process-body)).

*SC_CTHREAD* and *SC_THREAD* are ***Supported with Restrictions***. They are ***Supported***, as
specified in the SystemC LRM, when the following restrictions regarding the process sensitivity and
process body are met (in addition to the other restrictions mentioned throughout this standard,
e.g., regarding break/goto statements).

***
**Note:**

In earlier versions of SystemC, *SC_THREAD* and *SC_CTHREAD* differed in their reset behavior.

Currently, the two differ in how the clock is specified and in the forms of *wait()* allowed by the
SystemC LRM, but for synthesis purposes, they have the same expressiveness.

***

### 6.2.1 Clock and reset

The sensitivity of the process *<u>shall</u>* be specified in the constructor of the *SC_MODULE*
enclosing the process. A process *<u>shall</u>* be statically sensitive to exactly one clock edge
 and *<u>shall</u>* have at least one reset specification.

A process *<u>shall</u>* have at most one synchronous reset specification and at most one asynchronous
reset specification.

Different *SC\_(C)THREAD*s *<u>may</u>* be sensitive to different clocks and resets, i.e., an
*SC_MODULE* *<u>may</u>* contain multiple clocks.

The use of *reset_signal_is* and *async_reset_signal_is* to specify reset sensitivity is
***Supported***.

*Example of SC_THREAD:*

```cpp
SC_MODULE(design) {
    sc_in <bool>    SC_NAMED(clk);
    sc_in <bool>    SC_NAMED(rst);
    ...

    SC_CTOR(design) {
        SC_THREAD(thread_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false); // active low asynchronous reset
    }

    void thread_process() {
        ...
    }
};
```

*Example of SC_CTHREAD:*

```cpp
SC_MODULE(design) {
    sc_in <bool>    SC_NAMED(clk);
    sc_in <bool>    SC_NAMED(rst);
    ...

    SC_CTOR(design) {
        SC_CTHREAD(thread_process, clk.pos());
        async_reset_signal_is(rst, false); // active low asynchronous reset
    }

    void thread_process() {
        ...
    }
};
```

### 6.2.2 Thread process body

In a SystemC *<u>application</u>*, it is a common coding idiom to include an infinite loop
containing a call to the *wait()* function within a thread process in order to prevent the process
from terminating prematurely, at the same time allowing co-operative scheduling during simulation
by suspending the process. The thread process body of a *<u>design</u>* *<u>shall</u>* also follow this
structure. Among the available constructs to suspend a process, a *<u>design</u>* *<u>shall</u>* only use a
call to *wait()*, where the wait condition is the clock edge to which the thread process is
sensitive, as specified in the module constructor. That is, the only ***Supported*** waits are the
following.

- *wait()*

- *wait(int)*: where the integer argument is statically determinable.

Other forms of suspending a thread process are ***Not Supported***.

The behavior of a thread process consists of reset behavior, which executes on reset, and
operational behavior, which executes after reset.

The body of the thread process in a *<u>design</u>* *<u>shall</u>* follow the form:

```
    [<optional reset behavior>]
    [<optional operational behavior>]
    <infinite loop> subject to the restrictions below
```

The reset and operational behavior *<u>shall</u>* be separated by a call to the *wait()* function.
Any behavior encountered in the thread process prior to encountering the first call to the *wait()*
function *<u>shall</u>* be considered by the *<u>synthesis tool</u>* as reset behavior. Any behavior
encountered in the thread process after encountering the first call to the *wait()* function
*<u>shall</u>* be considered by the *<u>synthesis tool</u>* as operational behavior.

The first call to the *wait()* function *<u>may</u>* occur before the infinite loop, or it
*<u>may</u>* occur within the infinite loop. In the case where the first call to the *wait()*
function occurs within the infinite loop, the behavior within the loop body prior to the first call
to the *wait()* function *<u>shall</u>* be considered by the *<u>synthesis tool</u>* to be both
reset behavior and operational behavior.

Multiple calls to the *wait()* function *<u>may</u>* occur within the body of the thread process,
both before the infinite loop, and within the body of the infinite loop. The behavior encountered
before the first call to the *wait()* function *<u>shall</u>* be the reset behavior; any behavior
encountered after the first call to the *wait()* function *<u>shall</u>* be the operational behavior.

Within the body of the thread process, the first call to the *wait()* function *<u>shall</u>* be
statically determinable. The first call to the *wait()* function *<u>may</u>* be within conditional
constructs only when the condition is statically determinable (e.g., in the case where the condition
is based on a template parameter). Subsequent calls to the *wait()* function (in the operational
behavior) *<u>may</u>* occur within conditional constructs.

Reset values *<u>shall</u>* be statically determinable constants.

The following forms of infinite loop are ***Supported***, others are ***Not Supported***.

```cpp
    while (1) { }
```
```cpp
    while (true) { }
```
```cpp
    do { } while (1);
```
```cpp
    do { } while (true);
```
```cpp
    for ( ; ; ) { }
```

*Example 1 of SC_THREAD/SC_CTHREAD body:*

```cpp
    void dut::thread_process() {
        // Reset behavior goes here

        wait(); // This wait separates reset behavior from operational behavior.

        // You can have operational behavior here

        while(1) {
            // The infinite loop prevents the operational behavior
            // from terminating.

            // Repeated operational behavior.

            wait(); // This wait allows suspension of the process.
        }
    }
```

*Example 2 of SC_THREAD/SC_CTHREAD body:*

```cpp
    void dut::thread_process() {
        // Reset behavior goes here.

        // You cannot have operational behavior here, because there
        // is no wait() to separate them from reset behavior.
        while(1) {
            // The infinite loop prevents the process from terminating.

            // Behavior here is both reset behavior and operational behavior.

            wait(); // This wait separates reset behavior from  operational
                    // behavior and also allows suspension of the process.

            // Operational behavior.
        }
    }
```

***
**Note:**

Reset behavior is behavior that is executed when reset is asserted. As Section
[5.4](05-modules.md#54-constructors) indicates, writing to data members, signals, or
ports of a module from inside the *SC_MODULE* constructor is not reset behavior.

Default constructors of data members of the *SC_MODULE* that initialize the data members
(e.g., the default constructor for *sc_int* initializes the value to 0), are not part of the
reset behavior. If the behavior of the *<u>design</u>* depends on those non-reset initial
values, then synthesis results might differ from simulation results.

***

*Example:*

```cpp
class X {
    public:
    X() {
            m_1 = 0;
    }
    ...
    private:
    int m_1;
};

class XChild : public X {
    ...
};

SC_MODULE(Module) {
    sc_signal<X>        SC_NAMED(xSig);
    sc_signal<XChild>   SC_NAMED(xChildSig);
    sc_in_clk           SC_NAMED(clk);
    sc_in<bool>         SC_NAMED(rst);

    SC_CTOR(Module)     // Warning: Synthesis will not invoke default
                        //          constructors for xSig and xChildSig!
    {
        SC_CTHREAD(proc, clk.pos());
        reset_signal_is(rst, true);
    }
    void proc() {
        // Reset clause
        X x_tmp; // OK: Invoke default constructor for X.
        xSig = x_tmp; // OK: Initialize xSig with x_tmp.
        xChildSig = XChild(); // OK: Initialize xChildSig by the  default
                              //     constructor.
        wait();
        while (true) {
            // Main loop
            ...
        }
    }
};
```
