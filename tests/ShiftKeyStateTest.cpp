#include "ShiftKeyState.h"

#include <gtest/gtest.h>

using McBopomofo::TSF::ShiftKeyState;

TEST(ShiftKeyStateTest, ConformingHostDispatchesShiftTap) {
  ShiftKeyState state;
  ASSERT_TRUE(state.testKeyDown(true, false, true));
  ASSERT_TRUE(state.keyDown(true, false, true));
  ASSERT_TRUE(state.testKeyUp(true, false, true));
  EXPECT_TRUE(state.keyUp(true, false, true));
  EXPECT_FALSE(state.testKeyUp(true, false, true));
}

TEST(ShiftKeyStateTest, PredictionDoesNotArmToggle) {
  ShiftKeyState state;
  EXPECT_TRUE(state.testKeyDown(true, false, true));
  EXPECT_FALSE(state.testKeyUp(true, false, true));
  EXPECT_FALSE(state.keyUp(true, false, true));
}

TEST(ShiftKeyStateTest, ShiftLetterDoesNotToggleWhenLetterIsPassedThrough) {
  ShiftKeyState state;
  ASSERT_TRUE(state.keyDown(true, false, true));
  EXPECT_FALSE(state.testKeyDown(false, false, true));
  EXPECT_FALSE(state.testKeyUp(true, false, true));
  EXPECT_FALSE(state.keyUp(true, false, true));
}

TEST(ShiftKeyStateTest, ModifiedShiftIsNotClaimed) {
  ShiftKeyState state;
  EXPECT_FALSE(state.testKeyDown(true, true, true));
  EXPECT_FALSE(state.keyDown(true, true, true));
  EXPECT_FALSE(state.testKeyUp(true, true, true));
  EXPECT_FALSE(state.keyUp(true, true, true));
}

TEST(ShiftKeyStateTest, DisabledSettingDoesNotClaimShift) {
  ShiftKeyState state;
  EXPECT_FALSE(state.testKeyDown(true, false, false));
  EXPECT_FALSE(state.keyDown(true, false, false));
  EXPECT_FALSE(state.testKeyUp(true, false, false));
  EXPECT_FALSE(state.keyUp(true, false, false));
}

TEST(ShiftKeyStateTest, FocusLossCancelsPendingToggle) {
  ShiftKeyState state;
  ASSERT_TRUE(state.keyDown(true, false, true));
  state.reset();
  EXPECT_FALSE(state.keyUp(true, false, true));
}

TEST(ShiftKeyStateTest, RepeatedShiftDownTogglesOnlyOnce) {
  ShiftKeyState state;
  ASSERT_TRUE(state.keyDown(true, false, true));
  ASSERT_TRUE(state.keyDown(true, false, true));
  EXPECT_TRUE(state.keyUp(true, false, true));
  EXPECT_FALSE(state.keyUp(true, false, true));
}

TEST(ShiftKeyStateTest, PreservedReleaseThenActualReleaseTogglesOnce) {
  ShiftKeyState state;
  ASSERT_TRUE(state.keyDown(true, false, true));
  EXPECT_TRUE(state.keyUp(true, false, true));
  EXPECT_FALSE(state.keyUp(true, false, true));
}

TEST(ShiftKeyStateTest, ActualReleaseThenPreservedReleaseTogglesOnce) {
  ShiftKeyState state;
  ASSERT_TRUE(state.keyDown(true, false, true));
  EXPECT_TRUE(state.testKeyUp(true, false, true));
  EXPECT_TRUE(state.keyUp(true, false, true));
  EXPECT_FALSE(state.keyUp(true, false, true));
}
