
/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCaps_DEFINED
#define GrD3DCaps_DEFINED

#include "src/gpu/GrCaps.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DAttachment.h"

/**
 * Stores some capabilities of a D3D backend.
 */
class GrD3DCaps : public GrCaps {
public:
    /**
     * Creates a GrD3DCaps that is set such that nothing is supported. The init function should
     * be called to fill out the caps.
     */
    GrD3DCaps(const GrContextOptions& contextOptions, IDXGIAdapter1*, ID3D12Device*);

    bool isFormatSRGB(const GrBackendFormat&) const override;

    bool isFormatTexturable(const GrBackendFormat&, GrTextureType) const override;
    bool isFormatTexturable(DXGI_FORMAT) const;

    bool isFormatCopyable(const GrBackendFormat&) const override { return true; }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;
    bool isFormatRenderable(DXGI_FORMAT, int sampleCount) const;

    bool isFormatUnorderedAccessible(DXGI_FORMAT) const;

    int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, DXGI_FORMAT) const;

    int maxRenderTargetSampleCount(const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(DXGI_FORMAT) const;

    GrColorType getFormatColorType(DXGI_FORMAT) const;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    /**
     * Returns both a supported and most preferred stencil format to use in draws.
     */
    DXGI_FORMAT preferredStencilFormat() const {
        return fPreferredStencilFormat;
    }
    static int GetStencilFormatTotalBitCount(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return 32;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            // DXGI_FORMAT_D32_FLOAT_S8X24_UINT has 24 unused bits at the end so total bits is 64.
            return 64;
        default:
            SkASSERT(false);
            return 0;
        }
    }

    /**
     * Helpers used by canCopySurface. In all cases if the SampleCnt parameter is zero that means
     * the surface is not a render target, otherwise it is the number of samples in the render
     * target.
     */
    bool canCopyTexture(DXGI_FORMAT dstFormat, int dstSampleCnt,
                        DXGI_FORMAT srcFormat, int srcSamplecnt) const;

    bool canCopyAsResolve(DXGI_FORMAT dstFormat, int dstSampleCnt,
                          DXGI_FORMAT srcFormat, int srcSamplecnt) const;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    DXGI_FORMAT getFormatFromColorType(GrColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    void addExtraSamplerKey(GrProcessorKeyBuilder*,
                            GrSamplerState,
                            const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;

    bool resolveSubresourceRegionSupport() const { return fResolveSubresourceRegionSupport; }
    bool standardSwizzleLayoutSupport() const { return fStandardSwizzleLayoutSupport; }

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    enum D3DVendor {
        kAMD_D3DVendor = 0x1002,
        kARM_D3DVendor = 0x13B5,
        kImagination_D3DVendor = 0x1010,
        kIntel_D3DVendor = 0x8086,
        kNVIDIA_D3DVendor = 0x10DE,
        kQualcomm_D3DVendor = 0x5143,
    };

    void init(const GrContextOptions& contextOptions, IDXGIAdapter1*, ID3D12Device*);

    void initGrCaps(const D3D12_FEATURE_DATA_D3D12_OPTIONS&,
                    ID3D12Device*);
    void initShaderCaps(int vendorID, const D3D12_FEATURE_DATA_D3D12_OPTIONS& optionsDesc);

    void initFormatTable(const DXGI_ADAPTER_DESC&, ID3D12Device*);
    void initStencilFormat(ID3D12Device*);

    void applyDriverCorrectnessWorkarounds(int vendorID);

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
            // Indicates that this colorType is supported only if we are wrapping a texture with
            // the given format and colorType. We do not allow creation with this pair.
            kWrappedOnly_Flag = 0x4,
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

        void init(const DXGI_ADAPTER_DESC&, ID3D12Device*, DXGI_FORMAT);
        static void InitFormatFlags(const D3D12_FEATURE_DATA_FORMAT_SUPPORT&, uint16_t* flags);
        void initSampleCounts(const DXGI_ADAPTER_DESC& adapterDesc, ID3D12Device*, DXGI_FORMAT);

        enum {
            kTexturable_Flag = 0x1, // Can be sampled in a shader
            kRenderable_Flag = 0x2, // Rendertarget and blendable
            kMSAA_Flag = 0x4,
            kResolve_Flag = 0x8,
            kUnorderedAccess_Flag = 0x10,
        };

        uint16_t fFlags = 0;

        SkTDArray<int> fColorSampleCounts;

        // This GrColorType represents how the actually GPU format lays out its memory. This is used
        // for uploading data to backend textures to make sure we've arranged the memory in the
        // correct order.
        GrColorType fFormatColorType = GrColorType::kUnknown;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    static const size_t kNumDxgiFormats = 15;
    FormatInfo fFormatTable[kNumDxgiFormats];

    FormatInfo& getFormatInfo(DXGI_FORMAT);
    const FormatInfo& getFormatInfo(DXGI_FORMAT) const;

    DXGI_FORMAT fColorTypeToFormatTable[kGrColorTypeCnt];
    void setColorType(GrColorType, std::initializer_list<DXGI_FORMAT> formats);

    int fMaxPerStageShaderResourceViews;
    int fMaxPerStageUnorderedAccessViews;

    DXGI_FORMAT fPreferredStencilFormat;

    bool fResolveSubresourceRegionSupport : 1;
    bool fStandardSwizzleLayoutSupport : 1;

    using INHERITED = GrCaps;
};

#endif
