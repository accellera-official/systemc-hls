# systemc-hls

Draft **SystemC High-level Synthesis Standard** and its reference library,
**systemc-hls**, developed by the [Accellera][1]
[SystemC Synthesis Working Group][2] (SWG).

> **Public review.** The draft standard and the systemc-hls 0.1.0 library are
> open for public review (TODO: review period dates). Your feedback directly
> shapes the final standard: please read, try, and tell us what you think —
> see [Get involved](#get-involved).

## What is here

| Path | Content |
|------|---------|
| [`SystemC_HLS_Standard_draft_1_6_pub_rev_2026-11-03.pdf`][4] | The draft SystemC High-level Synthesis Standard (PDF) |
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
  missing or hard to implement as an [issue][3]. Please include the section
  number.
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

## License

This repository is owned by the Accellera Systems Initiative and licensed
under the [Apache License, Version 2.0](LICENSE).

[1]: https://www.accellera.org
[2]: https://accellera.org/activities/working-groups/systemc-synthesis
[3]: https://github.com/accellera-official/systemc-hls/issues
[4]: SystemC_HLS_Standard_draft_1_6_pub_rev_2026-11-03.pdf
