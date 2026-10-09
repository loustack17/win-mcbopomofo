# Release versions

The canonical source is `Version.cmake`. All executable and DLL resources,
MSI metadata, default installer names, and GitHub Actions artifact names derive
from it. Generated files must not be edited manually.

## Semantic version policy

Follow [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
For this fork, the upstream baseline at `7b09eef` is treated as
`1.0.0-beta.1`; this is a historical baseline designation, not a new upstream tag.
The first explicitly versioned fork release is `1.0.0-beta.2`, containing the
punctuation, Space, and Shift fixes. Earlier unversioned artifacts remain unchanged.

Increment beta numbers for subsequent test releases of the same target version:
`1.0.0-beta.2`, `1.0.0-beta.3`, and so on. Use `1.0.0-rc.1` when ready for final
release validation, then `1.0.0` after acceptance. Alpha, beta, and rc identifiers
use positive numeric ordinals without leading zeros.

After a stable release, increment PATCH for compatible fixes, MINOR for compatible
features, and MAJOR for incompatible changes to supported behavior or configuration.
Reset lower components when incrementing MINOR or MAJOR. Release version strings
are immutable: changed published binaries require a new release version.

## Windows upgrade version

[Windows Installer](https://learn.microsoft.com/en-us/windows/win32/msi/productversion)
accepts numeric versions and compares only the first three components. Therefore
its version is `MAJOR.MINOR.WINDOWS_REVISION`, where WINDOWS_REVISION is a separate
release serial, not the SemVer patch. Increment it for every published release
within the same MAJOR.MINOR, including beta, rc, stable, and patch releases.
Never reset it when moving from beta to rc or stable, or when changing PATCH.
It may restart at 1 only when MAJOR or MINOR increases. MAJOR and MINOR must fit
0..255 and WINDOWS_REVISION must fit 1..65535; generation rejects invalid values.

Current mapping: `1.0.0-beta.2` -> MSI `1.0.2` -> numeric PE resources `1.0.2.0`.
This upgrades the legacy `1.0.0.0` MSI, which compares as `1.0.0`. Full semantic
versions appear in PE FileVersion/ProductVersion strings, the installed product
name, and installer/artifact filenames. Prerelease resources carry the prerelease
flag. UpgradeCode and component GUIDs remain stable.

## Release checklist

1. Update the core version, prerelease, and Windows revision in `Version.cmake`.
2. Record changes and remaining known issues in `CHANGELOG.md`.
3. Configure and build; run CTest and inspect the actual binary version resources.
4. Build the MSI. Packaging rejects binaries whose ProductVersion differs from
   the canonical version, including when using `-SkipBuild`.
5. Verify MSI ProductVersion and an upgrade from the previous installed release.
6. For input behavior changes, test Windows Terminal, WezTerm, and LINE with the
   installed build; automated engine/state tests cannot establish host compatibility.
7. Publish a new versioned artifact. When a Git tag is explicitly requested, use
   `v` followed by the full semantic version, for example `v1.0.0-beta.2`.

The default installer is `Win-McBopomofo-1.0.0-beta.2-Installer.msi` and the Actions
artifact is `Win-McBopomofo-1.0.0-beta.2`. Every release must increase both semantic
precedence and the Windows upgrade version. Rebuilding the same source for local
verification does not require a version increment.
Custom `-OutputName` values must also contain the full semantic version and end
in `.msi`.
