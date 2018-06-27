/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTypes_DEFINED
#define GrMtlTypes_DEFINED

#include "GrTypes.h"
#include "../private/GrMtlTrampoline.h"

typedef unsigned int GrMTLPixelFormat;

struct GrMtlTextureInfo {
public:
    int fWidth = 0;
    int fHeight = 0;
    int fSampleCnt = 1;
    int fLevelCount = 1;
    GrMTLPixelFormat fFormat = 0; // MTLPixelFormatInvalid
    void* fTexture = nullptr;

    GrMtlTextureInfo(void* mtlTexture = nullptr) {
        if (mtlTexture) {
            GrMtlTrampoline::ExtractMTLTextureInfo(mtlTexture, this);
        }
    }

    bool operator==(const GrMtlTextureInfo& that) const {
        return fWidth == that.fWidth && fHeight == that.fHeight && fSampleCnt == that.fSampleCnt &&
        fLevelCount == that.fLevelCount && fFormat == that.fFormat && fTexture == that.fTexture;
    }
};

#endif
