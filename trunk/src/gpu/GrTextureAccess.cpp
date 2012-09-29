/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAccess.h"

#include "GrTexture.h"

GrTextureAccess::GrTextureAccess() {
#if GR_DEBUG
    memcpy(fSwizzle, "void", 5);
    fSwizzleMask = 0xbeeffeed;
#endif
}

GrTextureAccess::GrTextureAccess(GrTexture* texture, const char* swizzle) {
    this->reset(texture, swizzle);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture) {
    this->reset(texture);
}

void GrTextureAccess::reset(GrTexture* texture, const char* swizzle) {
    GrAssert(NULL != texture);
    GrAssert(strlen(swizzle) >= 1 && strlen(swizzle) <= 4);

    texture->ref();
    fTexture.reset(texture);

    fSwizzleMask = 0;
    fSwizzle[4] = '\0';
    int i = 0;
    do {
        fSwizzle[i] = swizzle[i];
        switch (swizzle[i]) {
            case 'r':
                fSwizzleMask |= kR_SwizzleFlag;
                break;
            case 'g':
                fSwizzleMask |= kG_SwizzleFlag;
                break;
            case 'b':
                fSwizzleMask |= kB_SwizzleFlag;
                break;
            case 'a':
                fSwizzleMask |= kA_SwizzleFlag;
                break;
            case '\0':
                break;
            default:
                GrCrash("Unexpected swizzle string character.");
                break;
        }
    } while ('\0' != swizzle[i] && ++i < 4);
}

void GrTextureAccess::reset(GrTexture* texture) {
    GrAssert(NULL != texture);
    texture->ref();
    fTexture.reset(texture);
    memcpy(fSwizzle, "rgba", 5);
    fSwizzleMask = (kRGB_SwizzleMask | kA_SwizzleFlag);
}
