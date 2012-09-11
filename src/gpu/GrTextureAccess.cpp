/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAccess.h"
#include "SkString.h"

GrTextureAccess::GrTextureAccess(const GrTexture* texture, const SkString& swizzle)
    : fTexture(texture) {
    GrAssert(swizzle.size() <= 4);
    for (unsigned int offset = 0; offset < swizzle.size(); ++offset) {
        fSwizzle[offset] = swizzle[offset];
    }
    if (swizzle.size() < 4) {
      fSwizzle[swizzle.size()] = 0;
    }
}

