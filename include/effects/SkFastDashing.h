/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFastDashing_DEFINED
#define SkFastDashing_DEFINED

#include "include/core/SkPathEffect.h"

class SK_API SkFastDashing {
public:

    static sk_sp<SkPathEffect> Make(const SkScalar intervals[], int count);
};

#endif
