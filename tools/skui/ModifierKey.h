// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef skui_modifierkey_defined
#define skui_modifierkey_defined

#include "include/private/SkBitmaskEnum.h"

namespace skui {
enum class ModifierKey {
    kNone       = 0,
    kShift      = 1 << 0,
    kControl    = 1 << 1,
    kOption     = 1 << 2,   // same as ALT
    kCommand    = 1 << 3,
    kFirstPress = 1 << 4,
};
}  // namespace skui

namespace sknonstd {
template <> struct is_bitmask_enum<skui::ModifierKey> : std::true_type {};
}  // namespace sknonstd
#endif  // skui_modifierkey_defined
