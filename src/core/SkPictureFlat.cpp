/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkChecksum.h"
#include "src/core/SkPictureFlat.h"

#include <memory>

///////////////////////////////////////////////////////////////////////////////

void SkTypefacePlayback::setCount(size_t count) {
    fCount = count;
    fArray = std::make_unique<sk_sp<SkTypeface>[]>(count);
}
