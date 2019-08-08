/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrHack.h"

#include "GrTestUtils.h"

#include "GrTexturePriv.h"

const char* GrPixelConfigToStr(GrPixelConfig config) {
    return "";
}

void GrTextureCacheInfo::dump() const {
    SkDebugf("{ %d, %d, %d, %s, %d, %d, %s, %d, %s, %s, %zu }",
             fRefCnt,
             fPendingReads,
             fPendingWrites,
             GrPixelConfigToStr(fConfig),
             fWidth,
             fHeight,
             fRenderable ? "GrRenderable::kYes" : "GrRenderable::kNo",
             fSampleCount,
             (GrMipMapped::kYes == fMipMapped) ? "GrMipMapped::kYes" : "GrMipMapped::kNo",
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
            const GrTextureCacheInfo* info = this->nonPurgeableResource(i);

            info->dump();
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
            const GrTextureCacheInfo* info = this->purgeableResource(i);

            info->dump();
            SkDebugf(",\n");
        }
        SkDebugf("};\n");
    }

    SkDebugf("GrCacheState state{ \"%s\", %d, %s, %zu, %d, %s, %zu }; // %zu\n",
             fName.c_str(),
             fNumNonPurgeable,
             nonPurgeableName.c_str(),
             fNonPurgeableBytes,
             fNumPurgeable,
             purgeableName.c_str(),
             fPurgeableBytes,
             fNonPurgeableBytes + fPurgeableBytes);
}
