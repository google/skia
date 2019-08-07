// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef ModifierKey_DEFINED
#define ModifierKey_DEFINED

#include "include/private/SkBitmaskEnum.h"

enum class ModifierKey {
    kNone       = 0,
    kShift      = 1 << 0,
    kControl    = 1 << 1,
    kOption     = 1 << 2,   // same as ALT
    kCommand    = 1 << 3,
    kFirstPress = 1 << 4,
};

namespace skstd {
template <> struct is_bitmask_enum<ModifierKey> : std::true_type {};
}

#endif  // ModifierKey_DEFINED
