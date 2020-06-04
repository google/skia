/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkAlignedDashPathEffect.h"
#include "src/effects/SkDashImpl.h"
#include "src/utils/SkDashPathPriv.h"


sk_sp<SkPathEffect> SkAlignedDashPathEffect::Make(const SkScalar intervals[], int count) {
    if (!SkDashPath::ValidDashPath(0, intervals, count)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkAlignedDashImpl(intervals, count));
}
