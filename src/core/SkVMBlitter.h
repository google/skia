/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVMBlitter_DEFINED
#define SkVMBlitter_DEFINED

#include "include/core/SkBlendMode.h"
#include "src/core/SkVM.h"

namespace skvm {
    struct Color { skvm::I32 r,g,b,a; };

    bool BlendModeSupported(SkBlendMode);
    Color BlendModeProgram(Builder*, SkBlendMode, const Color& src, const Color& dst);
}

#endif
