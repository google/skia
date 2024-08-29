/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrUtil_DEFINED
#define GrUtil_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"

#include <cstdint>

class GrStyle;
class SkMatrix;

enum GrIntelGpuFamily {
    kUnknown_IntelGpuFamily,

    // 6th gen
    kSandyBridge_IntelGpuFamily,

    // 7th gen
    kIvyBridge_IntelGpuFamily,
    kValleyView_IntelGpuFamily, // aka BayTrail
    kHaswell_IntelGpuFamily,

    // 8th gen
    kCherryView_IntelGpuFamily, // aka Braswell
    kBroadwell_IntelGpuFamily,

    // 9th gen
    kApolloLake_IntelGpuFamily,
    kSkyLake_IntelGpuFamily,
    kGeminiLake_IntelGpuFamily,
    kKabyLake_IntelGpuFamily,
    kCoffeeLake_IntelGpuFamily,

    // 11th gen
    kIceLake_IntelGpuFamily,
};

GrIntelGpuFamily GrGetIntelGpuFamily(uint32_t deviceID);

// Helper for determining if we can treat a thin stroke as a hairline w/ coverage.
// If we can, we draw lots faster (raster device does this same test).
bool GrIsStrokeHairlineOrEquivalent(const GrStyle&, const SkMatrix&, SkScalar* outCoverage);

static inline SkSLType SkSLCombinedSamplerTypeForTextureType(GrTextureType type) {
    switch (type) {
        case GrTextureType::k2D:
            return SkSLType::kTexture2DSampler;
        case GrTextureType::kRectangle:
            return SkSLType::kTexture2DRectSampler;
        case GrTextureType::kExternal:
            return SkSLType::kTextureExternalSampler;
        default:
            SK_ABORT("Unexpected texture type");
    }
}

static constexpr const char* GrBackendApiToStr(GrBackendApi api) {
    switch (api) {
        case GrBackendApi::kOpenGL:      return "OpenGL";
        case GrBackendApi::kVulkan:      return "Vulkan";
        case GrBackendApi::kMetal:       return "Metal";
        case GrBackendApi::kDirect3D:    return "Direct3D";
        case GrBackendApi::kMock:        return "Mock";
        case GrBackendApi::kUnsupported: return "Unsupported";
    }
    SkUNREACHABLE;
}

static constexpr const char* GrColorTypeToStr(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return "kUnknown";
        case GrColorType::kAlpha_8:          return "kAlpha_8";
        case GrColorType::kBGR_565:          return "kBGR_565";
        case GrColorType::kRGB_565:          return "kRGB_565";
        case GrColorType::kABGR_4444:        return "kABGR_4444";
        case GrColorType::kRGBA_8888:        return "kRGBA_8888";
        case GrColorType::kRGBA_8888_SRGB:   return "kRGBA_8888_SRGB";
        case GrColorType::kRGB_888x:         return "kRGB_888x";
        case GrColorType::kRG_88:            return "kRG_88";
        case GrColorType::kBGRA_8888:        return "kBGRA_8888";
        case GrColorType::kRGBA_1010102:     return "kRGBA_1010102";
        case GrColorType::kBGRA_1010102:     return "kBGRA_1010102";
        case GrColorType::kRGB_101010x:      return "kRGB_101010x";
        case GrColorType::kRGBA_10x6:        return "kBGRA_10x6";
        case GrColorType::kGray_8:           return "kGray_8";
        case GrColorType::kGrayAlpha_88:     return "kGrayAlpha_88";
        case GrColorType::kAlpha_F16:        return "kAlpha_F16";
        case GrColorType::kRGBA_F16:         return "kRGBA_F16";
        case GrColorType::kRGBA_F16_Clamped: return "kRGBA_F16_Clamped";
        case GrColorType::kRGB_F16F16F16x:   return "kRGB_F16F16F16x";
        case GrColorType::kRGBA_F32:         return "kRGBA_F32";
        case GrColorType::kAlpha_8xxx:       return "kAlpha_8xxx";
        case GrColorType::kAlpha_F32xxx:     return "kAlpha_F32xxx";
        case GrColorType::kGray_8xxx:        return "kGray_8xxx";
        case GrColorType::kR_8xxx:           return "kR_8xxx";
        case GrColorType::kAlpha_16:         return "kAlpha_16";
        case GrColorType::kRG_1616:          return "kRG_1616";
        case GrColorType::kRGBA_16161616:    return "kRGBA_16161616";
        case GrColorType::kRG_F16:           return "kRG_F16";
        case GrColorType::kRGB_888:          return "kRGB_888";
        case GrColorType::kR_8:              return "kR_8";
        case GrColorType::kR_16:             return "kR_16";
        case GrColorType::kR_F16:            return "kR_F16";
        case GrColorType::kGray_F16:         return "kGray_F16";
        case GrColorType::kARGB_4444:        return "kARGB_4444";
        case GrColorType::kBGRA_4444:        return "kBGRA_4444";
    }
    SkUNREACHABLE;
}

static constexpr const char* GrSurfaceOriginToStr(GrSurfaceOrigin origin) {
    switch (origin) {
        case kTopLeft_GrSurfaceOrigin:    return "kTopLeft";
        case kBottomLeft_GrSurfaceOrigin: return "kBottomLeft";
    }
    SkUNREACHABLE;
}

#endif
