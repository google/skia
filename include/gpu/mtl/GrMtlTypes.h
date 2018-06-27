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

/**
 * Declares typedefs for Metal types used in Ganesh cpp code
 */
typedef unsigned int GrMTLPixelFormat;

///////////////////////////////////////////////////////////////////////////////
/**
 * Types for interacting with Metal resources created externally to Skia. Holds the MTLTexture as a
 * const void* and will extract the relevant info so it can be easily accessed by Ganesh cpp code.
 * This is used by GrBackendObjects.
 */
struct GrMtlTextureInfo {
public:
    int fWidth = 0;
    int fHeight = 0;
    int fSampleCnt = 1;
    int fLevelCount = 1;
    GrMTLPixelFormat fFormat = 0; // MTLPixelFormatInvalid
    const void* fTexture = nullptr;

    GrMtlTextureInfo(const void* mtlTexture = nullptr) {
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
