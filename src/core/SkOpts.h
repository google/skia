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
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init(), and automatically #if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS.
    void Init();

    // (Function pointers go here).
}

#endif//SkOpts_DEFINED
