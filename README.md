# systemc-hls

Draft **SystemC High-level Synthesis Standard** and its reference library,
**systemc-hls**, developed by the [Accellera][1]
[SystemC Synthesis Working Group][2] (SWG).

> **Public review.** The draft standard and the systemc-hls 0.1.0 library are
> open for public review from November 19, 2026 to February 17, 2027. Your
> feedback directly shapes the final standard: please read, try, and tell us
> what you think — see [Get involved](#get-involved).

## Download

Release [v0.1.0-pub_rev][5]:

| File | Content |
|------|---------|
| [`systemc-hls-lib-0.1.0-pub_rev.tar.gz`][7] | systemc-hls reference library, with examples and tests |
| [`systemc-hls-std-2_0-public-review_1.pdf`][8] | The draft SystemC High-level Synthesis Standard (PDF) |

All releases: [Releases][9].

## What is here

| Path | Content |
|------|---------|
| [`systemc-hls-std-2_0-public-review_1.pdf`][4] | The draft SystemC High-level Synthesis Standard (PDF) |
| [`doc/systemc_hls_std/`](doc/systemc_hls_std/) | Markdown source of the draft standard, one file per clause |
| [`code/systemc-hls/`](code/systemc-hls/) | systemc-hls reference library: header-only, with examples and tests ([README](code/systemc-hls/README.md), [release notes](code/systemc-hls/RELEASE_NOTES.md)) |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | How to report issues and contribute |
| [`LICENSE`](LICENSE) | Apache License, Version 2.0 |

## Quick start

The library needs SystemC 2.3.4 or later and a C++11 compiler. Include
`<systemc-hls>` and see [`code/systemc-hls/README.md`](code/systemc-hls/README.md)
for the protocols, the examples and how to run the tests.

## Get involved

The standard is only as good as the review it gets. Every contribution helps,
large or small:

- **Review the draft.** Read the [PDF][4] and report anything unclear, incorrect,
  missing or hard to implement on the [public review forum][10] or as an
  [issue][3]. Please include the section number.
- **Try the library.** Build the examples against your own designs and flows,
  and report bugs or gaps as [issues][3] (SystemC version, compiler, and a
  short code sample help a lot).
- **Propose changes.** Pull requests are welcome, both on the library
  (`code/systemc-hls/`) and on the standard text (`doc/systemc_hls_std/`).
  Contributions must be signed off under the Accellera Developer's
  Certificate of Origin (`git commit --signoff`); see
  [CONTRIBUTING.md](CONTRIBUTING.md).
- **Join the working group.** Accellera members can join the SWG and take
  part in shaping the standard; see [CONTRIBUTING.md](CONTRIBUTING.md#join-the-accellera-systemc-synthesis-working-group).

## Acknowledgement

Many of the key sc_hls::msg_lib library features such as throughput-accurate modeling, latency and capacity back annotation, random stall injection, and
transaction recording and tracing are based on the Matchlib library developed by NVIDIA Research. The original DAC 2018 paper from NVIDIA Research
that describes this work is freely available for download via the following link:

https://research.nvidia.com/publication/2018-06_modular-digital-vlsi-flow-high-productivity-soc-design

## License

This repository is owned by the Accellera Systems Initiative and licensed
under the [Apache License, Version 2.0](LICENSE).

[1]: https://www.accellera.org
[2]: https://accellera.org/activities/working-groups/systemc-synthesis
[3]: https://github.com/accellera-official/systemc-hls/issues
[4]: systemc-hls-std-2_0-public-review_1.pdf
[5]: https://github.com/accellera-official/systemc-hls/releases/tag/v0.1.0-pub_rev
[7]: https://github.com/accellera-official/systemc-hls/releases/download/v0.1.0-pub_rev/systemc-hls-lib-0.1.0-pub_rev.tar.gz
[8]: https://github.com/accellera-official/systemc-hls/releases/download/v0.1.0-pub_rev/systemc-hls-std-2_0-public-review_1.pdf
[9]: https://github.com/accellera-official/systemc-hls/releases
[10]: https://forums.accellera.org/forum/65-systemc-high-level-synthesis-hls-20-draft-public-review/
