/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLumaColorFilter.h"

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    const SkScalar luma_to_alpha[] = {
              0,       0,       0, 0, 0,
              0,       0,       0, 0, 0,
              0,       0,       0, 0, 0,
        0.2126f, 0.7152f, 0.0722f, 0, 0,
    };
    return SkColorFilter::MakeMatrixFilterRowMajor255(luma_to_alpha);
}
