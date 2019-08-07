/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrHack.h"

#include "src/gpu/GrTestUtils.h"

#include "src/gpu/GrTexturePriv.h"

void GrTextureCacheInfo::dump() const {
    SkDebugf("{ %s, %d, %d, %s, %d, %s, %s }\n",
        GrPixelConfigToStr(fConfig),
        fWidth,
        fHeight,
        (fRenderable == GrRenderable::kYes) ? "GrRenderable::kYes" : "GrRenderable::kNo",
        fSampleCount,
        (fMipMapped == GrMipMapped::kYes) ? "GrMipMapped::kYes" : "GrMipMapped::kNo",
        fHasScratchKey ? "true" : "false");
}

void GrTextureCacheInfo::computeScratchKey(GrScratchKey* dst) const {
    GrTexturePriv::ComputeScratchKey(fConfig, fWidth, fHeight, fRenderable,
        fSampleCount, fMipMapped, dst);
}

void GrCacheState::dump() const {

    SkDebugf("const GrTextureCacheInfo gNonPurgeable-%s[%d] = {",
             fName.c_str(),
             fNumNonPurgeable);
    for (int i = 0; i < fNumNonPurgeable; ++i) {
        this->nonPurgeableResource(i)->dump();
    }
    SkDebugf("};");

    SkDebugf("const GrTextureCacheInfo gPurgeable-%s[%d] = {",
             fName.c_str(),
             fNumPurgeable);
    for (int i = 0; i < fNumPurgeable; ++i) {
        this->purgeableResource(i)->dump();
    }
    SkDebugf("};");

    SkDebugf("{ \"%s\", %d, %d, gNonPurgeable-%s, %d, gPurgeable-%s }\n",
             fName.c_str(),
             fTotal,
             fNumNonPurgeable,
             fName.c_str(),
             fNumPurgeable,
             fName.c_str());
}
