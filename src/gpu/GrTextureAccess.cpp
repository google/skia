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
#ifdef SK_DEBUG
    memcpy(fSwizzle, "void", 5);
    fSwizzleMask = 0xbeeffeed;
#endif
}

GrTextureAccess::GrTextureAccess(GrTexture* texture, const GrTextureParams& params) {
    this->reset(texture, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 GrTextureParams::FilterMode filterMode,
                                 SkShader::TileMode tileXAndY) {
    this->reset(texture, filterMode, tileXAndY);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 const char* swizzle,
                                 const GrTextureParams& params) {
    this->reset(texture, swizzle, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 const char* swizzle,
                                 GrTextureParams::FilterMode filterMode,
                                 SkShader::TileMode tileXAndY) {
    this->reset(texture, swizzle, filterMode, tileXAndY);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const char* swizzle,
                            const GrTextureParams& params) {
    SkASSERT(texture);
    SkASSERT(strlen(swizzle) >= 1 && strlen(swizzle) <= 4);

    fParams = params;
    fTexture.set(SkRef(texture), GrIORef::kRead_IOType);
    this->setSwizzle(swizzle);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const char* swizzle,
                            GrTextureParams::FilterMode filterMode,
                            SkShader::TileMode tileXAndY) {
    SkASSERT(texture);
    SkASSERT(strlen(swizzle) >= 1 && strlen(swizzle) <= 4);

    fParams.reset(tileXAndY, filterMode);
    fTexture.set(SkRef(texture), GrIORef::kRead_IOType);
    this->setSwizzle(swizzle);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const GrTextureParams& params) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), GrIORef::kRead_IOType);
    fParams = params;
    memcpy(fSwizzle, "rgba", 5);
    fSwizzleMask = kRGBA_GrColorComponentFlags;
}

void GrTextureAccess::reset(GrTexture* texture,
                            GrTextureParams::FilterMode filterMode,
                            SkShader::TileMode tileXAndY) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), GrIORef::kRead_IOType);
    fParams.reset(tileXAndY, filterMode);
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
                SkFAIL("Unexpected swizzle string character.");
                break;
        }
    }
}
