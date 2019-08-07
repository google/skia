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
    SkDebugf("{ %s, %d, %d, %s, %d, %s, %s, %zu }",
        GrPixelConfigToStr(fConfig),
        fWidth,
        fHeight,
        (fRenderable == GrRenderable::kYes) ? "GrRenderable::kYes" : "GrRenderable::kNo",
        fSampleCount,
        (fMipMapped == GrMipMapped::kYes) ? "GrMipMapped::kYes" : "GrMipMapped::kNo",
        fHasScratchKey ? "true" : "false",
        fBytes);
}

void GrTextureCacheInfo::computeScratchKey(GrScratchKey* dst) const {
    GrTexturePriv::ComputeScratchKey(fConfig, fWidth, fHeight, fRenderable,
        fSampleCount, fMipMapped, dst);
}

void GrCacheState::dump() const {

    SkString nonPurgeableName("nullptr");
    if (fNumNonPurgeable) {
        nonPurgeableName.printf("gNonPurgeable-%s", fName.c_str());

        SkDebugf("const GrTextureCacheInfo %s[%d] = {\n",
                 nonPurgeableName.c_str(),
                 fNumNonPurgeable);
        for (int i = 0; i < fNumNonPurgeable; ++i) {
            this->nonPurgeableResource(i)->dump();
            SkDebugf(",\n");
        }
        SkDebugf("};\n");
    }

    SkString purgeableName("nullptr");
    if (fNumPurgeable) {
        purgeableName.printf("gPurgeable-%s", fName.c_str());

        SkDebugf("const GrTextureCacheInfo gPurgeable-%s[%d] = {\n",
                 purgeableName.c_str(),
                 fNumPurgeable);
        for (int i = 0; i < fNumPurgeable; ++i) {
            this->purgeableResource(i)->dump();
            SkDebugf(",\n");
        }
        SkDebugf("};\n");
    }

    SkDebugf("GrCacheState state{ \"%s\", %d, %s, %zu, %d, %s, %zu };\n",
             fName.c_str(),
             fNumNonPurgeable,
             nonPurgeableName.c_str(),
             fNonPurgeableBytes,
             fNumPurgeable,
             purgeableName.c_str(),
             fPurgeableBytes);
}
