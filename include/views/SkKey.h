
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkKey_DEFINED
#define SkKey_DEFINED

#include "SkTypes.h"

enum SkModifierKeys {
    kShift_SkModifierKey    = 1 << 0,
    kControl_SkModifierKey  = 1 << 1,
    kOption_SkModifierKey   = 1 << 2,   // same as ALT
    kCommand_SkModifierKey  = 1 << 3,
};

#endif
