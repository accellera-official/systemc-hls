---
style: chapter no-header
---

{% include metadata %}

# 1. Overview

## 1.1 Scope

This standard defines the following aspects of high-level synthesis of SystemC:

1. The minimum ISO C++17 \[1\] and IEEE Std 1666 \[2\] SystemC syntax subset to be supported by HLS tools

2. Standard input code structures to generate specific RTL structures

3. Expected output structures in the generated RTL code

4. A library of communication and synchronization components for HLS-based designs

The intent of this version of the standard is to describe a minimum initial subset which can be supported by tools. It is not meant to restrict synthesis support for syntax beyond this subset, including additional vendor-specific design/library components.

## 1.2 Purpose

In this standard document, the Synthesis Working Group (SWG) of the Accellera Systems Initiative has
defined a subset of SystemC that is appropriate for synthesis. This is intended to be useful for
hardware designers to accelerate the modeling process with SystemC and for EDA tool developers to
develop SystemC compliant high-level synthesis (HLS) tools. The synthesizable subset of SystemC will
be defined within the existing ISO C++ and IEEE Std 1666 SystemC specifications.
Hence, familiarity with these standards is presumed.

There are a wide variety of resources available to assist users in modeling with SystemC, including
the IEEE Std 1666 Language Reference Manual (LRM), user guides of the HLS tools, and a number of
books on SystemC modeling. This document is intended to fill a gap by defining a standard for
creating synthesizable hardware description in SystemC, allowing a smooth transition between
abstract modeling in SystemC and synthesizable description. Familiarity with hardware design and HLS may also be helpful in understanding this standard.

## 1.3 Word usage

The word *<u>shall</u>* indicates mandatory requirements strictly to be followed to conform to the
standard and from which no deviation is permitted (*<u>shall</u>* equals *<u>is required to</u>*;
*<u>shall not</u>* equals *<u>is not permitted to</u>*).

The word *<u>should</u>* indicates a certain course of action is preferred, but is not a mandatory
requirement; or (in the negative form, *<u>should not</u>*) that a certain course of action is
permitted, but such usage is discouraged (*<u>should</u>* equals *<u>is recommended that</u>*).

The word *<u>may</u>* indicates a course of action permissible within the limits of the standard
(*<u>may</u>* equals *<u>is permitted</u>*).

The word *<u>application</u>* is a C++ program written by an end user that uses the SystemC, TLM-1,
and TLM-2.0 class libraries, i.e., it uses classes, functions, or macros defined in the SystemC LRM.

The word *<u>implementation</u>* means any specific implementation of the full SystemC, TLM-1,
and TLM-2.0 class libraries, as defined in the SystemC LRM, of which only the public interface need
be exposed to the application.

The word *<u>design</u>* is used to mean a SystemC design written by an end user that describes
hardware in conformance with this standard.

A synthesis tool is said to *<u>accept</u>* a SystemC construct if it allows that construct to
be a legal input; it is said to *<u>interpret</u>* the construct (or to provide an
*<u>interpretation</u>* of the construct) if it accepts that construct and produces a
corresponding synthesis result.

The word *<u>synthesis tool</u>* is used to mean a high-level synthesis tool that *<u>accepts</u>*
and *<u>interprets</u>* a *<u>design</u>* in conformance with this standard.

The term *<u>call</u>* is taken to mean call directly or indirectly. Call indirectly means call an
intermediate function that in turn calls the function in question, where the chain of function calls
*<u>may</u>* be extended indefinitely. Similarly, *<u>called from</u>* means called from directly or
indirectly.

The term *<u>class</u>* is used to cover the C++ keywords *class* or *struct*.

Except where explicitly qualified, the term *<u>derived from</u>* (or *<u>inherited from</u>*) is
taken to mean derived directly or indirectly from a *<u>class</u>*. Derived indirectly from means
derived from one or more intermediate base *<u>classes</u>*.

A *<u>synthesis refinement</u>* in this document imposes a restriction or alteration upon some other
standard (e.g., the SystemC LRM or the ISOC++ standard) in order to subset the otherwise supported
methods to describe behavior to a set that can be implemented in hardware.

The word *<u>deprecated</u>* is used to describe a feature that is superseded by a better, safer,
easier to use alternative. Use of the *<u>deprecated</u>* feature is strongly discouraged and future
standards *<u>may</u>* make such use illegal.

### 1.3.1 Construct categories

The constructs in this standard *<u>shall</u>* be categorized as:

***Supported***
: A *<u>synthesis tool</u>* *<u>shall</u>* interpret the construct.

***Ignored***
: A *<u>synthesis tool</u>* *<u>shall</u>* accept the construct, but *<u>may</u>* choose not to
interpret the construct. The mechanism, if any, by which a *<u>synthesis tool</u>* notifies (warns)
the user of such constructs is not defined in this standard.

***Not Supported***
: A *<u>synthesis tool</u>* *<u>may</u>* choose not to accept the construct. The behavior of the
*<u>synthesis tool</u>* upon encountering such a construct is not defined in this standard.
For example, a *<u>synthesis tool</u>* *<u>may</u>* choose to fail upon encountering such a
construct; alternatively, it *<u>may</u>* choose to accept/interpret such a construct.
Even if a *<u>synthesis tool</u>* accepts/interprets some of the constructs that are
***Not Supported***, a *<u>design</u>* that uses such constructs runs the risk of losing
portability.
However, it is also possible that, in a future revision of this standard, some of the constructs
that are ***Not Supported*** now will be ***Supported***.

***Supported with Restrictions***
: A *<u>synthesis tool</u>* *<u>shall</u>* interpret the construct with certain restrictions.
This means, instances of the construct which are within the restrictions, as set forth by this
standard, are ***Supported***;
while instances that violate these restrictions are either ***Ignored*** or ***Not Supported***.
Unless explicitly categorized as ***Ignored***, the violating instances are ***Not Supported***.
A construct that is ***Supported with Restrictions*** is also said to have ***Restricted Support***.

## 1.4 Conventions

This document uses the following conventions:

1)  The body of the text of this standard uses *italics* to denote SystemC or C++ reserved words
    (e.g., *sensitive* and *SC_MODULE*).

2)  The body of the text of this standard also uses *<u>italics underlined</u>* to highlight
    definitions (e.g., *<u>application</u>* or *<u>design</u>*) or to visually reinforce key terms
    (e.g., *<u>shall</u>*, *<u>should not</u>*, and *<u>synthesis refinement</u>*).

3)  The body of the text of this standard uses ***bold italics*** to visually reinforce construct
    categories (e.g., ***Supported*** and ***Ignored***).

4)  The text of the SystemC examples and code fragments is represented in a fixed-width font.

5)  A note provides an informative expansion of certain key concepts. Notes are intended to assist in
    understanding of a construct, but are not intended as a restriction or enhancement of a
    *<u>synthesis tool</u>*.

6)  The examples that appear in this document under "*Example:*" are for the sole purpose of
    demonstrating the syntax and semantics of SystemC for synthesis. It is not the intent of this
    standard to demonstrate, recommend, or emphasize coding styles that are more (or less) efficient
    in generating an equivalent hardware representation. In addition, it is not the intent of this
    standard to present examples that represent a compliance test suite or a performance benchmark,
    even though these examples are compliant to this standard (except as noted otherwise).
