/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderPriv_DEFINED
#define SkImageEncoderPriv_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkImageInfoPriv.h"

static inline bool SkPixmapIsValid(const SkPixmap& src) {
    if (!SkImageInfoIsValid(src.info())) {
        return false;
    }

    if (!src.addr() || src.rowBytes() < src.info().minRowBytes()) {
        return false;
    }

    return true;
}

#endif // SkImageEncoderPriv_DEFINED
