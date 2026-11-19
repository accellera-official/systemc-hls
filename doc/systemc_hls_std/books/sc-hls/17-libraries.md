---
style: chapter
---

# 17. Libraries

This section outlines support for external C++ or SystemC libraries.

## 17.1 Standard C and C++ libraries

Generally, library functions from C/C++ have ***Restricted Support***. Most of them are ***Not Supported***,
while some are ***Ignored*** as described below.


### 17.1.1 Standard console I/O

Stream-based I/O (C++ Standard Library `<iostream>`):
- The `operator<<` when applied to `cout` or `cerr` is ***Ignored***.
- The `operator<<` when applied to `std::ostream` is ***Ignored***, except when applied to the file
  stream types listed in Section [17.1.2](17-libraries.md#1712-standard-file-streaming), where it is
  ***Not Supported***.

C Standard Library (`<cstdio>`):
- All `printf` functions and `puts` are ***Ignored***.

Functions or operators used to compute arguments for an Ignored function are not considered part of the Ignored construct.
Specifically, if an argument to an Ignored function includes a side effect, that side effect *<u>should</u>* be supported, but a synthesis tool may choose to reject the input with an appropriate error message.

***
*Examples:*
```cpp
printf("x = %d", x);  // Ignored
printf("y = %d", ++y); // Side effect ++y is Supported
cout << "z = " << z << endl; // Ignored
```
### 17.1.2 Standard file streaming

C Standard Library (`<cstdio>`):
- The following functions for file input/output operations are ***Not Supported***:
  `fopen`, `freopen`, `fclose`, `fread`, `fwrite`, `fseek`, `ftell`, `fgetc`, `fscanf`,
  `fputs`.

C++ Standard Library (`<fstream>`):
- The following constructs for file streaming are ***Not Supported***:
  - `ifstream` and `ofstream` constructors
  - `operator<<` applied to `fstream`
  - `operator>>` applied to `fstream`
  - `operator<<` applied to `ofstream`
  - `operator>>` applied to `ifstream`

### 17.1.3 Type traits

The C++ metaprogramming facilities provided in the `<type_traits>` header of the standard library are ***Supported*** for all constructs defined up to the ISO C++17 standard.

### 17.1.4 Standard C++ array

Usage of the standard C++ array class (`std::array`), defined in `<array>` is ***Supported with Restrictions***.

A `std::array` *<u>shall</u>* have a constant size greater than zero. Multidimensional arrays of type `std::array` are ***Supported*** and *<u>shall</u>* be of constant size in every dimension.

For `std::array`, the following member types are ***Supported***:
- `value_type`
- `size_type`
- `difference_type`
- `reference`
- `const_reference`
- `iterator`
- `const_iterator`
- `reverse_iterator`
- `const_reverse_iterator`

For `std::array`, usage of the following member types is ***Not Supported***:
- `pointer`
- `const_pointer`

For `std::array`, the following member functions are ***Supported***:
- `at()`
- `operator[]`
- `front()`
- `back()`
- `begin()`
- `cbegin()`
- `end()`
- `cend()`
- `rbegin()`
- `rend()`
- `crbegin()`
- `crend()`
- `empty()`
- `size()`
- `fill()`

For `std::array`, usage of the following member functions is ***Not Supported***:
- `data()`
- `swap()`
- `max_size()`

For `std::array`, the following non-member functions are ***Supported***:
- `get()`
- `operator==`

For `std::array`, usage of the following non-member functions is ***Not Supported***:
- `operator!=`
- `operator<`
- `operator<=`
- `operator>`
- `operator>=`
- `swap()`

## 17.2 SystemC functions and types

### 17.2.1 Tracing

The following tracing constructs are ***Ignored***:

- declaration of a variable of type `sc_trace_file*`
- assignment to a variable of type `sc_trace_file*`,
- calls to `sc_create_vcd_trace_file`,
- calls to `sc_close_vcd_trace_file`,
- declaration or definition of any function named `sc_trace`, and
- calls to any function named `sc_trace`.

### 17.2.2 `set_stack_size`

***Ignored***.

### 17.2.3 `sc_gen_unique_name`

***Ignored***.

### 17.2.4 `before_end_of_elaboration` and `end_of_elaboration`

***Not Supported***.

### 17.2.5 `dont_initialize`

***Supported***.

### 17.2.6 `next_trigger`

***Not Supported***.

### 17.2.7 `print` and `dump`

The SystemC functions `print` and `dump` are ***Ignored***.

### 17.2.8 `kind`

***Ignored***.

### 17.2.9 `sc_report_handler`

The `sc_report_handler` class has ***Restricted Support***.
The member function `sc_report_handler::report` is ***Ignored***.
All other member functions of `sc_report_handler` are ***Not Supported***.

***
**Note:**

This implies that the macros `sc_assert`, `SC_REPORT_FATAL`, `SC_REPORT_ERROR`, `SC_REPORT_WARNING` and `SC_REPORT_INFO` are ***Ignored***.

***