#pragma once

#include <string_view>

#include "Ipc.h"

namespace McBopomofo::TSF {

inline bool ShouldCommitNativeAscii(
    std::string_view hostName,
    const IPC::StateUpdatePayload& before,
    const IPC::StateUpdatePayload& after, unsigned ascii,
    bool otherModifier, bool hasComposition) {
  const auto equalsIgnoreCase = [](std::string_view left,
                                   std::string_view right) {
    if (left.size() != right.size()) {
      return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
      char leftChar = left[i];
      char rightChar = right[i];
      if (leftChar >= 'A' && leftChar <= 'Z') {
        leftChar = static_cast<char>(leftChar - 'A' + 'a');
      }
      if (rightChar >= 'A' && rightChar <= 'Z') {
        rightChar = static_cast<char>(rightChar - 'A' + 'a');
      }
      if (leftChar != rightChar) {
        return false;
      }
    }
    return true;
  };

  const bool supportedHost = equalsIgnoreCase(hostName, "wezterm-gui.exe") ||
                             equalsIgnoreCase(hostName, "wezterm.exe") ||
                             equalsIgnoreCase(hostName, "LINE.exe");
  const bool emptyState = before.composingBuffer.empty() &&
                          before.candidates.empty() &&
                          after.composingBuffer.empty() &&
                          after.candidates.empty() &&
                          after.commitString.empty();
  const bool digit = ascii >= '0' && ascii <= '9';
  const bool upperLetter = ascii >= 'A' && ascii <= 'Z';
  const bool lowerLetter = ascii >= 'a' && ascii <= 'z';
  const bool punctuation = ascii >= 0x21 && ascii <= 0x7e && !digit &&
                           !upperLetter && !lowerLetter;

  return supportedHost && (digit || punctuation) && emptyState &&
         !after.consumed && !otherModifier && !hasComposition;
}

}
