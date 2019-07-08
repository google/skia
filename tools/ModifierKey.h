// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef ModifierKey_DEFINED
#define ModifierKey_DEFINED
namespace ModifierKey {
    static constexpr unsigned kNone     = 0;
    static constexpr unsigned kShift    = 1 << 0;
    static constexpr unsigned kControl  = 1 << 1;
    static constexpr unsigned kOption   = 1 << 2;   // same as ALT
    static constexpr unsigned kCommand  = 1 << 3;
}
#endif  // ModifierKey_DEFINED
