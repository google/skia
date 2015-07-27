/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "SkTypes.h"

namespace SkOpts {
    // Called by SkGraphics::Init().
    // It's not thread safe, but it's fine to call more than once.
    // If Init() were somehow not called, that'd also be fine: you'll get portable fallbacks.
    void Init();

    // (Function pointers go here).
}

#endif//SkOpts_DEFINED
