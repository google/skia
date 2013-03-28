/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAccess.h"
#include "GrColor.h"
#include "GrTexture.h"

GrTextureAccess::GrTextureAccess() {
#if GR_DEBUG
    memcpy(fSwizzle, "void", 5);
    fSwizzleMask = 0xbeeffeed;
#endif
}

GrTextureAccess::GrTextureAccess(GrTexture* texture, const GrTextureParams& params) {
    this->reset(texture, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 bool bilerp,
                                 SkShader::TileMode tileXAndY) {
    this->reset(texture, bilerp, tileXAndY);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 const char* swizzle,
                                 const GrTextureParams& params) {
    this->reset(texture, swizzle, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 const char* swizzle,
                                 bool bilerp,
                                 SkShader::TileMode tileXAndY) {
    this->reset(texture, swizzle, bilerp, tileXAndY);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const char* swizzle,
                            const GrTextureParams& params) {
    GrAssert(NULL != texture);
    GrAssert(strlen(swizzle) >= 1 && strlen(swizzle) <= 4);

    fParams = params;
    fTexture.reset(SkRef(texture));
    this->setSwizzle(swizzle);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const char* swizzle,
                            bool bilerp,
                            SkShader::TileMode tileXAndY) {
    GrAssert(NULL != texture);
    GrAssert(strlen(swizzle) >= 1 && strlen(swizzle) <= 4);

    fParams.reset(tileXAndY, bilerp);
    fTexture.reset(SkRef(texture));
    this->setSwizzle(swizzle);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const GrTextureParams& params) {
    GrAssert(NULL != texture);
    fTexture.reset(SkRef(texture));
    fParams = params;
    memcpy(fSwizzle, "rgba", 5);
    fSwizzleMask = kRGBA_GrColorComponentFlags;
}

void GrTextureAccess::reset(GrTexture* texture,
                            bool bilerp,
                            SkShader::TileMode tileXAndY) {
    GrAssert(NULL != texture);
    fTexture.reset(SkRef(texture));
    fParams.reset(tileXAndY, bilerp);
    memcpy(fSwizzle, "rgba", 5);
    fSwizzleMask = kRGBA_GrColorComponentFlags;
}

void GrTextureAccess::setSwizzle(const char* swizzle) {
    fSwizzleMask = 0;
    memset(fSwizzle, '\0', 5);
    for (int i = 0; i < 4 && '\0' != swizzle[i]; ++i) {
        fSwizzle[i] = swizzle[i];
        switch (swizzle[i]) {
            case 'r':
                fSwizzleMask |= kR_GrColorComponentFlag;
                break;
            case 'g':
                fSwizzleMask |= kG_GrColorComponentFlag;
                break;
            case 'b':
                fSwizzleMask |= kB_GrColorComponentFlag;
                break;
            case 'a':
                fSwizzleMask |= kA_GrColorComponentFlag;
                break;
            default:
                GrCrash("Unexpected swizzle string character.");
                break;
        }
    }
}
