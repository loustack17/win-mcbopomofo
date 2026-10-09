# Development language

- Use English for new general-purpose identifiers, code comments, configuration labels, and development tooling.
- Keep developer instruction files, including AGENTS.md, entirely in English; do not mix languages within them.
- Documentation may use Traditional Chinese or English, including the root README.
- Preserve language-specific content required by the input method: Bopomofo symbols and identifiers, Chinese UI strings, dictionaries, linguistic mappings, and corresponding test data.
- Do not enforce blanket ASCII conversion or escape existing text solely to remove non-English characters. Preserve third-party content and attribution.
- Avoid invisible or bidirectional control characters in maintained source and filenames; preserve punctuation required by syntax or input-method behavior.

# Architecture direction

- Follow `docs/maintenance-direction.md` when planning cleanup or refactoring.
- Keep C++ as the primary implementation language. Prefer one maintained implementation language, or at most two if a second language replaces existing complexity.
- Consider Rust or Zig only for a concrete replacement with clear maintenance benefits; do not introduce both alongside C++.
- Gradually reduce maintained C, VBScript, and PowerShell code by consolidating responsibilities and removing verified dead code. Preserve required Windows build and installer integration until a validated replacement exists.
- Do not rewrite third-party dependencies or alter language statistics just to change GitHub's language chart.
- Migrate incrementally, preserve input behavior and installation compatibility, and verify each replacement before removing the old path.

# Release versioning

- Use `master` as this fork's development and release trunk. Preserve `main` as the upstream reference.
- Use `.github/workflows/fork-ci.yml` for fork CI and releases; keep the legacy `build.yml` workflow disabled in GitHub.
- Use the full semantic version alone for release titles, without localized suffixes. Indicate testing stages with beta or rc. Publish GitHub releases as latest, non-prerelease entries so the repository sidebar displays the version.

- Follow `docs/version-metadata.md` and use `Version.cmake` as the sole version source.
- Treat upstream `7b09eef` as the historical `1.0.0-beta.1` baseline.
- Before publishing changed binaries, increment the semantic release version and Windows revision; never reuse a published version for changed contents.
- Increment Windows revision for every release within the same major/minor, including beta, rc, stable, and patch releases.
- Keep full semantic versions in binary metadata, MSI product names, installer filenames, and Actions artifact names.
- Verify generated metadata, actual binary resources, tests, and MSI properties before reporting a versioned installer as ready.
- Preserve the installer UpgradeCode and component GUIDs.
