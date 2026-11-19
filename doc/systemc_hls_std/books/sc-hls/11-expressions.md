---
style: chapter
---

# 11. Expressions

An expression is a sequence of operators and operands that can result in a value.
Expressions can cause side effects (ISOC++ Section 8).
Except where ISOC++ specifies otherwise, the evaluations of operands and the side effects are
unsequenced, and some such expressions have undefined behavior (ISOC++ 4.6). The relative order of
the evaluation of function arguments is indeterminately sequenced, that is, unspecified
(ISOC++ 8.2.2).
For example the statement:

```cpp
    f(i++, i++);
```

has a behavior that is not specified in ISOC++. Expressions that are legal in ISOC++, but whose
behavior is unspecified (not undefined) due to order of evaluation or order of side effects, are supported for
synthesis. *<u>Synthesis tools</u>* are permitted to interpret such expressions in any way that is
compliant with the ISOC++ standard.

***
**Note:**

User code *<u>should not</u>* use such expressions to avoid differences in the results between simulation using a particular compiler and synthesis using a particular *<u>synthesis tool</u>*.

***

## 11.1 Primary expressions

### 11.1.1 General

General expressions (ISOC++ 8.1) are ***Supported***.

### 11.1.2 Lambda expressions

Lambda expressions (ISOC++ 8.1.5) are ***Supported***.

*constexpr* lambda expressions are ***Supported***.
A lambda expression can be declared or implicitly deduced as *constexpr*, allowing its use in constant expressions for compile-time computation.

Capture of `*this` by value (ISOC++ 8.1.5.2) is ***Supported***.
This allows a lambda to capture a copy of the enclosing object, ensuring the lambda does not depend on the lifetime of the original object.

### 11.1.3 Fold expressions

Fold expressions (ISOC++ 8.1.6) are ***Supported***.

## 11.2 Function call

Non-recursive function calls (ISOC++ 8.2.2) are ***Supported***.

Recursive functions — that is, functions that invoke themselves either directly or indirectly — are ***Supported with Restrictions***.
Recursion is permitted only when the maximum recursion depth can be determined statically.
The maximum allowable recursion depth is implementation-defined.

## 11.3 Explicit type conversion

An explicit type conversion (ISOC++ 8.4) can be expressed using a type conversion operator, a
functional notation (ISOC++ 8.2.3), or cast notation. An explicit type conversion of non-pointer type is
***Supported***, except as stated in Sections
[11.3.1.1](11-expressions.md#11311-dynamic-cast) and
[11.3.1.3](11-expressions.md#11313-reinterpret-cast).
On the other hand, an explicit type conversion of pointer is ***Supported with Restrictions***.

### 11.3.1 Type conversion operators

#### 11.3.1.1 Dynamic cast

The type conversion operator function *dynamic_cast\<T\>(v)* (ISOC++ 8.2.7) is ***Not Supported***.

#### 11.3.1.2 Static cast

The type conversion operator function *static_cast\<T\>(v)* (ISOC++ 8.2.9) is ***Supported***.
Conversions of pointer type are subject to the restrictions on pointers in Section
[10.3.1](10-declarators.md#1031-pointers).

#### 11.3.1.3 Reinterpret cast

The type conversion operator function *reinterpret_cast\<T\>(v)* (ISOC++ 8.2.10) is
***Not Supported***.

#### 11.3.1.4 Const cast

The type conversion operator function *const_cast\<T\>(v)* (ISOC++ 8.2.11) is ***Supported***.
Conversions of pointer type are subject to the restrictions on pointers in Section
[10.3.1](10-declarators.md#1031-pointers).

### 11.3.2 Functional notation

An explicit type conversion of pointer type using functional notation (ISOC++ 8.2.3) is
***Supported with Restrictions***. Each type conversion of pointer type using functional notation
can be mapped to any of type conversion operators described in
Section [11.3](#113-explicit-type-conversion).
The restriction is the same as those of the type conversion operators.

### 11.3.3 Cast notation

An explicit type conversion of pointer type using cast notation (ISOC++ 8.4) is ***Supported with Restrictions***.
Each type conversion of pointer type using cast notation can be mapped to any of type conversion
operators described in Section [11.3](#113-explicit-type-conversion).
The restriction is the same as those of the type conversion operators, which are equivalent to the
given explicit type conversion using cast notation.

## 11.4 typeid

The type identification function *typeid* (ISOC++ 8.2.8) is ***Not Supported***.

## 11.5 Unary expressions and operators

### 11.5.1 Unary operators

The unary operators (ISOC++ 8.3.1) of the form \* & + - ! and ~ are ***Supported***. For pointers see Section
[10.3.1](10-declarators.md#1031-pointers).

### 11.5.2 Increment and decrement

***Supported*** (ISOC++ 8.2.6 and 8.3.2). See Section [8.1.3](08-types.md#813-operators) for the
restrictions on integer types.

### 11.5.3 sizeof

The *sizeof* operator (ISOC++ 8.3.3) is ***Not Supported***, except for `sizeof...`, which is
***Supported***.

### 11.5.4 new and delete

The *new* operator (ISOC++ 8.3.4) has ***Restricted Support***. It is ***Supported*** for the
instantiation of variable arrays and *SC_MODULE*. It is ***Not Supported*** for the instantiation of
types derived from the *sc_object* type, other than *SC_MODULE*.  The *new* operator *<u>may</u>*
appear in the constructor or constructor initializer list within *SC_MODULE* only and that use of
*new* is ***Not Supported*** anywhere within the behavioral description, such as inside *SC_METHOD*,
*SC_THREAD*, or *SC_CTHREAD*.  When allocating an array of objects using *new*, the number of
elements *<u>shall</u>* be statically determinable.

The *delete* operator (ISOC++ 8.3.5) is ***Not Supported***.

The use of *set_new_handler()* *<u>shall</u>* be ***Ignored***.

Overloading the *new* or *delete* operators is ***Not Supported***. The *placement* *new* construct
is ***Not Supported***.

Example:
```cpp
SC_MODULE(MyModule) {
    sc_in<bool>     SC_NAMED(CLK);
    sc_in<bool>     SC_NAMED(RST);
    sc_in<int>      SC_NAMED(a);
    sc_in<int>      SC_NAMED(b);
    sc_out<int>     SC_NAMED(c);
    sc_out<bool>    SC_NAMED(RDY);
    sc_signal<int>  SC_NAMED(tmp);

    GCD *gcd;

    unsigned int *mem ;

    SC_CTOR(MyModule): mem(new unsigned[128])
    {
        gcd = new GCD("GCD");
        gcd->CLK(CLK);
        gcd->RST(RST);
        gcd->x(tmp);
        gcd->y(b);
        gcd->z(c);
        gcd->RDY(RDY);
    }

};
```

### 11.5.5 alignof

The *alignof* expression (ISOC++ 8.3.6) is ***Ignored***.

### 11.5.6 noexcept operator

The *noexcept* operator (ISOC++ 8.3.7) is ***Ignored***.

## 11.6 Pointer-to-member operators

The pointer-to-member operators -\>\* and .\* have ***Restricted Support*** (ISOC++ 8.5).

The restrictions are based on the ***Restricted Support*** of pointers as defined in Section
[10.3.1](10-declarators.md#1031-pointers).

## 11.7 Multiplicative, additive, shift, relational and equality operators

***Supported*** (ISOC++ 8.6, 8.7, 8.8, 8.9, and 8.10). See Section
[8.1.3](08-types.md#813-operators) for the restrictions on integer types.

## 11.8 Bitwise AND/XOR/OR and logical AND/OR operators

***Supported*** (ISOC++ 8.11 to 8.15).

## 11.9 Conditional operator

***Supported*** (ISOC++ 8.16).

## 11.10 Throwing an exception

***Not Supported*** (ISOC++ 8.17).

## 11.11 Assignment and compound assignment operators

***Supported*** (ISOC++ 8.18).

## 11.12 Comma operator

***Supported*** (ISOC++ 8.19).

## 11.13 Constant expressions

***Supported*** (ISOC++ 8.20).
