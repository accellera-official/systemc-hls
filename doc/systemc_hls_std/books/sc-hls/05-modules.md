---
style: chapter
---

# 5. Modules

This section specifies the SystemC modules used in a *<u>design</u>*. In this document
`sc_module` and module are used interchangeably.
A module models a hardware block and a system is described by creating a hierarchy of submodules
stemming from a parent module.

This section does not limit the optimizations that can be performed on the model, the scope of which
depends on the *<u>synthesis tool</u>* itself. Irrespective of the level of optimizations performed,
the result of the synthesis *<u>shall</u>* have the same functionality as the input model, but
*<u>may</u>* have different sequential timed behavior.

## 5.1 Module definitions

A SystemC module, as defined in the SystemC LRM, *<u>may</u>* contain the port-level
interfaces, any required internal storage elements, and any required behavior for that module.

The ***Supported*** possibilities for module definition are as follows.

- Use of the `SC_MODULE` macro;

- Derivation of a *<u>class</u>* from `sc_module`.

In addition to the above, templated module definition has ***Restricted Support***.

The restriction is that, for the top-level modules, only modules that have been instantiated
(as defined in ISOC++ 17.7.1 and 17.7.2) or have been explicitly specialized
(as defined in ISOC++ 17.7.3) are ***Supported***.

Template specialization and partial-specialization of modules is also ***Supported***.

***
**Note:**

In C++, template classes and functions are not instantiated until they are instantiated either
* implicitly, as defined in ISOC++ 17.7.1, or
* explicitly, as defined in ISOC++ 17.7.2.

In C++, template classes and functions can be explicitly specialized (as defined in ISOC++ 17.7.3)
using the `template<>` declaration (i.e., the class is fully specialized).

Examples of explicit instantiation and explicit specialization are shown below.
The most common form for synthesis is the use of implicit instantiation for submodules.
Note that explicit specializations are less common and only indicated when there
is an advantage in providing a specialized version of the module
(the example below is not an illustration of such a case).

```cpp
template <int N>
SC_MODULE(design) {
    sc_in <int>   SC_NAMED(a);
    sc_out<int>   SC_NAMED(c);

    void add() { c = a + N;}

    SC_CTOR(design) {
      SC_METHOD(add);
      sensitive << a;
    }
};

// EXPLICIT INSTANTIATION:
template class design<5>;

template<>
SC_MODULE(design<3>) // EXPLICIT SPECIALIZATION
{
    sc_in <int> SC_NAMED(a);
    sc_out<int> SC_NAMED(c);

    void add() { c = a + 3; }

    SC_CTOR(design) {
        SC_METHOD(add);
        sensitive << a;
    }
};
```
***

## 5.2 Data members for hardware interface

A module’s member specification consists of a set of declarations and definitions. These data members are classified into **interface data members** and **non-interface data members**.

**Interface data members** define the externally visible interface of a module and are used to transfer data into and out of the module.

- **Ports**: SystemC ports, declared using the `sc_in` and `sc_out` constructs, describe the signal-level interface at the module boundary. The bidirectional port `sc_inout` is ***Not Supported***, see Section [7.2](07-channels.md#72-basic-ports).
- **Signals**: Signals, declared using the `sc_signal` construct, enable communication between processes within a module and between modules through their ports.
- **Memories**: Modules can also communicate via shared memories. A memory shared between modules is modeled as an array passed to the constructors of the communicating modules.

All these mechanisms are ***Supported with Restrictions***, as detailed in Section [7](07-channels.md#7-communication-signals-ports-channels-and-memories).
Any other type of data member used for inter-process or inter-module communication is ***Not Supported***.

**Non-interface data members** are ***Supported with Restrictions*** as specified in
Sections [9](09-declarations.md#9-declarations), [10](10-declarators.md#10-declarators) and
[14](14-classes.md#14-classes).

## 5.3 Member functions

Module member functions are ***Supported with Restrictions*** as described in Section [6](06-processes.md#6-processes) for processes and Section [10](10-declarators.md#10-declarators) for all other functions.

## 5.4 Constructors

Module constructors are ***Supported*** through the use of
- the `SC_CTOR` macro, or
- a constructor function with a `const sc_module_name&` argument, or
- a constructor function with a `const sc_module_name&` argument and array references.

Every module declaration *<u>shall</u>* contain at least one declaration or definition of a constructor
member function.

The module constructor is used for design elaboration, to instantiate and connect the modules, start and initialize the processes.

To that end, within the constructor of an `SC_MODULE` the following is ***Supported***:

1. Constructor calls for interface ports and internal signals `sc_in`, `sc_out` and `sc_signal` in the constructor initializer list

2. Constructor calls for `SC_MODULE` sub-module hierarchy:
    - initialization of sub-module data members in the constructor initializer list
    - initialization of sub-module pointer data members using the `new` operator

3. Port bindings between the sub-modules ports and signals or top-module ports, using their `bind()` or `operator()` functions

4. Creation of processes using `SC_THREAD`, `SC_CTHREAD` and `SC_METHOD`, and sensitivity specification for these processes, as described in Section [6](06-processes.md#6-processes).

5. Initialization of constant data members in the constructor initializer list.

6. Initialization of array pointer or references in the constructor initializer list.

Within the constructor body, control-flow statements are ***Supported with Restrictions***: the expressions used in decisions and bounds *<u>shall</u>* be constant expressions.
Function calls within a constructor are ***Supported with Restrictions***, as described in Section [11.2](11-expressions.md#112-function-call). An additional constraint applies: the body of any function called from within a constructor *<u>shall</u>* be inlined at the call site.

Within the constructor body, the following is  ***Not Supported***:
- writing to data members, other than for the initialization explicitly specified above
- calls to `wait()` statement
- calls to the `read()` or `write()` functions of a port or signal.

Once the design structure is elaborated, in a hardware sense, the design structure shall never change. To that end, overwriting pointers or references to `sc_module` or memory array variables after constructor initialization is ***Not Supported***.

***
**Note:**

Initialization of an `SC_MODULE` data member should be performed within the reset portion of an
`SC_THREAD`, `SC_CTHREAD`, or sequential `SC_METHOD` in order to have the initialization be
performed at reset time.

***

## 5.5 Ignored member functions

The following member functions are ***Ignored***:
- `void set_stack_size( size_t );`
- `virtual void start_of_simulation();`
- `virtual void end_of_simulation();`

Additionally, all module destructors are ***Ignored***.

## 5.6 Not Supported member functions

The following functions are ***Not Supported*** by the synthesis tool:
- `virtual void before_end_of_elaboration();`
- `virtual void end_of_elaboration();`
- `void next_trigger();`
- `void next_trigger( const sc_event& );`
- `void next_trigger( const sc_event_or_list & );`
- `void next_trigger( const sc_event_and_list & );`
- `void next_trigger( const sc_time& );`
- `void next_trigger( double , sc_time_unit );`
- `void next_trigger( const sc_time& , const sc_event& );`
- `void next_trigger( double , sc_time_unit , const sc_event& );`
- `void next_trigger( const sc_time& , const sc_event_or_list &);`
- `void next_trigger( double , sc_time_unit , const sc_event_or_list & );`
- `void next_trigger( const sc_time& , const sc_event_and_list & );`
- `void next_trigger( double , sc_time_unit , const sc_event_and_list & );`
- `void wait( const sc_event& );`
- `void wait( const sc_event_or_list &);`
- `void wait( const sc_event_and_list & );`
- `void wait( const sc_time& );`
- `void wait( double , sc_time_unit );`
- `void wait( const sc_time& , const sc_event& );`
- `void wait( double , sc_time_unit , const sc_event& );`
- `void wait( const sc_time& , const sc_event_or_list & );`
- `void wait( double , sc_time_unit , const sc_event_or_list & );`
- `void wait( const sc_time& , const sc_event_and_list & );`
- `void wait( double , sc_time_unit , const sc_event_and_list & );`

The copy constructor and the assignment operator (ISOC++ 15.8) are disabled for `sc_module` by the SystemC LRM 5.2.2 and are therefore ***Not Supported***.

## 5.7 Port bindings

Port binding, as described in SystemC LRM 4.2.4 is ***Supported***.

Calling the binding function for a port that is a data member of a module, from the constructor of the parent module, is ***Supported***.

Note that positional binding (as described in SystemC LRM 5.2.19) is ***Not Supported***.

## 5.8 Deriving modules

Deriving modules, as defined in SystemC LRM 5.2.3, is ***Supported***.

*Example:*

```cpp
// Deriving a module:
SC_MODULE(BaseModule) {
    sc_in<bool>    SC_NAMED(reset);
    sc_in<bool>    SC_NAMED(clock);

    BaseModule(const sc_module_name& n) : sc_module(n) {}
};

class DerivedModule : public BaseModule {
    void newProcess() { /* ... */ }

public:
    DerivedModule(const sc_module_name& n) : BaseModule(n)
    {
        SC_CTHREAD(newProcess, this->clock.pos());
        reset_signal_is(this->reset, true);
    }
};
```

## 5.9 Selecting the top of a design hierarchy

The mechanism for specifying the top-level module(s) of a *<u>design</u>* is *<u>synthesis tool</u>*
dependent and is not defined in this standard.