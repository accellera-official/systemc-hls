---
style: chapter
---

# 4. Translation units

## 4.1 Translation units and their analysis

The text of a *<u>design</u>* is, as described by ISOC++ Sections 5.1 and 5.2, kept in units called
source files.  A source file together with all included sources, less any lines skipped by
conditional inclusion directives (ISOC++ 19.1), is known as a translation unit.

A translation unit *<u>shall</u>* be specified using the basic source character set, as described
by ISOC++ Section 5.3.  Alternative tokens (ISOC++ Section 5.5) are ***Not Supported***.
Furthermore, instance names using Universal-Character-Name as described in ISOC++ Sections 5.3
and 5.10 are *<u>synthesis tool</u>*-dependent and, therefore, ***Not Supported***.

Using multiple translation units to describe a *<u>design</u>* is ***Supported***. This standard,
however, does not specify how a user will provide the translation-unit information to the
*<u>synthesis tool</u>*.

The use of the keyword *extern* to refer to a declaration in a different scope within the same
translation unit or in a different translation unit is ***Supported***. However, the use of
libraries (pre-compiled binary files) is ***Not Supported***, and a *<u>design</u>* *<u>shall</u>*
contain all the source files required for synthesis
(with the exception of SystemC implementation and C/C++ native library files).

The use of *extern* string-literals for linkage specification (e.g., extern "C", extern "C++") is
***Not Supported*** (ISOC++ Section 10.5).

The functions *main* and *sc_main* are ***Ignored***.

## 4.2 Pre-processing directives

The full set of C/C++ preprocessing directives is ***Supported***
(refer to ISOC++ Clause 19).

The `__has_include` preprocessor expression (ISOC++ 19.1) is ***Supported***.
This allows conditionally including headers based on their availability.

A *<u>synthesis tool</u>* *<u>shall</u>* recognize pragma directives (#pragma).
It *<u>may</u>* ignore or process pragma directives.

A *<u>synthesis tool</u>* *<u>shall</u>* predefine the following macro names:

| Macro name        | Description |
|-------------------|-------------|
| \_\_STDC\_\_      | The value is implementation-dependent. |
| \_\_cplusplus     | The value is 201703L (ISOC++ 19.8). |
| SC_SYNTHESIS      | The version of the synthesis subset, in a year and month format such as 202611L. |
| \_\_SYNTHESIS\_\_ |  The value is implementation-dependent. |
| SC_HLS_DEFAULT_SYNTH_VIEW_TAG | The synthesis view tag (see [18.2](18-systemc-hls-library.md#182-simulation-and-synthesis-views)). |

***
**Note:**

The expected use of the \_\_SYNTHESIS\_\_ and SC_SYNTHESIS macros is to guard non-synthesizable code.

The version contained in SC_SYNTHESIS is meant to select different versions of the same behavior
that depend on updates to the synthesis subset.

***
