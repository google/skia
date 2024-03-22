/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockCaps_DEFINED
#define GrMockCaps_DEFINED

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

class GrProgramInfo;
class GrRenderTarget;
namespace GrTest { struct TestFormatColorTypeCombination; }
struct GrContextOptions;
struct SkIRect;

class GrMockCaps : public GrCaps {
public:
    GrMockCaps(const GrContextOptions& contextOptions, const GrMockOptions& options)
            : INHERITED(contextOptions), fOptions(options) {
        fMipmapSupport = options.fMipmapSupport;
        fDrawInstancedSupport = options.fDrawInstancedSupport;
        fHalfFloatVertexAttributeSupport = options.fHalfFloatVertexAttributeSupport;
        fMapBufferFlags = options.fMapBufferFlags;
        fBufferMapThreshold = SK_MaxS32; // Overridable in GrContextOptions.
        fMaxTextureSize = options.fMaxTextureSize;
        fMaxWindowRectangles = options.fMaxWindowRectangles;
        fMaxRenderTargetSize = std::min(options.fMaxRenderTargetSize, fMaxTextureSize);
        fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;
        fMaxVertexAttributes = options.fMaxVertexAttributes;
        fSampleLocationsSupport = true;
        fSupportsProtectedContent = true;

        fShaderCaps = std::make_unique<GrShaderCaps>();
        fShaderCaps->fIntegerSupport = options.fIntegerSupport;
        fShaderCaps->fFlatInterpolationSupport = options.fFlatInterpolationSupport;
        fShaderCaps->fMaxFragmentSamplers = options.fMaxFragmentSamplers;
        fShaderCaps->fShaderDerivativeSupport = options.fShaderDerivativeSupport;
        fShaderCaps->fDualSourceBlendingSupport = options.fDualSourceBlendingSupport;
        fShaderCaps->fSampleMaskSupport = true;

        this->finishInitialization(contextOptions);
    }

    bool isFormatSRGB(const GrBackendFormat& format) const override {
        SkTextureCompressionType compression = format.asMockCompressionType();
        if (compression != SkTextureCompressionType::kNone) {
            return false;
        }

        auto ct = format.asMockColorType();
        return GrGetColorTypeDesc(ct).encoding() == GrColorTypeEncoding::kSRGBUnorm;
    }

    bool isFormatTexturable(const GrBackendFormat& format, GrTextureType) const override {
        SkTextureCompressionType compression = format.asMockCompressionType();
        if (compression != SkTextureCompressionType::kNone) {
            return fOptions.fCompressedOptions[(int)compression].fTexturable;
        }

        auto index = static_cast<int>(format.asMockColorType());
        return fOptions.fConfigOptions[index].fTexturable;
    }

    bool isFormatCopyable(const GrBackendFormat& format) const override {
        return false;
    }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override {
        // Currently we don't allow RGB_888X to be renderable because we don't have a way to
        // handle blends that reference dst alpha when the values in the dst alpha channel are
        // uninitialized.
        if (ct == GrColorType::kRGB_888x) {
            return false;
        }
        return this->isFormatRenderable(format, sampleCount);
    }

    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override {
        if (format.asMockCompressionType() != SkTextureCompressionType::kNone) {
            return false;  // compressed formats are never renderable
        }

        return sampleCount <= this->maxRenderTargetSampleCount(format.asMockColorType());
    }

    int getRenderTargetSampleCount(int requestCount, GrColorType) const;

    int getRenderTargetSampleCount(int requestCount,
                                   const GrBackendFormat& format) const override {
        SkTextureCompressionType compression = format.asMockCompressionType();
        if (compression != SkTextureCompressionType::kNone) {
            return 0; // no compressed format is renderable
        }

        return this->getRenderTargetSampleCount(requestCount, format.asMockColorType());
    }

    int maxRenderTargetSampleCount(GrColorType ct) const {
        switch (fOptions.fConfigOptions[(int)ct].fRenderability) {
            case GrMockOptions::ConfigOptions::Renderability::kNo:
                return 0;
            case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
                return 1;
            case GrMockOptions::ConfigOptions::Renderability::kMSAA:
                return kMaxSampleCnt;
        }
        return 0;
    }

    int maxRenderTargetSampleCount(const GrBackendFormat& format) const override {
        SkTextureCompressionType compression = format.asMockCompressionType();
        if (compression != SkTextureCompressionType::kNone) {
            return 0; // no compressed format is renderable
        }

        return this->maxRenderTargetSampleCount(format.asMockColorType());
    }

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override {
        return {surfaceColorType, 1};
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface* surface) const override {
        return surface->isProtected() ? SurfaceReadPixelsSupport::kUnsupported
                                      : SurfaceReadPixelsSupport::kSupported;
    }

    GrBackendFormat getBackendFormatFromCompressionType(SkTextureCompressionType) const override {
        return {};
    }

    skgpu::Swizzle getWriteSwizzle(const GrBackendFormat& format, GrColorType ct) const override {
        SkASSERT(this->areColorTypeAndFormatCompatible(ct, format));
        return skgpu::Swizzle("rgba");
    }

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;

#if defined(GR_TEST_UTILS)
    std::vector<GrTest::TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    bool onSurfaceSupportsWritePixels(const GrSurface*) const override { return true; }
    bool onCanCopySurface(const GrSurfaceProxy* dst, const SkIRect& dstRect,
                          const GrSurfaceProxy* src, const SkIRect& srcRect) const override {
        if (src->isProtected() == GrProtected::kYes && dst->isProtected() != GrProtected::kYes) {
            return false;
        }
        return true;
    }
    GrBackendFormat onGetDefaultBackendFormat(GrColorType ct) const override {
        return GrBackendFormat::MakeMock(ct, SkTextureCompressionType::kNone);
    }

    bool onAreColorTypeAndFormatCompatible(GrColorType ct,
                                           const GrBackendFormat& format) const override {
        if (ct == GrColorType::kUnknown) {
            return false;
        }

        SkTextureCompressionType compression = format.asMockCompressionType();
        if (compression == SkTextureCompressionType::kETC2_RGB8_UNORM ||
            compression == SkTextureCompressionType::kBC1_RGB8_UNORM) {
            return ct == GrColorType::kRGB_888x; // TODO: this may be too restrictive
        }
        if (compression == SkTextureCompressionType::kBC1_RGBA8_UNORM) {
            return ct == GrColorType::kRGBA_8888;
        }

        return ct == format.asMockColorType();
    }

    SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType, const GrBackendFormat&,
                                                 GrColorType) const override {
        return SupportedRead{srcColorType, 1};
    }

    skgpu::Swizzle onGetReadSwizzle(const GrBackendFormat& format, GrColorType ct) const override {
        SkASSERT(this->areColorTypeAndFormatCompatible(ct, format));
        return skgpu::Swizzle("rgba");
    }

    static const int kMaxSampleCnt = 16;

    GrMockOptions fOptions;
    using INHERITED = GrCaps;
};

#endif
