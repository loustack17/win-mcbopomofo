# Candidate Mode and Space Bar Behavior

## Purpose

This document explains the actual definition of the space bar behavior in the current Windows version, to avoid confusing two different things:

- Whether the space bar is used to "enter candidate mode"
- Once in candidate mode, whether the space bar is used to "turn the page"

## Conclusion

The current system semantics are as follows:

1. The `ChooseCandidateUsingSpace` setting controls:
   Whether it is allowed to enter candidate mode using the space bar from a normal input state.
2. Once candidate mode has been entered:
   The meaning of the space bar is fixed to "next page in the candidate list".

In other words, this setting is not "whether the space bar selects characters in candidate mode", but rather "whether the space bar switches into candidate mode from a normal input state".

## Space Bar in Normal Input State

Implementation is located in `src/Server/KeyHandler.cpp`.

- If the current state is `NotEmpty`, and:
    - The user presses `Shift+Space`, or
    - `ChooseCandidateUsingSpace == false`
- Then the space bar is treated as a literal space character and inserted into the composing buffer.

At the end of the buffer, this commits the Chinese text together with the space
immediately. Inside the buffer, it keeps the composition active for editing.
With an empty buffer, a plain space passes through to the application. Space
still completes an unfinished Bopomofo reading before acting as a separator.

Conversely:

- If the current state is `NotEmpty`
- And `ChooseCandidateUsingSpace == true`
- And reading is empty

Then pressing the space bar will enter the candidate choosing state.

## Direct Punctuation

In normal McBopomofo mode, punctuation at the end of the composing buffer commits
immediately, including any preceding composed Chinese text. Standalone full-width
and half-width punctuation also commits immediately, without an extra Enter key.

When the buffer is empty and a half-width punctuation mapping produces exactly
the original character, the key passes through to the application. This avoids
creating a TSF composition for unchanged ASCII punctuation. Layout mappings that
change the character still use the input method's normal commit path.

Punctuation does not confirm an unfinished Bopomofo reading. Inserting punctuation
inside the buffer retains the composition for editing. Enabling repeated-key
punctuation selection retains the original composition behavior so alternatives
can still be selected. Plain Bopomofo mode and the punctuation menu retain their
existing candidate selection behavior.

## TSF Key Routing and Diagnostics

With `ShiftToggleOpenClose` enabled, a standalone Shift press and release is
claimed consistently by both the TSF test callbacks and the actual callbacks,
including while the input method is closed. Shift combined with another key
cancels the pending toggle. Focus changes also cancel it. The setting is cached
for up to one second to avoid querying the server on every callback.

With no composition or candidates, Space is passed through at the TSF test
callback, so the host can process its original key event.

When `[Server] LoggingEnabled=1` in `mcbopomofo.ini`, Release builds also write
`%TEMP%\mcbopomofo_tip_diagnostics.log`. It contains process IDs, timing, key
categories, context addresses, text lengths, and TSF result codes, without typed
text. The file rotates at 1 MiB with one backup. Busy writes are dropped rather
than waiting. Changes to the logging setting take effect within one second.
Logging is disabled by default; enabled logging performs local file I/O during
input processing and is intended for short diagnostic sessions.

Automated tests verify the Shift state transitions and server punctuation rules.
They do not verify TSF event delivery in LINE or WezTerm; those hosts require
testing with the installed build and comparing their diagnostic events with
Windows Terminal.

LINE and WezTerm may fail to output native punctuation and digits after the TIP
claims a key during testing but the server later declines it. For these hosts
only, a declined printable ASCII digit or punctuation is synchronously inserted
through TSF when both the previous and current composition/candidate state are
empty, no TIP composition exists, and Ctrl, Alt, and Windows keys are absent.
The actual key callback reports the key as eaten only after SetText succeeds;
failure before writing leaves it uneaten. If a later cleanup operation fails,
the already written key remains eaten to prevent duplicate insertion. Failures
are recorded by stage in the local diagnostic log. Letters and Space keep their
existing routing. Server punctuation mappings and active composition are retained.

Shift release is additionally registered as a TSF preserved key following the
Microsoft SampleIME pattern. Preserved and ordinary release callbacks share the
same pending-press guard, so a single press toggles at most once. Prediction
callbacks do not toggle modes. Actual delivery in each host requires live testing.

## Space Bar in Candidate Mode

Implementation is located in `HandleCandidateKey()` of `src/Server/InputController.cpp`.

When the current state is a candidate state, such as:

- `ChoosingCandidate`
- `SelectingDictionary`
- `ShowingCharInfo`
- `AssociatedPhrases`
- `AssociatedPhrasesPlain`
- `NumberInput`
- `SelectingFeature`
- `SelectingDateMacro`
- `IcuTransformInput`
- `CustomMenu`

Pressing the space bar will execute:

- `MoveCandidatePage(true)`

That is, it turns to the next page, rather than directly selecting the current candidate.

## Comparison with the fcitx5 Version

The fcitx5 version also treats "space bar in normal input state" and "space bar in candidate mode" as logic at different layers:

- In the normal input state, `KeyHandler` decides whether to use the space bar to switch into candidate choosing.
- After entering the candidate panel, the candidate list key processing is handled by the candidate mode logic and framework UI.

The previous issue in the Windows version was not missing the `Space -> page down` logic, but that `UI update` was not immediately called after turning the page, so the screen looked as if the page hadn't turned. This issue has been fixed by adding `ui_->Update(...)` in `InputController::HandleCandidateKey()`.

## Note on Setting Name

Currently, the setting UI displays:

- `使用空白鍵選取候選字` (Use Space to Select Candidates)

But according to the actual program behavior, this text is not precise. A description closer to the implementation would be:

- `使用空白鍵進入候選模式` (Use Space to Enter Candidate Mode)

Or:

- `空白鍵用於候選模式切入` (Space Bar Used for Candidate Mode Switch)

If we want to improve user understanding in the future, it is recommended to prioritize modifying the UI copy, rather than modifying the underlying semantics of this setting.
