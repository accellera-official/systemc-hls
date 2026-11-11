---
style: chapter
---

# 2. Normative references

## 2.1 Standard Baseline Versions

### Reference Documents

This standard shall be used in conjunction with the following
publication:

- ISO/IEC 14882:2017 C++ Standard (C++17) \[1\]
    * available at https://webstore.ansi.org/standards/iso/isoiec148822017

- IEEE Standard 1666-2023 for Standard SystemC Language, Reference Manual \[2\]
    * available at https://ieeexplore.ieee.org/servlet/opac?punumber=10246123

The *ISO/IEC 14882:2017 C++ Standard* is hereinafter called *ISOC++* in this document.

The *IEEE Standard 1666-2023 for Standard SystemC* is hereinafter called
*SystemC LRM* in this document.

The *IEEE Standard 1666-2011 for Standard SystemC, Language Reference Manual* is
available from https://ieeexplore.ieee.org/servlet/opac?punumber=6581814

### Other Materials

The website for the C++ Standard is https://isocpp.org/std/the-standard

C++ standard workgroup webpages documentating changes:
- from C++11 to C++14: https://isocpp.org/files/papers/p1319r0.html
- from C++14 to C++17: https://isocpp.org/files/papers/p0636r0.html

The *IEEE Standard 1666-2023/Cor1-2025 for Standard SystemC Language, Reference Manual - Corrigendum 1*
is available from https://ieeexplore.ieee.org/servlet/opac?punumber=10932725

Informal useful references from the CPP Reference website:
- C++14 https://en.cppreference.com/w/cpp/14
- C++17 https://en.cppreference.com/w/cpp/17


## 2.2 ISOC++ Implementation Compliance

The ISOC++ Section 4.1 Implementation Compliance applies to the synthesizable subset defined in this document.

### 2.2.1 Implementation-defined behavior

ISOC++ 3.12

Simulation of the *<u>design</u>* is based on C++ compilers on specific computer platforms.
In most cases, such platforms have converged on certain default implementation-defined behaviors
and *<u>synthesis tools</u>* are required to either adhere to those behaviors or provide warnings
to the effect that a different implementation-defined behavior is being followed.

An example of an implementation-defined behavior is the bit-width of fundamental integer types.

### 2.2.2 Undefined behavior

ISOC++ 3.27

The presence of undefined behavior *<u>may</u>* lead to differences between simulation and synthesis
and differences in results from different *<u>synthesis tools</u>*.
Unless this standard provides a *<u>synthesis refinement</u>* that provides a definition, such
undefined behavior is ***Not Supported***

In some cases, undefined behavior is assumed to be not present in the *<u>design</u>*.
One specific example is division by zero (0).
*<u>Synthesis tools</u>* *<u>may</u>* assume that such a condition will not be present during
simulation of the *<u>design</u>* and treat such a condition as a don’t care.

### 2.2.3 Unspecified behavior

ISOC++ 3.28

The presence of unspecified behavior *<u>may</u>* lead to differences between simulation and
synthesis results and differences in results from different *<u>synthesis tools</u>*.

An example is the order of evaluation of arguments to a function is not specified.
In ISOC++, the implementation is not required to document which behavior occurs.

## 2.3 SystemC LRM Compliance

This section describes some general terms that are used in the SystemC LRM that have similarities
with terms used in ISOC++.

### 2.3.1 Implementation-defined

SystemC LRM 3.2.1

In general, most of the implementation-defined items in the SystemC LRM do not affect synthesis.
One item that does affect synthesis is the type for limited precision (SystemC LRM 7).
In that case, this standard only supports one implementation as specified in Section [8.3](08-types.md#83-systemc-datatypes) on SystemC datatypes.

### 2.3.2 Undefined

The description in Section [2.2.2](#222-undefined-behavior) applies when the word *<u>undefined</u>*
is used in the context of the behavior not being defined.

### 2.3.3 Error

The word *<u>error</u>* is used in the SystemC LRM (3.3.6) to mean that the implementation is
required to report SC_ERROR through the report handler, normally during elaboration or simulation.
This differs from *<u>undefined</u>*, where an implementation may or may not issue an error or a
warning.
*<u>Synthesis tools</u>* assume that such conditions will not be present and treat such condition as
a don’t care, unless the *<u>synthesis tools</u>* are able to prove that the error is present, in
which case they *may* issue an error appropriately.
