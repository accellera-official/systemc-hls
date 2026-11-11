---
style: frontmatter
---

# Introduction

*High-Level Synthesis* (HLS), also known as *Behavioral Synthesis*, allows designing at a level of abstraction higher than the register-transfer level (RTL) by automating the translation and optimization of a behavioral description, or a high-level model, into an RTL implementation. It transforms un-timed or partially timed functional models into fully timed RTL implementations.

Since the micro-architecture is generated automatically, designers can focus on designing and verifying the module functionality. Design teams create and verify their designs in less time because it eliminates the need to fully schedule and allocate design resources, as done with traditional RTL methods. This behavioral design flow increases design productivity, reduces errors, and speeds up verification.

A typical high-level synthesis process incorporates a number of complex stages. This process starts with a high-level language description of a module's behavior, including I/O actions and computational functionality. Several algorithmic optimizations are performed to reduce the complexity of a result and then the description is analyzed to determine the essential operations and the dataflow dependencies between them.

The other inputs to the high-level synthesis process typically include a target technology library and a set of directives that will influence the resulting architecture. These directives include, for example, timing constraints used by the algorithms of the synthesis tool as they create a cycle-by-cycle schedule of the required operations. In some flows, a set of low-level components is characterized at a target frequency for the selected fabrication process. This characterized library is used when allocation and binding occurs, in order to assign these operations to specific functional units, such as adders, multipliers, comparators, etc.

Finally, a state machine is generated that will control the resulting datapath to implement the desired functionality. The datapath and state machine outputs are RTL code optimized for use with conventional simulation and logic synthesis or physical synthesis tools. Details of the different stages of high-level synthesis are out of the scope of this document.

The growing popularity of SystemC for system and hardware design has spurred significant growth in the HLS industry in the recent past.

While there are multiple commercially available HLS tools that accept SystemC as an input language, the inherent difference between the description of a simulation model and that of a synthesis model presents the question of which constructs and semantics of SystemC should be supported by these tools and how they should be interpreted when generating the RTL design.

This standard is intended for use by hardware designers and HLS tool developers in a manner that allows creating HLS designs in SystemC that will be portable among all conforming HLS tools.
