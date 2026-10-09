#pragma once

namespace McBopomofo::TSF {

class ShiftKeyState {
 public:
  bool testKeyDown(bool shiftKey, bool otherModifier, bool enabled) {
    if (!shiftKey || otherModifier) {
      pending_ = false;
    }
    return shiftKey && !otherModifier && enabled;
  }

  bool keyDown(bool shiftKey, bool otherModifier, bool enabled) {
    pending_ = shiftKey && !otherModifier && enabled;
    return pending_;
  }

  bool testKeyUp(bool shiftKey, bool otherModifier, bool enabled) const {
    return shiftKey && !otherModifier && enabled && pending_;
  }

  bool keyUp(bool shiftKey, bool otherModifier, bool enabled) {
    bool toggle = shiftKey && !otherModifier && enabled && pending_;
    pending_ = false;
    return toggle;
  }

  void reset() { pending_ = false; }

 private:
  bool pending_ = false;
};

}
