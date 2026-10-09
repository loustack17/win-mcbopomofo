#include "NativeAsciiPolicy.h"

#include <gtest/gtest.h>

using McBopomofo::IPC::StateUpdatePayload;
using McBopomofo::TSF::ShouldCommitNativeAscii;

namespace {

StateUpdatePayload EmptyState() { return {}; }

}

TEST(NativeAsciiPolicyTest, MatchesOnlySupportedHostsCaseInsensitively) {
  const auto state = EmptyState();
  EXPECT_TRUE(ShouldCommitNativeAscii("wezterm-gui.exe", state, state, '1',
                                      false, false));
  EXPECT_TRUE(ShouldCommitNativeAscii("WEZTERM.EXE", state, state, ';', false,
                                      false));
  EXPECT_TRUE(ShouldCommitNativeAscii("line.EXE", state, state, '!', false,
                                      false));
  EXPECT_FALSE(ShouldCommitNativeAscii("WindowsTerminal.exe", state, state,
                                       '1', false, false));
  EXPECT_FALSE(ShouldCommitNativeAscii("not-line.exe", state, state, '1',
                                       false, false));
}

TEST(NativeAsciiPolicyTest, AllowsDigitsAndPunctuation) {
  const auto state = EmptyState();
  EXPECT_TRUE(ShouldCommitNativeAscii("LINE.exe", state, state, '0', false,
                                      false));
  EXPECT_TRUE(ShouldCommitNativeAscii("LINE.exe", state, state, '.', false,
                                      false));
  EXPECT_TRUE(ShouldCommitNativeAscii("LINE.exe", state, state, '~', false,
                                      false));
}

TEST(NativeAsciiPolicyTest, RejectsLettersSpaceAndControlCharacters) {
  const auto state = EmptyState();
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, 'a', false,
                                       false));
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, 'Z', false,
                                       false));
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, ' ', false,
                                       false));
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, '\n', false,
                                       false));
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, 0x80, false,
                                       false));
}

TEST(NativeAsciiPolicyTest, RequiresEmptyStateBeforeAndAfterServerUpdate) {
  const auto empty = EmptyState();
  auto state = EmptyState();
  state.composingBuffer = "ㄅ";
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, empty, '1', false,
                                       false));
  state = EmptyState();
  state.candidates = {"候選"};
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, empty, '1', false,
                                       false));
  state = EmptyState();
  state.composingBuffer = "ㄅ";
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", empty, state, '1', false,
                                       false));
  state = EmptyState();
  state.candidates = {"候選"};
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", empty, state, '1', false,
                                       false));
  state = EmptyState();
  state.commitString = "已提交";
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", empty, state, '1', false,
                                       false));
}

TEST(NativeAsciiPolicyTest, RefusesKeysConsumedByServer) {
  auto state = EmptyState();
  state.consumed = true;
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", EmptyState(), state, '1',
                                       false, false));
}

TEST(NativeAsciiPolicyTest, IgnoresCommitStringFromPreviousKey) {
  auto before = EmptyState();
  before.commitString = "前一個字";
  const auto after = EmptyState();
  EXPECT_TRUE(ShouldCommitNativeAscii("LINE.exe", before, after, '1', false,
                                      false));
}

TEST(NativeAsciiPolicyTest, RefusesKeysWithOtherModifiers) {
  const auto state = EmptyState();
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, '1', true,
                                       false));
}

TEST(NativeAsciiPolicyTest, RefusesWhenTipCompositionIsActive) {
  const auto state = EmptyState();
  EXPECT_FALSE(ShouldCommitNativeAscii("LINE.exe", state, state, ';', false,
                                       true));
}
