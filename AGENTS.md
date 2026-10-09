# Release versioning

- Use `master` as this fork's development and release trunk. Preserve `main` as the upstream reference.
- Use `.github/workflows/fork-ci.yml` for fork CI and releases; keep the legacy `build.yml` workflow disabled in GitHub.
- 版號與發行名稱只使用完整語意版本，不加入中文或其他說明字尾；測試階段由 beta／rc 標示。GitHub 使用一般發行與最新版本標記，讓首頁右側顯示版號。

- Follow `docs/version-metadata.md` and use `Version.cmake` as the sole version source.
- Treat upstream `7b09eef` as the historical `1.0.0-beta.1` baseline.
- Before publishing changed binaries, increment the semantic release version and Windows revision; never reuse a published version for changed contents.
- Increment Windows revision for every release within the same major/minor, including beta, rc, stable, and patch releases.
- Keep full semantic versions in binary metadata, MSI product names, installer filenames, and Actions artifact names.
- Verify generated metadata, actual binary resources, tests, and MSI properties before reporting a versioned installer as ready.
- Preserve the installer UpgradeCode and component GUIDs.
