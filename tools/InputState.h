// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef InputState_DEFINED
#define InputState_DEFINED
enum class InputState {
    kDown,
    kUp,
    kMove   // only valid for mouse
};
#endif  // InputState_DEFINED
