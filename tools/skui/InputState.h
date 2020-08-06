// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef skui_inputstate_DEFINED
#define skui_inputstate_DEFINED
namespace skui {
enum class InputState {
    kDown,
    kUp,
    kMove,   // only valid for mouse
    kRight,  // only valid for fling
    kLeft,   // only valid for fling
};
}  // namespace skui
#endif  // skui_inputstate_DEFINED
