---
style: chapter
---

# 10. Declarators

Declarators, as defined in ISOC++ Section 11, have ***Restricted Support***.

## 10.1 Type names

***Supported*** as defined in ISOC++ 11.1.

## 10.2 Ambiguity resolution

***Supported*** as defined in ISOC++ 11.2.

## 10.3 Kinds of declarators

### 10.3.1 Pointers

Pointers have ***Restricted Support***. Pointers that are statically determinable are
***Supported***. Otherwise, they are ***Not Supported***. Statically determinable implies the
*<u>synthesis tool</u>* is able to determine the actual object whose address is contained by the
pointer. If the pointer points to an array, the size of the array *<u>shall</u>* also be
statically determinable.

Pointer arithmetic, and the use of pointer values as data, are ***Not Supported***.

This includes operations such as:
- Testing whether a pointer is equal to zero (`nullptr` or `0`)
- Using a pointer value in hashing or other data-dependent computations

### 10.3.2 References

***Supported*** as defined in ISOC++ 11.3.2.

### 10.3.3 Pointers to nonstatic class members

Pointers to members using `::*` and access via `->*` and `.*` have ***Restricted Support***. They
are ***Supported*** as described in ISOC++ 11.3.3 and 8.5, subject to the restrictions described in
Section [10.3.1](#1031-pointers) on pointers.


### 10.3.4 Arrays

The element type of an array (ISOC++ 11.3.4) *<u>shall</u>* be any of the types which are permitted by ISOC++ as
element types and which are ***Supported*** for synthesis; or any SystemC data
type which is ***Supported*** for synthesis and which conforms to the requirements on element types
stated in ISOC++.

Arrays of type `std::array` and C-style arrays are ***Supported***.

Any array declaration *<u>shall</u>* specify its dimensions either:
- **Explicitly**, when no initializer is provided, or
- **Implicitly**, as inferred from the initializer, when one is specified.

Array dimensions *<u>shall</u>* be compile-time constants and *<u>shall not</u>* be modified during the array’s lifetime.

Statically allocated arrays are ***Supported***. Dynamically allocated arrays are ***Not Supported***, except as allowed in Section
[11.5.4](11-expressions.md#1154-new-and-delete).


See Section [17.1.4](17-libraries.md#1714-standard-c-array) for more information on `std::array` support.

***
*Examples:*
```cpp
// Static allocation:
sc_uint<32>                  buf1[64];      // Supported
sc_ufixed<8, 3>              buf2[32][64];  // Supported
std::array<sc_uint<16>, 256> mem;           // Supported

int numbers[] = {1, 2, 3, 4, 5};  // Supported, size deduced as 5 from initializer

// Dynamic allocation:
int* buf1 = new int[64];        // Not Supported
delete[] buf1;                  // Not Supported
sc_uint<32>* buf2 = new sc_uint<32>[64];    // Not Supported
```


### 10.3.5 Function parameters

Function parameters (ISOC++ 11.3.5) have ***Restricted Support***. The restriction is that ellipsis
(`...`) function parameters are ***Not Supported***.

### 10.3.6 Default arguments

Default arguments (ISOC++ 11.3.6) are ***Supported***.

## 10.4 Function definition

Function definitions (ISOC++ 11.4) are ***Supported***.

### 10.4.1 Explicitly-defaulted functions

Explicitly-defaulted functions (ISOC++ 11.4.2) are ***Supported***.

### 10.4.2 Deleted definitions

Deleted definitions (ISOC++ 11.4.3) are ***Supported***.

## 10.5 Structured binding declarations

Structured binding declarations (ISOC++ 11.5) are ***Supported***.
Structured bindings allow decomposing an object into its constituent elements, providing a concise syntax for accessing members of structs, pairs, tuples, and arrays.

## 10.6 Initializers

Initializers (ISOC++ 11.6) are ***Supported***.

### 10.6.1 Aggregates

Initialization of aggregates (ISOC++ 11.6.1) is ***Supported***.

***
**Note:**

ISOC++ extends aggregate initialization to support public, non-virtual base classes, allowing initialization of derived aggregates that include base class members.

***

### 10.6.2 Character arrays

Initialization of character arrays (ISOC++ 11.6.2) is ***Supported***.
Also see Section [8.1.1.1](08-types.md#8111-literals) for character literals that are supported.

### 10.6.3 References

Initialization of references (ISOC++ 11.6.3) is ***Supported***.

### 10.6.4 List-initialization

List-initialization (ISOC++ 11.6.4) is ***Supported***.
