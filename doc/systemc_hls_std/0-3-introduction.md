---
style: frontmatter
---

# Introduction

The growing popularity of SystemC for system and hardware design has spurred significant growth in the high-level synthesis (HLS) industry in the recent past.

This standard is intended for use by hardware designers and HLS tool developers in a manner that allows hardware designers to create HLS designs in SystemC that will be portable among all conforming HLS tools.

While there are multiple commercially available HLS tools that accept SystemC as an input language, the inherent difference between the description of a simulation model and that of a synthesis model presents the question of which constructs and semantics of SystemC should be supported by these tools and how they should be interpreted when generating the register transfer-level (RTL) design.

## Scope

This standard addresses this issue by defining :

1. The SystemC syntax subset for HLS tools
    - i.e. the minimum subset to be supported by HLS tools

1. Standard input code structures and associated directive to generate specific RTL structures
    - e.g. the necessary input code style and directives necessary to generate a pipeline

1. Expected output structures in the generated RTL code
    - e.g. expected port names.

This standard is defined within the existing ISO C++17 and IEEE 1666 SystemC specifications.
Hence, familiarity with these standards is presumed.

It should be noted that the intent of this version of the standard is to define a subset of SystemC for synthesis, it is not meant to restrict HLS tools’ support for syntax beyond this subset, including additional vendor-specific design/library components.

Familiarity with hardware design with high-level synthesis may also be helpful in understanding this standard. A separate discussion on this topic is added as an Annex of this standard.

## Acknowledgments

This standard was defined by the Synthesis Working Group of the Accellera Systems Initiative with participation from various HLS tool vendors and users. Below is the list of individuals who contributed to this standard.

TBD