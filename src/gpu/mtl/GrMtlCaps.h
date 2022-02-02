/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCaps_DEFINED
#define GrMtlCaps_DEFINED

#include "include/private/SkTDArray.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/mtl/GrMtlAttachment.h"

#import <Metal/Metal.h>

class GrMtlRenderTarget;

/**
 * Stores some capabilities of a Mtl backend.
 */
class GrMtlCaps : public GrCaps {
public:
    GrMtlCaps(const GrContextOptions& contextOptions, id<MTLDevice> device);

    bool isFormatSRGB(const GrBackendFormat&) const override;

    bool isFormatTexturable(const GrBackendFormat&, GrTextureType) const override;
    bool isFormatTexturable(MTLPixelFormat) const;

    bool isFormatCopyable(const GrBackendFormat&) const override { return true; }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;
    bool isFormatRenderable(MTLPixelFormat, int sampleCount) const;

    int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, MTLPixelFormat) const;

    int maxRenderTargetSampleCount(const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(MTLPixelFormat) const;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    DstCopyRestrictions getDstCopyRestrictions(const GrRenderTargetProxy* src,
                                               GrColorType ct) const override;

    /**
     * Returns both a supported and most prefered stencil format to use in draws.
     */
    MTLPixelFormat preferredStencilFormat() const {
        return fPreferredStencilFormat;
    }

    bool canCopyAsBlit(MTLPixelFormat dstFormat, int dstSampleCount,
                       MTLPixelFormat srcFormat, int srcSampleCount,
                       const SkIRect& srcRect, const SkIPoint& dstPoint,
                       bool areDstSrcSameObj) const;

    bool canCopyAsResolve(MTLPixelFormat dstFormat, int dstSampleCount,
                          MTLPixelFormat srcFormat, int srcSampleCount,
                          bool srcIsRenderTarget, const SkISize srcDimensions,
                          const SkIRect& srcRect,
                          const SkIPoint& dstPoint,
                          bool areDstSrcSameObj) const;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    MTLPixelFormat getFormatFromColorType(GrColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;
    MTLPixelFormat getStencilPixelFormat(const GrProgramDesc& desc);

    bool isMac() const { return fGPUFamily == GPUFamily::kMac; }
    bool isApple() const { return fGPUFamily == GPUFamily::kApple; }

    size_t getMinBufferAlignment() const { return this->isMac() ? 4 : 1; }

    // if true, MTLStoreActionStoreAndMultiplesampleResolve is available
    bool storeAndMultisampleResolveSupport() const { return fStoreAndMultisampleResolveSupport; }

    // If true when doing MSAA draws, we will prefer to discard the MSAA attachment on load
    // and stores. The use of this feature for specific draws depends on the render target having a
    // resolve attachment, and if we need to load previous data the resolve attachment must
    // be readable in a shader. Otherwise we will just write out and store the MSAA attachment
    // like normal.
    bool preferDiscardableMSAAAttachment() const { return fPreferDiscardableMSAAAttachment; }
    bool renderTargetSupportsDiscardableMSAA(const GrMtlRenderTarget*) const;

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif
    void onDumpJSON(SkJSONWriter*) const override;

private:
    void initGPUFamily(id<MTLDevice> device);

    void initStencilFormat(id<MTLDevice> device);

    void initGrCaps(id<MTLDevice> device);
    void initShaderCaps();

    void applyDriverCorrectnessWorkarounds(const GrContextOptions&, const id<MTLDevice>);

    void initFormatTable();

    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    GrBackendFormat onGetDefaultBackendFormat(GrColorType) const override;
    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

    GrSwizzle onGetReadSwizzle(const GrBackendFormat&, GrColorType) const override;

    // ColorTypeInfo for a specific format
    struct ColorTypeInfo {
        GrColorType fColorType = GrColorType::kUnknown;
        enum {
            kUploadData_Flag = 0x1,
            // Does Ganesh itself support rendering to this colorType & format pair. Renderability
            // still additionally depends on if the format itself is renderable.
            kRenderable_Flag = 0x2,
        };
        uint32_t fFlags = 0;

        GrSwizzle fReadSwizzle;
        GrSwizzle fWriteSwizzle;
    };

    struct FormatInfo {
        uint32_t colorTypeFlags(GrColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        enum {
            kTexturable_Flag  = 0x1,
            kRenderable_Flag  = 0x2, // Color attachment and blendable
            kMSAA_Flag        = 0x4,
            kResolve_Flag     = 0x8,
        };
        static const uint16_t kAllFlags = kTexturable_Flag | kRenderable_Flag |
                                          kMSAA_Flag | kResolve_Flag;

        uint16_t fFlags = 0;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
#ifdef SK_BUILD_FOR_IOS
    inline static constexpr size_t kNumMtlFormats = 17;
#else
    inline static constexpr size_t kNumMtlFormats = 16;
#endif
    static size_t GetFormatIndex(MTLPixelFormat);
    FormatInfo fFormatTable[kNumMtlFormats];

    const FormatInfo& getFormatInfo(const MTLPixelFormat pixelFormat) const {
        size_t index = GetFormatIndex(pixelFormat);
        return fFormatTable[index];
    }

    MTLPixelFormat fColorTypeToFormatTable[kGrColorTypeCnt];
    void setColorType(GrColorType, std::initializer_list<MTLPixelFormat> formats);

    enum class GPUFamily {
        kMac,
        kApple,
    };
    bool getGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group);
    bool getGPUFamilyFromFeatureSet(id<MTLDevice> device, GrMtlCaps::GPUFamily* gpuFamily,
                                    int* group);

    GPUFamily fGPUFamily;
    int fFamilyGroup;

    SkTDArray<int> fSampleCounts;

    MTLPixelFormat fPreferredStencilFormat;

    bool fStoreAndMultisampleResolveSupport : 1;
    bool fPreferDiscardableMSAAAttachment : 1;

    using INHERITED = GrCaps;
};

#endif
