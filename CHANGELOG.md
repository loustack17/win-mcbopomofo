# Changelog

## 1.0.0-beta.2

- Commit punctuation immediately at the end of the normal composing buffer.
- Pass unchanged half-width ASCII punctuation through from an empty buffer.
- Route standalone Shift consistently through TSF test and actual callbacks.
- Pass Space to the host when no composition or candidates are active.
- Add opt-in local TSF diagnostics for host compatibility investigation.
- Generate binary, installer, and artifact version metadata from one source.

LINE and WezTerm compatibility requires installed-build testing. Host-specific
punctuation failures are still under investigation.

## 1.0.0-beta.1 (upstream baseline)

Upstream baseline `7b09eef`, designated as the first beta for this fork's version
history. No upstream release or tag is modified.
