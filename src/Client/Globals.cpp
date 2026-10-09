// Copyright (c) 2026 and onwards The McBopomofo Authors.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include "Globals.h"

#include <dwmapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cwchar>
#include <iterator>
#include <string>

#include "../Common/Ipc.h"
#include "../Common/NamedPipe.h"

namespace {

constexpr ULONGLONG kDiagnosticLogMaxBytes = 1024 * 1024;
constexpr ULONGLONG kDiagnosticSettingRefreshMs = 1000;

bool IsDiagnosticLoggingEnabled() {
  struct Cache {
    ULONGLONG checkedAt = 0;
    bool enabled = false;
    bool initialized = false;
  };
  static thread_local Cache cache;

  ULONGLONG now = GetTickCount64();
  if (cache.initialized && now - cache.checkedAt < kDiagnosticSettingRefreshMs) {
    return cache.enabled;
  }

  wchar_t appDataPath[MAX_PATH] = {};
  DWORD length = GetEnvironmentVariableW(L"APPDATA", appDataPath, MAX_PATH);
  if (length == 0 || length >= MAX_PATH) {
    cache.checkedAt = now;
    cache.enabled = false;
    cache.initialized = true;
    return false;
  }

  std::wstring settingsPath(appDataPath);
  settingsPath += L"\\WinMcBopomofo\\mcbopomofo.ini";
  cache.checkedAt = now;
  cache.enabled = GetPrivateProfileIntW(L"Server", L"LoggingEnabled", 0,
                                        settingsPath.c_str()) != 0;
  cache.initialized = true;
  return cache.enabled;
}

std::wstring DiagnosticLogPath() {
  wchar_t tempPath[MAX_PATH] = {};
  DWORD length = GetTempPathW(MAX_PATH, tempPath);
  if (length == 0 || length >= MAX_PATH) {
    return {};
  }

  std::wstring path(tempPath);
  path += L"mcbopomofo_tip_diagnostics.log";
  return path;
}

bool RotateDiagnosticLogIfNeeded(const std::wstring& path,
                                 DWORD incomingBytes) {
  WIN32_FILE_ATTRIBUTE_DATA attributes = {};
  if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attributes)) {
    return GetLastError() == ERROR_FILE_NOT_FOUND ||
           GetLastError() == ERROR_PATH_NOT_FOUND;
  }

  ULARGE_INTEGER fileSize = {};
  fileSize.HighPart = attributes.nFileSizeHigh;
  fileSize.LowPart = attributes.nFileSizeLow;
  if (fileSize.QuadPart + incomingBytes <= kDiagnosticLogMaxBytes) {
    return true;
  }

  std::wstring backupPath = path + L".1";
  return MoveFileExW(path.c_str(), backupPath.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}

void AppendDiagnosticLine(const char* line, DWORD length) {
  std::wstring path = DiagnosticLogPath();
  if (path.empty()) {
    return;
  }

  HANDLE mutex = CreateMutexW(nullptr, FALSE,
                              L"Local\\WinMcBopomofoTipDiagnosticsLog");
  if (!mutex) {
    return;
  }

  DWORD waitResult = WaitForSingleObject(mutex, 0);
  if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED) {
    CloseHandle(mutex);
    return;
  }

  if (RotateDiagnosticLogIfNeeded(path, length)) {
    HANDLE file = CreateFileW(
        path.c_str(), FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
      DWORD totalBytesWritten = 0;
      while (totalBytesWritten < length) {
        DWORD bytesWritten = 0;
        if (!WriteFile(file, line + totalBytesWritten,
                       length - totalBytesWritten, &bytesWritten, nullptr) ||
            bytesWritten == 0) {
          break;
        }
        totalBytesWritten += bytesWritten;
      }
      CloseHandle(file);
    }
  }

  ReleaseMutex(mutex);
  CloseHandle(mutex);
}

#ifndef NDEBUG

thread_local bool g_isRelayingClientLog = false;

ULONGLONG ElapsedMsSinceProcessStart() {
  static const ULONGLONG kStartTick = GetTickCount64();
  return GetTickCount64() - kStartTick;
}

void AppendLogLine(const char* path, DWORD processId, ULONGLONG elapsedMs,
                   const char* message) {
  FILE* fp = nullptr;
  if (fopen_s(&fp, path, "a") == 0) {
    fprintf(fp, "[%lu][+%llums] %s\n", processId, elapsedMs, message);
    fclose(fp);
  }
}

void AppendLogLineToTemp(DWORD processId, ULONGLONG elapsedMs,
                         const char* message) {
  char tempPath[MAX_PATH] = {0};
  DWORD len = GetTempPathA(MAX_PATH, tempPath);
  if (len == 0 || len >= MAX_PATH) {
    return;
  }

  std::string tempLogPath(tempPath);
  tempLogPath += "mcbopomofo_tip.log";
  AppendLogLine(tempLogPath.c_str(), processId, elapsedMs, message);
}

bool ShouldRelayToServer(const char* message) {
  static const char* const kPrefixes[] = {
      "Sending IPC request:",
      "Received IPC response:",
      "State deserialized.",
      "Failed to deserialize state update",
      "IPC Call failed",
      "CandidateUI ",
      "CandidateWindow ",
      "TooltipWindow ",
      "CCandidateListUIElement::",
      "CReadingInformationUIElement::",
      "MoveWindowsToRange ",
      "MoveWindowsToSelection ",
      "MoveWindowsToCaretFallback ",
  };

  for (const char* prefix : kPrefixes) {
    size_t prefixLength = strlen(prefix);
    if (strncmp(message, prefix, prefixLength) == 0) {
      return true;
    }
  }
  return false;
}

void RelayClientLogToServer(DWORD processId, ULONGLONG elapsedMs,
                            const char* message) {
  if (g_isRelayingClientLog || !ShouldRelayToServer(message)) {
    return;
  }

  g_isRelayingClientLog = true;

  McBopomofo::IPC::ClientLogPayload payload;
  payload.processId = processId;
  payload.elapsedMs = elapsedMs;
  payload.message = message;

  McBopomofo::IPC::NamedPipeClient pipe(McBopomofo::IPC::PIPE_NAME);
  std::string response;
  pipe.Call(McBopomofo::IPC::SerializeClientLog(payload), response);

  g_isRelayingClientLog = false;
}

void LogMessageImpl(bool relayToServer, const char* format, va_list args) {
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);

  DWORD processId = GetCurrentProcessId();
  ULONGLONG elapsedMs = ElapsedMsSinceProcessStart();

  char dbgBuffer[1100];
  sprintf_s(dbgBuffer, "[WinMcBopomofo] [%lu][+%llums] %s\n", processId,
            elapsedMs, buffer);
  OutputDebugStringA(dbgBuffer);

  AppendLogLine("C:\\Users\\Public\\mcbopomofo_tip.log", processId, elapsedMs,
                buffer);
  AppendLogLineToTemp(processId, elapsedMs, buffer);

  if (relayToServer) {
    RelayClientLogToServer(processId, elapsedMs, buffer);
  }
}

#endif  // !NDEBUG

}  // namespace

void LogMessage(const char* format, ...) {
#ifndef NDEBUG
  va_list args;
  va_start(args, format);
  LogMessageImpl(true, format, args);
  va_end(args);
#else
  (void)format;
#endif
}

void LogMessageFileOnly(const char* format, ...) {
#ifndef NDEBUG
  va_list args;
  va_start(args, format);
  LogMessageImpl(false, format, args);
  va_end(args);
#else
  (void)format;
#endif
}

void LogDiagnostic(const char* format, ...) {
  if (!format || !IsDiagnosticLoggingEnabled()) {
    return;
  }

  char message[1024] = {};
  va_list args;
  va_start(args, format);
  int messageLength = vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  if (messageLength < 0) {
    return;
  }
  if (static_cast<size_t>(messageLength) >= sizeof(message)) {
    constexpr char kTruncated[] = "...";
    memcpy(message + sizeof(message) - sizeof(kTruncated), kTruncated,
           sizeof(kTruncated));
  }

  size_t safeLength = strlen(message);
  for (size_t i = 0; i < safeLength; ++i) {
    if (message[i] == '\r' || message[i] == '\n') {
      message[i] = ' ';
    }
  }

  char line[1152] = {};
  int lineLength = snprintf(line, sizeof(line), "[%lu][%llu] %s\r\n",
                            GetCurrentProcessId(),
                            static_cast<unsigned long long>(GetTickCount64()),
                            message);
  if (lineLength <= 0 || static_cast<size_t>(lineLength) >= sizeof(line)) {
    return;
  }

  size_t bytesToWrite = static_cast<size_t>(lineLength);
  AppendDiagnosticLine(line, static_cast<DWORD>(bytesToWrite));
}

float GetDpiScaleForWindow(HWND hwnd) {
  if (!hwnd) return 1.0f;
  UINT dpi = 96;
  HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
  auto pGetDpiForWindow =
      (UINT(WINAPI*)(HWND))GetProcAddress(hUser32, "GetDpiForWindow");
  if (pGetDpiForWindow) {
    dpi = pGetDpiForWindow(hwnd);
  } else {
    HDC hdc = GetDC(hwnd);
    if (hdc) {
      dpi = GetDeviceCaps(hdc, LOGPIXELSX);
      ReleaseDC(hwnd, hdc);
    }
  }
  return (float)dpi / 96.0f;
}

void EnableWindowDropShadow(HWND hwnd) {
  if (!hwnd) {
    return;
  }

  // Ask the window manager to keep a tiny frame so borderless popup windows
  // can still receive the standard DWM shadow.
  const MARGINS margins = {1, 1, 1, 1};
  DwmExtendFrameIntoClientArea(hwnd, &margins);

  BOOL enabled = FALSE;
  if (FAILED(DwmIsCompositionEnabled(&enabled)) || !enabled) {
    return;
  }

  const DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
  DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy,
                        sizeof(policy));

  // Keep the DWM shadow but suppress the compositor-drawn border/frame color
  // that otherwise shows up as a gray rectangle around popup windows.
  constexpr COLORREF kDwmColorNone = static_cast<COLORREF>(0xFFFFFFFE);
  DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &kDwmColorNone,
                        sizeof(kDwmColorNone));
}
