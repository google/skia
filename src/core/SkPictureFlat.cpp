/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureFlat.h"
#include "SkChecksum.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkShader.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////

void SkTypefacePlayback::setCount(size_t count) {
    fCount = count;
    fArray.reset(new sk_sp<SkTypeface>[count]);
}
