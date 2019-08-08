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
    switch (config) {
        case kUnknown_GrPixelConfig:           return "Unknown";
        case kAlpha_8_GrPixelConfig:           return "Alpha8";
        case kAlpha_8_as_Alpha_GrPixelConfig:  return "Alpha8_asAlpha";
        case kAlpha_8_as_Red_GrPixelConfig:    return "Alpha8_asRed";
        case kGray_8_GrPixelConfig:            return "Gray8";
        case kGray_8_as_Lum_GrPixelConfig:     return "Gray8_asLum";
        case kGray_8_as_Red_GrPixelConfig:     return "Gray8_asRed";
        case kRGB_565_GrPixelConfig:           return "RGB565";
        case kRGBA_4444_GrPixelConfig:         return "RGBA444";
        case kRGBA_8888_GrPixelConfig:         return "RGBA8888";
        case kRGB_888_GrPixelConfig:           return "RGB888";
        case kRGB_888X_GrPixelConfig:          return "RGB888X";
        case kRG_88_GrPixelConfig:             return "RG88";
        case kBGRA_8888_GrPixelConfig:         return "BGRA8888";
        case kSRGBA_8888_GrPixelConfig:        return "SRGBA8888";
        case kRGBA_1010102_GrPixelConfig:      return "RGBA1010102";
        case kRGBA_float_GrPixelConfig:        return "RGBAFloat";
        case kAlpha_half_GrPixelConfig:        return "AlphaHalf";
        case kAlpha_half_as_Red_GrPixelConfig: return "AlphaHalf_asRed";
        case kRGBA_half_GrPixelConfig:         return "RGBAHalf";
        case kRGB_ETC1_GrPixelConfig:          return "RGBETC1";
        default:                               return "Unknown";
    }
}

void GrTextureCacheInfo::dump() const {
    SkDebugf("{ %d, %d, %d, %s, %d, %d, %s, %d, %s, %s, %s, %s, %zu }",
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
             fIsWrapped ? "true" : "false",
             (GrBudgetedType::kBudgeted == fIsBudgeted) ? "GrBudgetedType::kBudgeted" :
                (GrBudgetedType::kUnbudgetedUncacheable == fIsBudgeted) ? "GrBudgetedType::kUnbudgetedUncacheable"
                                                                        : "GrBudgetedType::kUnbudgetedCacheable",
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

        SkDebugf("const GrTextureCacheInfo %s[%d] = {\n",
                 purgeableName.c_str(),
                 fNumPurgeable);
        for (int i = 0; i < fNumPurgeable; ++i) {
            const GrTextureCacheInfo* info = this->purgeableResource(i);

            info->dump();
            SkDebugf(",\n");
        }
        SkDebugf("};\n");
    }

    SkDebugf("GrCacheState state{ \"%s\", %d, %s, %zu, %d, %s, %zu }; // %d %zu\n",
             fName.c_str(),
             fNumNonPurgeable,
             nonPurgeableName.c_str(),
             fNonPurgeableBytes,
             fNumPurgeable,
             purgeableName.c_str(),
             fPurgeableBytes,
             fNumNonPurgeable + fNumPurgeable,
             fNonPurgeableBytes + fPurgeableBytes);
}
