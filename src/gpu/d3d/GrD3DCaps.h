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

class GrShaderCaps;

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
    SkImage::CompressionType compressionType(const GrBackendFormat&) const override;

    bool isFormatTexturable(const GrBackendFormat&) const override;
    bool isFormatTexturable(DXGI_FORMAT) const;

    bool isFormatCopyable(const GrBackendFormat&) const override { return false; }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;
    bool isFormatRenderable(DXGI_FORMAT, int sampleCount) const;

    int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, DXGI_FORMAT) const;

    int maxRenderTargetSampleCount(const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(DXGI_FORMAT) const;

    size_t bytesPerPixel(const GrBackendFormat&) const override;
    size_t bytesPerPixel(DXGI_FORMAT) const;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat&,
                                                  bool isAlphaChannel) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    DXGI_FORMAT getFormatFromColorType(GrColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    GrSwizzle getReadSwizzle(const GrBackendFormat&, GrColorType) const override;
    GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    void addExtraSamplerKey(GrProcessorKeyBuilder*,
                            GrSamplerState,
                            const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(const GrRenderTarget*, const GrProgramInfo&) const override;

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
                    const D3D12_FEATURE_DATA_D3D12_OPTIONS2&);
    void initShaderCaps(int vendorID, const D3D12_FEATURE_DATA_D3D12_OPTIONS& optionsDesc);

    void initFormatTable(const DXGI_ADAPTER_DESC&, ID3D12Device*);

    void applyDriverCorrectnessWorkarounds(int vendorID);

    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    GrBackendFormat onGetDefaultBackendFormat(GrColorType) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

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
        };

        uint16_t fFlags = 0;

        SkTDArray<int> fColorSampleCounts;
        // This value is only valid for regular formats. Compressed formats will be 0.
        size_t fBytesPerPixel = 0;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    static const size_t kNumDxgiFormats = 16;
    FormatInfo fFormatTable[kNumDxgiFormats];

    FormatInfo& getFormatInfo(DXGI_FORMAT);
    const FormatInfo& getFormatInfo(DXGI_FORMAT) const;

    DXGI_FORMAT fColorTypeToFormatTable[kGrColorTypeCnt];
    void setColorType(GrColorType, std::initializer_list<DXGI_FORMAT> formats);

    int fMaxPerStageShaderResourceViews;
    int fMaxPerStageUnorderedAccessViews;

    typedef GrCaps INHERITED;
};

#endif
