# Windows Installer

The MSI packages the x64, x86, and ARM64 builds and their runtime data. See
[development requirements](development-requirements.md) and
[version metadata](version-metadata.md) for the toolchain and version rules.

## Installation and Upgrade

Installation and upgrade require administrator elevation because the MSI is per-machine.

The installer selects the appropriate binaries, deploys them with the language
models and OpenCC dictionaries, registers the TSF DLLs, creates the configuration
shortcut and server autorun entry, and starts the server.

`installer/installer.wxs` defines these actions. The active application-close
check uses `scripts/CheckAndCloseAppsWithTIP.vbs`; the development installation
path uses `scripts/close_ime_apps.ps1` instead.

Server close actions first request graceful shutdown, then use a five-second
termination fallback with exit code zero (`TerminateProcess="0"`). They use `RebootPrompt="no"`; configuration-app close
actions use `RebootPrompt="yes"`. Windows Installer may still request a reboot
when locked files cannot be replaced. Preserve this distinction when changing
shutdown behavior.

Upgrades detected through `WIX_UPGRADE_DETECTED` retain the installation
directory and skip directory editing. Preserve the UpgradeCode and component
GUIDs. Uninstallation reverses registration, shortcuts, autorun, and deployed
files.

## Build

Use Visual Studio C++ tools for all three architectures, CMake, and WiX 7.
`build_msi.ps1` manages `WixToolset.UI.wixext/7.0.0` and
`WixToolset.Util.wixext/7.0.0`.

```powershell
wix eula accept wix7
.\build_msi.ps1
```

The script builds Release binaries for all architectures, generates OpenCC
dictionaries, verifies their version metadata, converts the license to RTF,
and packages the MSI. `-Configuration Debug` selects Debug binaries.

After building matching binaries, packaging-only verification can use:

```powershell
.\build_msi.ps1 -SkipBuild
```

Outputs are written to `dist/Win-McBopomofo-<version>-Installer.msi`, with the
complete semantic version from `Version.cmake`. A custom `-OutputName` must
contain that current version and end in `.msi`.

License generation preserves `LICENSE.txt`, joins hard-wrapped lines within
each paragraph, and separates paragraphs with `\par\par` in the generated RTF.
Keep the license dialog's paragraph spacing readable.

## Verification

Before release, check the MSI ProductVersion, installer UpgradeCode, and version
resources in all nine packaged binaries. Test registration and upgrade behavior
when changing installer actions. An administrative extraction verifies package
contents but does not substitute for an installed upgrade test.
