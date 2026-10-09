# 小麥注音 Windows 維護版

[![建置與發行](https://github.com/loustack17/win-mcbopomofo/actions/workflows/fork-ci.yml/badge.svg?branch=master)](https://github.com/loustack17/win-mcbopomofo/actions/workflows/fork-ci.yml)

這個專案來自 [OpenVanilla 的 Windows 版小麥注音](https://github.com/openvanilla/win-mcbopomofo)。原始開發與 Windows 移植都是上游作者和貢獻者的成果，作者與版權標示也都保留。

我平常需要使用小麥注音，所以把遇到的問題修好，並在這裡繼續維護。希望它能穩定使用，也希望程式碼保持簡潔。上游不接受外部合併請求，因此這裡獨立維護自己的修改與安裝版本。

## 下載與安裝

支援 Windows 10 以上版本，包含 x64、x86 與 ARM64。

已發布的版本可從[版本下載頁](https://github.com/loustack17/win-mcbopomofo/releases)取得。尚未發布的建置可在[自動建置頁](https://github.com/loustack17/win-mcbopomofo/actions/workflows/fork-ci.yml)找到：開啟成功的執行紀錄，下載附有版本號的安裝檔。

安裝檔目前沒有數位簽章。升級前請先儲存工作；安裝後重新開啟使用輸入法的程式，讓它載入新版元件。若安裝程式要求重新啟動，請依提示操作。

## 分支與發行

- `master`：日常開發、修正與發行使用的主幹。
- `main`：保留上游版本，供比對與參考。

推送到 `master`，或提出以 `master` 為目標的合併請求，都會自動建置、測試並產生安裝檔。

要發布版本，在自動建置頁選擇手動執行，分支選 `master`，並勾選 `publish_release`。流程通過後會建立版本標籤、發布版本並附上安裝檔。測試版會標示為預先發行版本，已使用的版本標籤不會覆寫。

版號統一由 `Version.cmake` 管理。修改後要發布新的程式檔案時，必須使用新的版號；詳細規則見[版本管理](docs/version-metadata.md)。

## 開發與測試

需要具備 C++ 桌面開發工具與 Windows SDK 的 Visual Studio，以及 CMake。製作安裝檔另外需要 WiX Toolset 7；建置 ARM64 時也要安裝對應的編譯工具。

基本建置與測試：

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

製作安裝檔：

```powershell
wix eula accept wix7
.\build_msi.ps1
```

安裝檔會放在 `dist`，檔名包含完整版本號，例如 `Win-McBopomofo-1.0.0-beta.3-Installer.msi`。

`.\install.ps1` 提供開發用的本機建置與註冊流程，需要系統管理員權限。其他安裝、移除與程序管理腳本的用途見[腳本說明](scripts/README.md)。

## 專案結構

- `src/Client`：載入應用程式的輸入法元件，處理按鍵、組字與文字提交。
- `src/Server`：輸入引擎、候選字處理與選字視窗。
- `src/ConfigApp`：設定程式。
- `src/Common`：共用的通訊與工具。
- `data`：詞庫、語言模型與注音資料。
- `tests`：測試與問題重現案例。
- `installer`、`scripts`：安裝相關檔案。
- `docs`：詳細技術文件，部分文件目前仍為英文。
- `third_party`：第三方相依套件，保留各自的授權與來源。

後續會逐步移除確認不再使用的舊檔案與重複程式碼。每次整理都會先查相依性，再驗證功能；仍在使用的測試、安裝升級相容處理與授權資料會保留。

詞庫與語言模型來自 [macOS 版小麥注音](https://github.com/openvanilla/McBopomofo)。授權資訊見 [LICENSE.txt](LICENSE.txt)。
