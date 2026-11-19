---
style: chapter
---

# 8. Types

SystemC types are comprised of both the native C++ types and the additional SystemC types.

There are two kinds of native C++ types: fundamental types and compound types. Types describe
objects, references, or functions.

***
**Note:**

Alignment requirements mentioned in ISOC++ 6.11 are not relevant for synthesis.
Synthesis may choose alternative data representations for internal objects (not part of the
interface of the *<u>design</u>*) provided the I/O behavior of the *<u>design</u>* is unchanged.

For example, the bit-width of an integer variable could be reduced based on the range of the
variable or its representation could be changed from two’s complement to sign-magnitude.

***

## 8.1 Fundamental types

Fundamental types are comprised of integer types, floating-point types, and *void*.

### 8.1.1 Integer types

The following integer types, as specified in ISOC++ 6.9.1, are ***Supported***:

- *bool*
- *unsigned char*, *signed char*, *char*
- *unsigned short*, *signed short*
- *unsigned int*, *signed int*
- *unsigned long*, *signed long*
- *unsigned long long*, *signed long long*

The integer types *wchar_t, char16_t* and *char32_t* are ***Not Supported***.

***
**Note:**

ISOC++ 6.9.1 specifies that it is implementation defined whether a *char* object can hold negative
values; GNU G++ can support either mode through use of the “*-fsigned-char*/*-funsigned-char*” switch.
The *<u>synthesis tool</u>* vendor is likely to choose the mode that best meets the architecture for
which their simulation model was built in order to achieve consistent results between simulation and
synthesis models.

ISOC++ 6.9.1 also specifies that plain *char*, *signed char*, and *unsigned char* are three distinct
types. Even if a particular implementation allows a *char* to hold negative values, it is not the
same type as a *signed char*.

***

#### 8.1.1.1 Literals

Integer and Boolean literals, as specified in ISOC++ 5.13.2 and 5.13.6, are ***Supported***.

Floating literals, as defined in ISOC++ 5.13.4, have ***Restricted Support***, see Section
[8.1.4](08-types.md#814-floating-point-types).

Pointer literals, *nullptr*, as defined in ISOC++ 5.13.7 are ***Supported***.

Character literals as described in ISOC++ 5.13.3 have ***Restricted Support***. The *L* prefix
denoting wide character support is ***Not Supported***.

For synthesis, if the numerical value of the *char* literal has an effect on functionality (the
exception being comparing *char*s for equality), characters *<u>shall</u>* be assumed to be encoded
in the ASCII character set. This is a *<u>synthesis refinement</u>* over ISOC++ 5.13.3 and 5.13.5,
which allows alternative execution character sets (ISOC++ Section 5.3, Paragraph 3).

#### 8.1.1.2 Representation and bit sizes

Two’s complement integer representations are ***Supported***. One’s complement and sign magnitude
integer representations are ***Not Supported***.

A *<u>synthesis tool</u>* *<u>shall</u>* have mechanisms to support the I/O observable behavior
implied by bit-widths and representation for the computer platforms as indicated in the last column
of the table below. It *<u>shall</u>* warn in case the choice of bit-width or the representation is
not consistent with the definition for the computer platform.

{% include table
   reference="Table 1"
   caption="Bit sizes for integer types (for current compilers)"
   class="fixed allow-break"
   caption-position="top"
   markdown="
| Integer Type          | Relative Requirement         | Representation   | Bit Width |
| :-                    |                           -: |               -: |        -: |
| (un)signed char, char |                              | two’s complement | 8         |
| (un)signed short      | bits(short) ≥ bits(char)     | two’s complement | 16        |
| (un)signed int        | bits(int) ≥ bits(short)      | two’s complement | 32        |
| (un)signed long       | bits(long) ≥ bits(int)       | two’s complement | 32/64     |
| (un)signed long long  | bits(long long) ≥ bits(long) | two’s complement | 64        |
"%}

***
**Note:**

1. The representation and the bit-width of an integer type determines its numerical range and its
overflow behavior.

2. The ISOC++ standards set minimum requirements for the bit widths of integer types, but leave bit
widths and the representation implementation-dependent.

3. ISOC++ 6.9.1 specifies unsigned integers *<u>shall</u>* obey the laws of arithmetic modulo
2*<sup>n</sup>*, where *n* is the number of bits. Signed integers are of a pure binary numeration
system and the representations allowed are two’s complement, one’s complement, and sign magnitude.

4. Table 1 provides an overview of the ISOC++ 6.9.1 requirements and bits sizes for integer types
used on most compilers for popular computer platforms. It constrains the relative sizes of the
different integer types, and also requires the signed and unsigned (and plain in the case of
characters) versions of integer types to have the same storage. The last column in the table shows
the bit widths for current platforms. As the table indicates, there is only a difference for the
(un)signed long types in current platforms.

5. ISOC++ provides optional typedefs (ISOC++ 21.4.1) for "exact-width" integer types. These are *intN_t* and *uintN_t*, where
*N* can be 8, 16, 32, or 64. The typedefs are defined in the *std* namespace in the include
*&lt;cstdint&gt;* header. For example, int64_t is defined as *long* on platforms where *long* is
64-bits wide and as *long long* on platforms where *long* is 32-bits wide.

***

### 8.1.2 Type conversions

Type conversions are ***Supported*** as specified in the sections below.

***
**Note:**

ISOC++ defines two kinds of conversions between integer types that are applied in the evaluation of
expressions: integer promotions and usual arithmetic conversions.

An example of an integer promotion is when a *short* is promoted to an *int* in the unary minus
expression “-*a*” (variable “*a*” is of type *short*).

The usual arithmetic conversions are defined by the C++ language to yield a common type for many
binary operators that expect operands of arithmetic or enumeration type.

An example of a usual arithmetic conversion is when an operand of type *short* is converted to
*long long* in the expression “*a*+*b*” where “*a*” is of type *short* and “*b*” is of type
*long long*. In that case, “*a*” is first promoted to type *int* (integer promotion that is
performed as part of the usual arithmetic conversion) and then converted to *long long*.

***

#### 8.1.2.1 Integer promotions

Integer (Integral) Promotions (as defined in ISOC++ 7.6) are ***Supported***.

#### 8.1.2.2 Usual arithmetic conversions

Usual Arithmetic Conversions (as defined in ISOC++ Section 8) are ***Supported***.

### 8.1.3 Operators

The following operators are ***Supported*** for the integer types:

1. Unary operators (+, -, ~, and !).
2. Arithmetic binary operators (+, - , \*, /, and %) and the corresponding assign operators
   (+=, -=, \*=, /=, and %=).
3. Relational and Equality operators (\>, \>=, \<, \<=, ==, and !=).
4. Bitwise binary operators (&, \|, and ^) and the corresponding assign operators (&=, \|=, and ^=).
5. The Conditional Operator (?:).

The following operators have ***Restricted Support***.

1. Shift and shift assign operators (\<\<, \>\>, \<\<=, and \>\>=). Considering *E1* and *E2* as the
   two operands (as in “*E1* *shift_op* *E2*”), the support is as outlined below. The support of the
   assign shift operators (\<\<=, \>\>=) follow from the support of the shift operators (\<\<, \>\>).
    1. Shifts are ***Supported*** for valid ranges as defined by ISOC++ and ***Not Supported***
       otherwise. ISOC++ 8.8 specifies that valid ranges for *E2* are 0 to
       *length*(*promoted_type*(*E1*))-1 and that otherwise the behavior is undefined (Section 2.2.2).
    2. When *E1* has an unsigned type: both right and left shifts are ***Supported***.
    3. When *E1* has a signed type the shift operators are ***Supported*** with the following
       *<u>synthesis refinements</u>*:
        - For right shifts, the sign bit *<u>shall</u>* be shifted in. This is a
          *<u>synthesis refinement</u>* since ISOC++ leaves the behavior implementation-defined when
          *E1* is negative.
        - For left shifts, ISOC++ specifies cases where the shift is undefined. A
          *<u>synthesis refinement</u>* on ISOC++ is that *E1 \<\< E2*, for *E1* signed, is defined
          in all cases to be equivalent to  *signed(unsigned(promoted_type(E1)) \<\< E2)*.
2. The prefix and postfix increment and decrement operators (`++x`, `--x`, `x++`, and `x--`) have
   ***Restricted Support***. The restriction is that the prefix and postfix increment and decrement
   operators on a *bool* operand are ***Not Supported*** as they are not allowed by ISOC++ (8.2.6 and 8.3.2).

***
**Note:**

The ~ operator on a *bool* argument x first promotes it to an *int* and then computes the one’s
complement (value is -x-1) of the promoted value (ISOC++ 8.3.1).


```cpp
    bool x = true;
    int  y = ~x; // y is -2
    bool z = ~x; // z is true
```

If the intended behavior is to get the logical complement, the logical negation operator *!*
(ISOC++ 8.3.1) should be used.

The result of division is truncated towards zero.

***

### 8.1.4 Floating point types

Floating literals (as specified in ISOC++ 5.13.4) are ***Supported*** for initializing synthesizable
datatypes. If the floating literal is used to initialize an integer type (a floating-integral
conversion is involved as specified in ISOC++ 7.10) and the truncated value is not representable in
the integer type, then the behavior is undefined (see Section 2.2.2).

Otherwise, floating-point types (specified in ISOC++ 6.9.1 as *float*, *double*, and *long double*)
are ***Not Supported***.

***
**Note:**

One of the challenges for providing general synthesis support for floating-point datatypes is the
fact that the bit accurate behavior of floating point arithmetic is dependent on implementations and
compiler options used.

***

### 8.1.5 The void type

The *void* type (ISOC++ 6.9.1) is ***Supported***.

## 8.2 Compound types

Compound types in C++, as described in ISOC++ 6.9.2, have ***Restricted Support*** determined by the
support restrictions of the constituent types.

## 8.3 SystemC datatypes

SystemC provides a number of datatypes that are useful for hardware design. These datatypes are
implemented as C++ classes.

The following SystemC types have ***Restricted Support*** as described in the sections below:

- Limited Precision Integer Types: *sc_int* and *sc_uint* (Section 8.3.1)
- Finite Precision Integer Types: *sc_bigint* and *sc_biguint* (Section 8.3.2)
- Finite Precision Fixed-point Types: *sc_fixed* and *sc_ufixed* (Section 8.3.3)
- Finite Word-Length Bit Vector Type: *sc_bv* (Section 8.3.4.1)
- Finite Word-Length Logic Vector Types (4-valued): *sc_lv* (Section 8.3.4.2)
- Single Bit Logic (4-valued): *sc_logic* (Section 8.3.4.2)

The explicit use of SystemC datatypes that are not in the list above is ***Not Supported***.
The implicit use of other related types that arise as return types from operators on the types
listed above has ***Restricted Support***.
The restriction is that the bitwidth of a return type needs to be statically determinable.

***
**Note:**

1. All datatypes supported for synthesis have vector length/precision that is specified by template
parameters. Thus, their vector length/precision is statically determinable during compilation.

2. Underlying classes, such as *sc_signed* and *sc_unsigned*, can appear as the result of expressions
on supported types, but are not directly synthesizable. Their length/precision is dynamic. Some base
and helper classes are specifically denoted in SystemC LRM 3.2.4 as classes that should not be used
explicitly. The SystemC LRM annotates those classes with a superscript dagger ( <sup>†</sup> ).

***

### 8.3.1 Limited precision integer types

SystemC LRM 7.5.2 and 7.5.3 state that the limited precision *sc_int/sc_uint* *<u>shall</u>* be held
in an implementation-dependent native C++ integer which *<u>shall</u>* have a minimum representation
size of 64 bits. For synthesis, representation sizes greater than 64-bits are ***Not Supported***.

In summary, the following types have ***Restricted Support***.

- *sc_int\<W\>*: limited precision signed integer (*W* ≤ 64).

- *sc_uint\<W\>*: limited precision unsigned integer (*W* ≤ 64).

Support restrictions on operators and functions are covered in Section 8.3.5. Many operators are
available through implicit conversions to *int_type* and *uint_type* and their support is determined
by the support of those native C++ types.

### 8.3.2 Finite precision integer types

The following Finite Precision Integer types have ***Restricted Support***.

- *sc_bigint\<W\>*: finite precision signed integer.

- *sc_biguint\<W\>*: finite precision unsigned integer.

The support restrictions of common operators and functions are covered in Section 8.3.5.

### 8.3.3 Finite precision fixed-point types

The fixed-point types *sc_fixed* and *sc_ufixed* have ***Restricted Support*** as listed below.

1. Overflow modes have ***Restricted Support*** as specified in Table 2 below as a function of the
   two template parameters that determine the overflow mode: *o_mode* and *n_bits*.
2. All Quantization modes are ***Supported*** as specified in Table 3 below as a function of the
   template parameter that determines the quantization mode: *q_mode*.
3. Common operators/functions: see Section 8.3.5.
4. Specific member functions:
    1. Query of parameters: *wl*, *iwl*, *q_mode*, *o_mode*, and *n_bits* are ***Supported***.
    2. Query of value: *is_neg*, *is_zero*, and *value* are ***Not Supported***.
    3. Other member functions such as *overflow_flag*, *quantization_flag*, *type_params*, and
       *cast_switch* are ***Not Supported***.
5. Simulation specific functionality such as *sc_fxtype_param*, *sc_fxcast_switch*, and *SC_OFF*
   are ***Not Supported***.


{% include table
   reference="Table 2"
   caption="Overflow Modes"
   class="fixed allow-break"
   caption-position="top"
   markdown="
|       Overflow Mode           | Parameter (o_mode)     | n_bits     |     Support     |
|:------------------------------|:-----------------------|-----------:|--------------------:|
| Wrap-around Basic (*default*) |   *SC_WRAP*            |     0      |   ***Supported***   |
|          Saturation           |    *SC_SAT*            |     \-     |   ***Supported***   |
|    Symmetrical Saturation     |  *SC_SAT_SYM*          |     \-     |   ***Supported***   |
|      Saturation to Zero       | *SC_SAT_ZERO*          |     \-     |   ***Supported***   |
|     Wrap-around Advanced      |   *SC_WRAP*            |    \> 0    | ***Not Supported*** |
|  Sign Magnitude Wrap-Around   |  *SC_WRAP_SM*          |    ≥ 0     | ***Not Supported*** |
"
%}


{% include table
   reference="Table 3"
   caption="Quantization Modes"
   class="fixed allow-break"
   caption-position="top"
   markdown="
|   Quantization Mode        | Parameter (q_mode)     |     Support     |
|:-                          |:-                      |               -:|
|   Truncation (*default*)   |        *SC_TRN*        | ***Supported*** |
| Rounding to plus Infinity  |        *SC_RND*        | ***Supported*** |
|     Truncation to zero     |     *SC_TRN_ZERO*      | ***Supported*** |
|      Rounding to zero      |     *SC_RND_ZERO*      | ***Supported*** |
| Rounding to minus infinity |    *SC_RND_MIN_INF*    | ***Supported*** |
|    Rounding to infinity    |      *SC_RND_INF*      | ***Supported*** |
|    Convergent rounding     |     *SC_RND_CONV*      | ***Supported*** |
"
%}

### 8.3.4 Logic and vector types

#### 8.3.4.1 Finite word-length bit vectors (sc_bv)

The finite word-length bit vector *sc_bv* has ***Restricted Support***.
The restrictions on common operators/functions are outlined in Section 8.3.5.

#### 8.3.4.2 Single-bit logic (sc_logic) and finite word-length logic vectors (sc_lv)

The single-bit logic *sc_logic* and the finite word-length logic vector *sc_lv* have
***Restricted Support***.
The restrictions on common operators/functions are outlined in Section 8.3.5.

In addition, the following restrictions apply.

- The unknown logic constant (`sc_logic('X')`, `SC_LOGIC_X`) is ***Not Supported***.

- The high-impedance logic constant (`sc_logic('Z')`, `SC_LOGIC_Z`) is ***Not Supported***.

### 8.3.5 Common operators and functions

#### 8.3.5.1 Bit select operator

The bit select operators as specified by SystemC LRM 7.2.6 have ***Restricted Support***.
The restriction is that the index is within the bounds of the object being accessed.

***
**Note:**

An out-of-bound access will be treated as an error by synthesis as defined in Section 2.3.3.

The bit select *operator[i]* allows the selection of a bit of a variable either as an *rvalue* or
an *lvalue*.

As defined by SystemC LRM 7.5.4.6, *sc_int*/*sc_uint* temporary values cannot have bit-select applied.

As defined by SystemC LRM 7.5.7, 7.6.8, and 7.7, the bit select on concatenations or subreferences also
cannot be performed.

*Example:*

```cpp
sc_int<8>    x;
sc_bigint<8> y;

x[3] = y[2]; // Legal

(x+x)[3] = 0 ; // Illegal, as x+x is promoted to a native C++ type

(y+y)[3] = 0 ; // Legal as y+y is still a sc_bigint

(y,y)[3] = 0 ; // Illegal as concatenation doesn’t support bitref
```

***

#### 8.3.5.2 Part select operator

The part select operators (*operator(l,r)* and *range(l,r)*), as specified by SystemC LRM 7.2.7,
have ***Restricted Support***. The first restriction is both the left and the right index positions
lie within the bounds of the object. The second restriction is the range of the part select has a
statically determinable length. The third restriction is the left hand index is greater or equal to
the right hand index.

Out-of-bound access *<u>shall</u>* be treated as an error (Section 2.3.3), as specified in the
SystemC LRM.

***
**Note:**

The third restriction implies bit-reversal using part selects is not supported. Given the second
restriction, the third restriction becomes trivial to check.

The bit-reversal behavior is specified for fixed-point types, as specified in SystemC LRM 7.10.6.
It is also implied for vector types in the specification of *reversed* in SystemC LRM 7.9.8.7.
Part select is not allowed to reverse bit-order for limited-precision integers, as stated in NOTE1
in SystemC LRM 7.2.7, and for finite-precision integers, as stated in SystemC LRM 7.6.8.3.

Example of statically determinable range length:


```cpp
x(i+5, i+3) = y(k+4, k+2); // range length = 3
```

***
**Note:**

As defined by SystemC LRM 7.5.4.6, *sc_int*/*sc_uint* temporary values cannot have part-select
applied.

Part select is not available for concatenations and subrefs of integer types.
Part select is available for concatenations and subrefs of vector types (*sc_bv* and *sc_lv*).

*Example:*

```cpp
sc_int<8>    x;
sc_bigint<8> y;

x(5,3) = y(4,2); // Legal

(x+x)(5,3) = 0 ; // Illegal: x+x is promoted to native C++ type

(y+y)(5,3) = 0 ; // Legal as y+y is still a sc_bigint

(y,y)(5,3) = 0 ; // Illegal: part select of a concatenation of integers is not allowed
```

The result of a part select cannot be directly assigned to a fixed point variable, but it can be
assigned to a range.

***

#### 8.3.5.3 Function concat(C1,C2) and operator,(C1,C2)

The concatenation function and operator are ***Supported***.

***
**Note:**

The SystemC datatype package defines concatenation functionality via a template specialized function
*concat(C1,C2)* and *operator,(C1,C2)*.

The concatenation operation (*op1,op2*) may be used as an *rvalue* or an *lvalue*.

*Example:*

```cpp
(x, y) = (z, w);
```

Because of the difference in return types for operators for *sc_bigint*/*sc_biguint* and
*sc_int*/*sc_uint*, using expressions (unless they are cast) may give different results for
finite precision integers than for limited precision integers.
Using uncast expressions other than concatenation, bit select, and part select as arguments of
the concatenate operation is not recommended.

For example, using the Accellera Open Source simulator:

```cpp
sc_int<4>    t  = 1;
sc_bigint<4> tb = 1;
sc_int<4>    x  = 2;
sc_bigint<4> xb = 2;

cout << (t,  t*x)    << endl; // result = 3
cout << (t, tb*xb)   << endl; // result = 258
cout << (t,  x >> 1) << endl; // result = 3
cout << (t, xb >> 1) << endl; // result = 17
```

***

#### 8.3.5.4 Reduction operators

The reduction operators specified in SystemC LRM 7.2.9 are ***Supported***.

***
**Note:**

The reduce operators are: *and_reduce*, *or_reduce*, *xor_reduce*, *nand_reduce*, *nor_reduce*, and
*xnor_reduce*.

The reduction operators are not available for the fixed-point datatypes.

***

#### 8.3.5.5 Arithmetic operators

The arithmetic operators for limited and finite precision integers, as defined in the SystemC LRM,
are ***Supported***.

The arithmetic operators for the finite precision fixed-point types, as defined in the SystemC LRM,
have ***Restricted Support***. The implementation-dependent behavior described in SystemC LRM 7.10.7
(variable-precision fixed-point value limits) is ***Not Supported***.
A consequence is that the division operator is ***Not Supported***.

***
**Note:**

Examples of implementation-dependent behavior described in the SystemC LRM 7.10.7 can be found in the
Accellera open source simulator in the form of the following compiler flags:

- *SC_FIXDIV_WL:* bounds the number of bits that are computed for division. Division can require
  an infinite number of bits to represent.
- *SC_FXMAX_WL*: is the maximum width of a fixed-point type. It is presumed precision is set high
  enough so it does not change the behavior of the *<u>design</u>*. Synthesis presumes these limits
  are not present.

The behavior of operators `++` and `--` for fixed-point datatypes, while using the standard
increment/decrement by 1, is in effect a NOP whenever *iwl* > *wl* and in some cases of *iwl* \< 1.

The arithmetic operators are defined for the numeric types (integer and fixed-point types).

The arithmetic operators include:

- Unary operators + and -
- Binary operators +, -, \*, /, and %; assign operators +=, -=, \*=, /=, and %=.
  The operators % and %= are not available for fixed-point types.
- Prefix and Postfix increment and decrement operators `++` and `--`.

***

#### 8.3.5.6 Bitwise complement operator

The unary bitwise complement operator ~ specified in the SystemC LRM is ***Supported***.

***
**Note:**

The unary ~ operator complements its argument bitwise. For vector types, the width of the return
type is identical to the width of the operand. This is consistent with the vector types not having
an arithmetic view (no signed/unsigned treatment).
The integer and fixed-point types (these are referred to in SystemC LRM 7.1 as *numeric types*) are
intended to have an arithmetic treatment.  In an arithmetic context, ~x is equal to (-x-1).
However, the unary operators ~ for the SystemC integer and fixed-point types are not consistent with
an arithmetic treatment and are in fact inconsistent amongst themselves:

```cpp
cout << ~((sc_uint<8>)     128) << endl; //  18446744073709551487
cout << ~((sc_biguint<8>)  128) << endl; //  383
cout << ~((sc_ufixed<8,8>) 128) << endl; //  127
cout << ~((unsigned char)  128) << endl; // -129, correct ~x = (-x-1)
```

***

#### 8.3.5.7 Bitwise logical operators

The binary bitwise logical operators (&, \|, and ^) and bitwise assignment operators
(&=, \|=, and ^=) specified in the SystemC LRM are ***Supported***.

***
**Note:**

The binary operations &, \|, ^ compute the bitwise and, or and xor operation respectively.

The mixing of signed and unsigned operands is not allowed for fixed-point types
(a difference compared to SystemC integer types).

***

#### 8.3.5.8 Logical negation

The logical negation operator ! specified in the SystemC LRM is ***Supported***.

***
**Note:**

The operator ! is available for *sc_int_bitref*, *sc_uint_bitref*, *sc_signed_bitref_r*, and
*sc_unsigned_bitref_r*. Using the operator ! on fixed-point types leads to implicit conversions, so
there *<u>may</u>* be unexpected truncation involved.

The operator ! is available on limited precision types through implicit conversions to *int_type*
(C++ native type).

***


#### 8.3.5.9 Relational operators

Relational Operators, as defined in the SystemC LRM, are ***Supported***.

***
**Note:**

The relational operators compare the two operands as in C++ and return a value of type *bool*.
The comparison is done arithmetically for integer and fixed-point types.
The relational operators == and != operators are available for vector types.

Mixing signed and unsigned limited precision integers can lead to unexpected results.

*Example:*

```cpp
(sc_uint<8>) 1 > (sc_int<8>) -1 // incorrectly returns false
```

***

#### 8.3.5.10 Shift operators

##### 8.3.5.10.1 Limited precision integers

The shift operators (\<\< and \>\>) and shift assign operators (\<\<= and \>\>=) have
***Restricted Support***. The restriction is the second operand needs to be in the range 0 to 63.

***
**Note:**

The restriction is inherited from the implicit conversion to the native C++ 64-bit integers in its
implementation (see Section 8.1.3 and SystemC LRM 7.5.4.6). This restriction is also tied to the
restriction in Section 8.3.1 on the maximum bitwidth for limited precision integers.

Outside the range 0 – 63, the behavior is undefined (see Section 2.2.2).

***

##### 8.3.5.10.2 Finite precision integers

The shift operators (\<\< and \>\>) and shift assign operators (\<\<= and \>\>=) have ***Restricted Support***.
The first restriction is the second operand has to have a non-negative value.
The support for the left shift operator (\<\<) is also restricted to have a statically determinable
return bit-width, which implies the value of second operand is statically determinable.

The restrictions above also apply to the said operators applied on the base types
(*sc_value_base*, *sc_signed*, and *sc_unsigned*), and the types resulting from part selects
(*sc_signed_subref_r*, *sc_signed_subref*, *sc_unsigned_subref_r*, and *sc_unsigned_subref*) and
concatenations (*sc_concatref*).

***
**Note:** SystemC LRM 7.6.3.7 specifies the behavior is undefined (see Section 2.3.2) if the second
operand is negative.

***

##### 8.3.5.10.3 Finite precision fixed-point types

The shift operators (\<\< and \>\>) have ***Restricted Support***. The restriction is the length
and integer length of the return type have to be statically determinable, which implies the value of
the second operand is statically determinable.

The shift assign operators (\<\<= and \>\>=) have ***Restricted Support***. The restriction is the
value of the second operand is statically determinable for all cases other than when the first
operand has *q_mode=SC_TRN*, *o_mode=SC_WRAP*, and *n_bits=0*. This restriction guarantees no hardware
is required for handling rounding and/or saturation.

The restrictions above also apply to the said operators applied on the base types
(*sc_fxval*, *sc_fxnum*, *sc_fix*, and *sc_ufix*) and types resulting from part selects
(*sc_fxnum_subref*).

***
**Note:**

Fixed-point shifts are bidirectional (though this is not explicitly stated in the SystemC LRM) as
the second argument is a C++ *int*. The hardware cost can be minimized if the range of the second
argument can be reduced. For example, if the range analysis of the *<u>synthesis tool</u>* can
determine that the second argument is non-negative, then a unidirectional shift suffices.

***

##### 8.3.5.10.4 Finite word-length vector types

The shift operators (\<\< and \>\>) and shift assign operators (\<\<= and \>\>=) have
***Restricted Support***.
The first restriction is the second operand has to have a non-negative value.
The support for the left shift operator (\<\<) is also restricted to have a statically determinable
return bit-width, which implies the value of second operand is statically determinable.

The restrictions above also apply to the said operators applied on the base types
(*sc_lv_base* and *sc_bv_base*), and the types resulting from part selects
(*sc_subref_r* and *sc_subref*) and concatenations (*sc_concref_r* and *sc_concref*).

***
**Note:**

SystemC LRM 7.9.3.7, 7.9.4.7, and 7.9.8.6 specify it is an error (Section 2.3.3) if the second
operand is negative. The SystemC LRM is silent on what happens when the second operand is negative
for the classes resulting from concatenation (*sc_concref_r* and *sc_concref*).

***

#### 8.3.5.11 Rotate operators

The rotate operators *lrotate* and *rrotate* have ***Restricted Support***. The first restriction is
the argument (the amount by which to rotate) is not negative. The second restriction is the value of
the argument is statically determinable.

***
**Note:**

The rotate operators are available for the vector types (*sc_lv* and *sc_bv*), their base classes
(*sc_lv_base* and *sc_bv_base*), and the types resulting from part select (*sc_subref*) and
concatenations (*sc_concref*) of vector classes.

The SystemC LRM does not explicitly define the behavior for negative values.

***

#### 8.3.5.12 Explicit conversions

Explicit conversions have ***Restricted Support*** as outlined below.

- Conversions *to_int*, *to_uint*, *to_long*, *to_ulong*, *to_int64*, and *to_uint64* are
***Supported***.

<!-- -->

- The following conversions are ***Not Supported***:
    - *to_float* and *to_double*;
    - *to_char* (conversion to character for *sc_logic* and bit select for vector types);
    - *to_string* (and its shortcut variants *to_dec*, *to_hex*, *to_oct*, and *to_bin*).
- Conversion *to_bool* is
    - ***Not Supported*** for types *sc_logic* and vector types;
    - ***Supported*** for bit-select for the limited and finite precision integers.
- Member function *is_01* (the SystemC LRM labels this as an explicit conversion function) is
  ***Not Supported***.
- Member function *value* (for *sc_logic* and vector types) is ***Not Supported***.

#### 8.3.5.13 Conversion operators

The conversion operators have ***Restricted Support***. The restriction is the implicit conversion
operator is to a supported type.

***
**Note:**

The following conversion operators are defined in the SystemC LRM, along with the classes that
define them.

| conversion operator      | classes where defined |
| :-                       | :-                    |
| `operator int_type()`    | limited precision integers. |
| `operator uint_type()`   | limited precision integers. |
| `operator uint64()`      | *sc_int_bitref_r*, *sc_uint_bitref_r*, *sc_signed_bitref_r*, *sc_unsigned_bitref_r*, and *sc_concatref*. |
| `operator sc_unsigned()` | *sc_signed_subref_r*, *sc_unsigned_subref_r*, and *sc_concatref* |
| `operator sc_logic()`    | *sc_bitref_r*. |
| `operator double()`      | *sc_fxnum* and *sc_fxval*. |
| `operator bool()`        | *sc_fxnum_bitref*. |
| `operator sc_bv_base()`  | *sc_fxnum_subref*. |

***

#### 8.3.5.14 Assignment operators and constructors

Assignment operators and constructors have ***Restricted Support*** as described below.

Initialization through String Literals, as specified in SystemC LRM 7.3, has
***Restricted Support***.
Initialization of numeric or vector type object with a string literal as the argument of a
constructor or assignment operator is ***Supported***, but an initialization through a variable of
*char* or *std::string* type is ***Not Supported***.
The *sc_numrep* of *SC_NOBASE* (implementation-defined prefix or missing prefix) is
***Not Supported***.

The SystemC LRM introduces the concept of case insensitivity to the prefix and magnitude
representations. These are ***Supported***.

***
**Note:**

In accordance with the SystemC definition of string literals, SystemC LRM 7.3, a literal
representation may be used as the value of a SystemC numeric or vector type object.

It consists of a standard case-insensitive prefix, followed by a magnitude expressed as one or more
digits.


| sc_numrep   | Prefix                 | Magnitude format                         |
| :-          | :-                     | :-                                       |
| *SC_NOBASE* | Implementation-defined | Implementation-defined                   |
| *SC_DEC*    | 0d                     | Decimal Number, digits 0-9               |
| *SC_BIN*    | 0b                     | Binary Number, digits 0-1                |
| *SC_BIN_US* | 0bus                   | Binary Unsigned                          |
| *SC_BIN_SM* | 0bsm                   | Binary Sign & Magnitude                  |
| *SC_OCT*    | 0o                     | Octal Number, digits 0-7                 |
| *SC_OCT_US* | 0ous                   | Octal Unsigned                           |
| *SC_OCT_SM* | 0osm                   | Octal Signed & Magnitude             |
| *SC_HEX*    | 0x                     | Hexadecimal Number, digits 0-9, a-f, A-F |
| *SC_HEX_US* | 0xus                   | Hexadecimal Unsigned                     |
| *SC_HEX_SM* | 0xsm                   | Hexadecimal Sign & Magnitude             |
| *SC_CSD*    | 0csd                   | Canonical Signed Digit                   |

***


#### 8.3.5.15 String input and output

- ***Not Supported*** for *dump( )*, *print( )*, and *scan( )*.
See SystemC LRM 7.2.11.

- ***Not Supported*** for *to_string( )*. See SystemC LRM 7.3.

- ***Not Supported*** for *String Shortcut Methods* (*to_dec( )*, *to_bin()*, and so on).
See SystemC LRM 7.10.9.2.

#### 8.3.5.16 Length

The member function *length* is ***Supported***.

#### 8.3.5.17 Reversed

The member function *reversed* is ***Not Supported***.

***
**Note:**

This is consistent with the restriction on bit-reversal in
Section [8.3.5.2](#8352-part-select-operator).

***
