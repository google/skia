/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"
#include "include/gpu/d3d/GrD3DTypes.h"

#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DUtil.h"

GrD3DCaps::GrD3DCaps(const GrContextOptions& contextOptions, IDXGIAdapter1* adapter,
                     ID3D12Device* device)
        : INHERITED(contextOptions) {
    /**************************************************************************
     * GrCaps fields
     **************************************************************************/
    fMipMapSupport = true;   // always available in Direct3D
    fNPOTTextureTileSupport = true;  // available in feature level 10_0 and up
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out
    fInstanceAttribSupport = true;

    // TODO: implement these
    fSemaphoreSupport = false;
    fFenceSyncSupport = false;
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = false;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;

    // TODO: implement these
    fTransferFromBufferToTextureSupport = false;
    fTransferFromSurfaceToBufferSupport = false;

    fMaxRenderTargetSize = 16384;  // minimum required by feature level 11_0
    fMaxTextureSize = 16384;       // minimum required by feature level 11_0

    // TODO: implement
    fDynamicStateArrayGeometryProcessorTextureSupport = false;

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->init(contextOptions, adapter, device);
}

bool GrD3DCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    return false;
}

void GrD3DCaps::init(const GrContextOptions& contextOptions, IDXGIAdapter1* adapter,
                     ID3D12Device* device) {
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1,
    };
    D3D12_FEATURE_DATA_FEATURE_LEVELS flDesc = {};
    flDesc.NumFeatureLevels = _countof(featureLevels);
    flDesc.pFeatureLevelsRequested = featureLevels;
    SkDEBUGCODE(HRESULT hr =) device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &flDesc,
                                                          sizeof(flDesc));
    SkASSERT(SUCCEEDED(hr));
    // This had better be true
    SkASSERT(flDesc.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);

    DXGI_ADAPTER_DESC adapterDesc;
    SkDEBUGCODE(hr =) adapter->GetDesc(&adapterDesc);
    SkASSERT(SUCCEEDED(hr));

    D3D12_FEATURE_DATA_D3D12_OPTIONS optionsDesc;
    SkDEBUGCODE(hr =) device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &optionsDesc,
                                     sizeof(optionsDesc));
    SkASSERT(SUCCEEDED(hr));

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2Desc;
    SkDEBUGCODE(hr =) device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &options2Desc,
                                     sizeof(options2Desc));
    SkASSERT(SUCCEEDED(hr));

    // See https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    if (D3D12_RESOURCE_BINDING_TIER_1 == optionsDesc.ResourceBindingTier) {
        fMaxPerStageShaderResourceViews = 128;
        if (D3D_FEATURE_LEVEL_11_0 == flDesc.MaxSupportedFeatureLevel) {
            fMaxPerStageUnorderedAccessViews = 8;
        } else {
            fMaxPerStageUnorderedAccessViews = 64;
        }
    } else {
        // The doc above says "full heap", but practically it seems like it should be
        // limited by the maximum number of samplers in a heap
        fMaxPerStageUnorderedAccessViews = 2032;
        fMaxPerStageShaderResourceViews = 2032;
    }

    this->initGrCaps(optionsDesc, options2Desc);
    this->initShaderCaps(adapterDesc.VendorId, optionsDesc);

    this->initFormatTable(adapterDesc, device);
    // TODO: set up stencil

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(adapterDesc.VendorId);
    }

    this->finishInitialization(contextOptions);
}

void GrD3DCaps::initGrCaps(const D3D12_FEATURE_DATA_D3D12_OPTIONS& optionsDesc,
                           const D3D12_FEATURE_DATA_D3D12_OPTIONS2& options2Desc) {
    // There doesn't seem to be a property for this, and setting it to MAXINT makes tests which test
    // all the vertex attribs time out looping over that many. For now, we'll cap this at 64 max and
    // can raise it if we ever find that need.
    fMaxVertexAttributes = 64;

    // TODO: we can set locations but not sure if we can query them
    fSampleLocationsSupport = false;

    if (D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED !=
            options2Desc.ProgrammableSamplePositionsTier) {
        // We "disable" multisample by colocating all samples at pixel center.
        fMultisampleDisableSupport = true;
    }

    // TODO: It's not clear if this is supported or not.
    fMixedSamplesSupport = false;

    if (D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED !=
            optionsDesc.ConservativeRasterizationTier) {
        fConservativeRasterSupport = true;
    }

    fWireframeSupport = true;

    // Feature level 11_0 and up support up to 16K in texture dimension
    fMaxTextureSize = 16384;
    // There's no specific cap for RT size, so use texture size
    fMaxRenderTargetSize = fMaxTextureSize;
    if (fDriverBugWorkarounds.max_texture_size_limit_4096) {
        fMaxTextureSize = std::min(fMaxTextureSize, 4096);
    }
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = fMaxTextureSize;

    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;

    // Assuming since we will always map in the end to upload the data we might as well just map
    // from the get go. There is no hard data to suggest this is faster or slower.
    fBufferMapThreshold = 0;

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag | kAsyncRead_MapFlag;

    fOversizedStencilSupport = true;

    // Advanced blend modes don't appear to be supported.
}

void GrD3DCaps::initShaderCaps(int vendorID, const D3D12_FEATURE_DATA_D3D12_OPTIONS& optionsDesc) {
    GrShaderCaps* shaderCaps = fShaderCaps.get();
    shaderCaps->fVersionDeclString = "#version 330\n";

    // Shader Model 5 supports all of the following:
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;
    // Flat interpolation appears to be slow on Qualcomm GPUs. This was tested in GL and is assumed
    // to be true with D3D as well.
    shaderCaps->fPreferFlatInterpolation = kQualcomm_D3DVendor != vendorID;

    shaderCaps->fSampleMaskSupport = true;

    shaderCaps->fShaderDerivativeSupport = true;

    shaderCaps->fGeometryShaderSupport = shaderCaps->fGSInvocationsSupport = true;

    shaderCaps->fDualSourceBlendingSupport = true;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fVertexIDSupport = true;
    shaderCaps->fFPManipulationSupport = true;

    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits =
        D3D12_SHADER_MIN_PRECISION_SUPPORT_NONE == optionsDesc.MinPrecisionSupport;

    // See https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    // The maximum number of samplers in a shader-visible descriptor heap is 2048, but
    // 16 of those are reserved for the driver.
    shaderCaps->fMaxFragmentSamplers =
        (D3D12_RESOURCE_BINDING_TIER_1 == optionsDesc.ResourceBindingTier) ? 16 : 2032;
}

void GrD3DCaps::applyDriverCorrectnessWorkarounds(int vendorID) {
    // Nothing yet.
}

// These are all the valid DXGI_FORMATs that we support in Skia. They are roughly ordered from most
// frequently used to least to improve look up times in arrays.
static constexpr DXGI_FORMAT kDxgiFormats[] = {
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_B4G4R4A4_UNORM,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16_FLOAT
};

void GrD3DCaps::setColorType(GrColorType colorType, std::initializer_list<DXGI_FORMAT> formats) {
#ifdef SK_DEBUG
    for (size_t i = 0; i < kNumDxgiFormats; ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType &&
                !SkToBool(ctInfo.fFlags & ColorTypeInfo::kWrappedOnly_Flag)) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kDxgiFormats[i] == *it) {
                        found = true;
                    }
                }
                SkASSERT(found);
            }
        }
    }
#endif
    int idx = static_cast<int>(colorType);
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        const auto& info = this->getFormatInfo(*it);
        for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
            if (info.fColorTypeInfos[i].fColorType == colorType) {
                fColorTypeToFormatTable[idx] = *it;
                return;
            }
        }
    }
}

const GrD3DCaps::FormatInfo& GrD3DCaps::getFormatInfo(DXGI_FORMAT format) const {
    GrD3DCaps* nonConstThis = const_cast<GrD3DCaps*>(this);
    return nonConstThis->getFormatInfo(format);
}

GrD3DCaps::FormatInfo& GrD3DCaps::getFormatInfo(DXGI_FORMAT format) {
    static_assert(SK_ARRAY_COUNT(kDxgiFormats) == GrD3DCaps::kNumDxgiFormats,
                  "Size of DXGI_FORMATs array must match static value in header");
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDxgiFormats); ++i) {
        if (kDxgiFormats[i] == format) {
            return fFormatTable[i];
        }
    }
    static FormatInfo kInvalidFormat;
    return kInvalidFormat;
}

void GrD3DCaps::initFormatTable(const DXGI_ADAPTER_DESC& adapterDesc, ID3D12Device* device) {
    static_assert(SK_ARRAY_COUNT(kDxgiFormats) == GrD3DCaps::kNumDxgiFormats,
                  "Size of DXGI_FORMATs array must match static value in header");

    std::fill_n(fColorTypeToFormatTable, kGrColorTypeCnt, DXGI_FORMAT_UNKNOWN);

    // Go through all the formats and init their support surface and data GrColorTypes.
    // Format: DXGI_FORMAT_R8G8B8A8_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: DXGI_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rgb1");
            }
        }
    }

    // Format: DXGI_FORMAT_R8_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 1;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                constexpr GrColorType ct = GrColorType::kAlpha_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrrr");
                ctInfo.fWriteSwizzle = GrSwizzle("aaaa");
            }
            // Format: DXGI_FORMAT_R8_UNORM, Surface: kGray_8
            {
                constexpr GrColorType ct = GrColorType::kGray_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrr1");
            }
        }
    }
    // Format: DXGI_FORMAT_B8G8R8A8_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                constexpr GrColorType ct = GrColorType::kBGRA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_B5G6R5_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_B5G6R5_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_B5G6R5_UNORM, Surface: kBGR_565
            {
                constexpr GrColorType ct = GrColorType::kBGR_565;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R16G16B16A16_FLOAT
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 8;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16G16B16A16_FLOAT, Surface: GrColorType::kRGBA_F16
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: DXGI_FORMAT_R16G16B16A16_FLOAT, Surface: GrColorType::kRGBA_F16_Clamped
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16_Clamped;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R16_FLOAT
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16_FLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16_FLOAT, Surface: kAlpha_F16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrrr");
                ctInfo.fWriteSwizzle = GrSwizzle("aaaa");
            }
        }
    }
    // Format: DXGI_FORMAT_R8G8_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R8G8_UNORM, Surface: kRG_88
            {
                constexpr GrColorType ct = GrColorType::kRG_88;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R10G10B10A2_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R10G10B10A2_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R10G10B10A2_UNORM, Surface: kRGBA_1010102
            {
                constexpr GrColorType ct = GrColorType::kRGBA_1010102;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_B4G4R4A4_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_B4G4R4A4_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_B4G4R4A4_UNORM, Surface: kABGR_4444
            {
                constexpr GrColorType ct = GrColorType::kABGR_4444;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("bgra");
                ctInfo.fWriteSwizzle = GrSwizzle("bgra");
            }
        }
    }
    // Format: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888_SRGB;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R16_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16_UNORM, Surface: kAlpha_16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrrr");
                ctInfo.fWriteSwizzle = GrSwizzle("aaaa");
            }
        }
    }
    // Format: DXGI_FORMAT_R16G16_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                constexpr GrColorType ct = GrColorType::kRG_1616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R16G16B16A16_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 8;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr GrColorType ct = GrColorType::kRGBA_16161616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: DXGI_FORMAT_R16G16_FLOAT
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: DXGI_FORMAT_R16G16_FLOAT, Surface: kRG_F16
            {
                constexpr GrColorType ct = GrColorType::kRG_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }

    // Format: DXGI_FORMAT_BC1_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_BC1_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fBytesPerPixel = 0;
        // No supported GrColorTypes.
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map GrColorTypes (used for creating GrSurfaces) to DXGI_FORMATs. The order in which the
    // formats are passed into the setColorType function indicates the priority in selecting which
    // format we use for a given GrcolorType.

    this->setColorType(GrColorType::kAlpha_8, { DXGI_FORMAT_R8_UNORM });
    this->setColorType(GrColorType::kBGR_565, { DXGI_FORMAT_B5G6R5_UNORM });
    this->setColorType(GrColorType::kABGR_4444, { DXGI_FORMAT_B4G4R4A4_UNORM });
    this->setColorType(GrColorType::kRGBA_8888, { DXGI_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(GrColorType::kRGBA_8888_SRGB, { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB });
    this->setColorType(GrColorType::kRGB_888x, { DXGI_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(GrColorType::kRG_88, { DXGI_FORMAT_R8G8_UNORM });
    this->setColorType(GrColorType::kBGRA_8888, { DXGI_FORMAT_B8G8R8A8_UNORM });
    this->setColorType(GrColorType::kRGBA_1010102, { DXGI_FORMAT_R10G10B10A2_UNORM });
    this->setColorType(GrColorType::kGray_8, { DXGI_FORMAT_R8_UNORM });
    this->setColorType(GrColorType::kAlpha_F16, { DXGI_FORMAT_R16_FLOAT });
    this->setColorType(GrColorType::kRGBA_F16, { DXGI_FORMAT_R16G16B16A16_FLOAT });
    this->setColorType(GrColorType::kRGBA_F16_Clamped, { DXGI_FORMAT_R16G16B16A16_FLOAT });
    this->setColorType(GrColorType::kAlpha_16, { DXGI_FORMAT_R16_UNORM });
    this->setColorType(GrColorType::kRG_1616, { DXGI_FORMAT_R16G16_UNORM });
    this->setColorType(GrColorType::kRGBA_16161616, { DXGI_FORMAT_R16G16B16A16_UNORM });
    this->setColorType(GrColorType::kRG_F16, { DXGI_FORMAT_R16G16_FLOAT });
}

void GrD3DCaps::FormatInfo::InitFormatFlags(const D3D12_FEATURE_DATA_FORMAT_SUPPORT& formatSupport,
                                            uint16_t* flags) {
    if (SkToBool(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE & formatSupport.Support1)) {
        *flags = *flags | kTexturable_Flag;

        // Ganesh assumes that all renderable surfaces are also texturable
        if (SkToBool(D3D12_FORMAT_SUPPORT1_RENDER_TARGET & formatSupport.Support1) &&
            SkToBool(D3D12_FORMAT_SUPPORT1_BLENDABLE & formatSupport.Support1)) {
            *flags = *flags | kRenderable_Flag;
        }
    }

    if (SkToBool(D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET & formatSupport.Support1)) {
        *flags = *flags | kMSAA_Flag;
    }

    if (SkToBool(D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE & formatSupport.Support1)) {
        *flags = *flags | kResolve_Flag;
    }
}

static bool multisample_count_supported(ID3D12Device* device, DXGI_FORMAT format, int sampleCount) {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msqLevels;
    msqLevels.Format = format;
    msqLevels.SampleCount = sampleCount;
    SkDEBUGCODE(HRESULT hr =) device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                                          &msqLevels, sizeof(msqLevels));
    SkASSERT(SUCCEEDED(hr));

    return msqLevels.NumQualityLevels > 0;
}

void GrD3DCaps::FormatInfo::initSampleCounts(const DXGI_ADAPTER_DESC& adapterDesc,
                                             ID3D12Device* device, DXGI_FORMAT format) {
    if (multisample_count_supported(device, format, 1)) {
        fColorSampleCounts.push_back(1);
    }
    // TODO: test these
    //if (kImagination_D3DVendor == adapterDesc.VendorId) {
    //    // MSAA does not work on imagination
    //    return;
    //}
    //if (kIntel_D3DVendor == adapterDesc.VendorId) {
    //    // MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
    //    return;
    //}
    if (multisample_count_supported(device, format, 2)) {
        fColorSampleCounts.push_back(2);
    }
    if (multisample_count_supported(device, format, 4)) {
        fColorSampleCounts.push_back(4);
    }
    if (multisample_count_supported(device, format, 8)) {
        fColorSampleCounts.push_back(8);
    }
    if (multisample_count_supported(device, format, 16)) {
        fColorSampleCounts.push_back(16);
    }
    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
}

void GrD3DCaps::FormatInfo::init(const DXGI_ADAPTER_DESC& adapterDesc, ID3D12Device* device,
                                 DXGI_FORMAT format) {
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupportDesc;
    formatSupportDesc.Format = format;
    SkDEBUGCODE(HRESULT hr =) device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                                          &formatSupportDesc,
                                                          sizeof(formatSupportDesc));
    SkASSERT(SUCCEEDED(hr));

    InitFormatFlags(formatSupportDesc, &fFlags);
    if (fFlags & kRenderable_Flag) {
        this->initSampleCounts(adapterDesc, device, format);
    }
}

bool GrD3DCaps::isFormatSRGB(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return false;
    }

    switch (dxgiFormat) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return true;
        default:
            return false;
    }
}

SkImage::CompressionType GrD3DCaps::compressionType(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return SkImage::CompressionType::kNone;
    }

    switch (dxgiFormat) {
        case DXGI_FORMAT_BC1_UNORM:    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
        default:                       return SkImage::CompressionType::kNone;
    }

    SkUNREACHABLE;
}

bool GrD3DCaps::isFormatTexturable(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return false;
    }

    return this->isFormatTexturable(dxgiFormat);
}

bool GrD3DCaps::isFormatTexturable(DXGI_FORMAT format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & info.fFlags);
}

bool GrD3DCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                             int sampleCount) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return false;
    }
    if (!this->isFormatRenderable(dxgiFormat, sampleCount)) {
        return false;
    }
    const auto& info = this->getFormatInfo(dxgiFormat);
    if (!SkToBool(info.colorTypeFlags(ct) & ColorTypeInfo::kRenderable_Flag)) {
        return false;
    }
    return true;
}

bool GrD3DCaps::isFormatRenderable(const GrBackendFormat& format, int sampleCount) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return false;
    }
    return this->isFormatRenderable(dxgiFormat, sampleCount);
}

bool GrD3DCaps::isFormatRenderable(DXGI_FORMAT format, int sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

int GrD3DCaps::getRenderTargetSampleCount(int requestedCount,
                                         const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return 0;
    }

    return this->getRenderTargetSampleCount(requestedCount, dxgiFormat);
}

int GrD3DCaps::getRenderTargetSampleCount(int requestedCount, DXGI_FORMAT format) const {
    requestedCount = std::max(1, requestedCount);

    const FormatInfo& info = this->getFormatInfo(format);

    int count = info.fColorSampleCounts.count();

    if (!count) {
        return 0;
    }

    if (1 == requestedCount) {
        SkASSERT(info.fColorSampleCounts.count() && info.fColorSampleCounts[0] == 1);
        return 1;
    }

    for (int i = 0; i < count; ++i) {
        if (info.fColorSampleCounts[i] >= requestedCount) {
            return info.fColorSampleCounts[i];
        }
    }
    return 0;
}

int GrD3DCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return 0;
    }
    return this->maxRenderTargetSampleCount(dxgiFormat);
}

int GrD3DCaps::maxRenderTargetSampleCount(DXGI_FORMAT format) const {
    const FormatInfo& info = this->getFormatInfo(format);

    const auto& table = info.fColorSampleCounts;
    if (!table.count()) {
        return 0;
    }
    return table[table.count() - 1];
}

size_t GrD3DCaps::bytesPerPixel(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return 0;
    }
    return this->bytesPerPixel(dxgiFormat);
}

size_t GrD3DCaps::bytesPerPixel(DXGI_FORMAT format) const {
    return this->getFormatInfo(format).fBytesPerPixel;
}

GrCaps::SupportedWrite GrD3DCaps::supportedWritePixelsColorType(
        GrColorType surfaceColorType, const GrBackendFormat& surfaceFormat,
        GrColorType srcColorType) const {
    DXGI_FORMAT dxgiFormat;
    if (!surfaceFormat.asDxgiFormat(&dxgiFormat)) {
        return { GrColorType::kUnknown, 0 };
    }

    // TODO: this seems to be pretty constrictive, confirm
    // Any buffer data needs to be aligned to 512 bytes and that of a single texel.
    size_t offsetAlignment = GrAlignTo(this->bytesPerPixel(dxgiFormat),
                                       D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == surfaceColorType) {
            return { surfaceColorType, offsetAlignment };
        }
    }
    return { GrColorType::kUnknown, 0 };
}

GrCaps::SurfaceReadPixelsSupport GrD3DCaps::surfaceSupportsReadPixels(
        const GrSurface* surface) const {
    if (surface->isProtected()) {
        return SurfaceReadPixelsSupport::kUnsupported;
    }
    // TODO
    return SurfaceReadPixelsSupport::kUnsupported;
}

bool GrD3DCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numSamples() <= 1 && SkToBool(surface->asTexture());
    }
    return true;
}

bool GrD3DCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                  const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return false;
    }

    SkImage::CompressionType compression = GrDxgiFormatToCompressionType(dxgiFormat);
    if (compression != SkImage::CompressionType::kNone) {
        return ct == (SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                      : GrColorType::kRGBA_8888);
    }

    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
}

GrColorType GrD3DCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format,
                                                         bool isAlphaChannel) const {
    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return GrColorType::kUnknown;
    }

    switch (dxgiFormat) {
        case DXGI_FORMAT_R8_UNORM:                 return isAlphaChannel ? GrColorType::kAlpha_8
                                                                         : GrColorType::kGray_8;
        case DXGI_FORMAT_R8G8B8A8_UNORM:           return GrColorType::kRGBA_8888;
        case DXGI_FORMAT_R8G8_UNORM:               return GrColorType::kRG_88;
        case DXGI_FORMAT_B8G8R8A8_UNORM:           return GrColorType::kBGRA_8888;
        case DXGI_FORMAT_R10G10B10A2_UNORM:        return GrColorType::kRGBA_1010102;
        case DXGI_FORMAT_R16_UNORM:                return GrColorType::kAlpha_16;
        case DXGI_FORMAT_R16_FLOAT:                return GrColorType::kAlpha_F16;
        case DXGI_FORMAT_R16G16_UNORM:             return GrColorType::kRG_1616;
        case DXGI_FORMAT_R16G16B16A16_UNORM:       return GrColorType::kRGBA_16161616;
        case DXGI_FORMAT_R16G16_FLOAT:             return GrColorType::kRG_F16;
        default:                                   return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
}

GrBackendFormat GrD3DCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    DXGI_FORMAT format = this->getFormatFromColorType(ct);
    if (format == DXGI_FORMAT_UNKNOWN) {
        return {};
    }
    return GrBackendFormat::MakeDxgi(format);
}

GrBackendFormat GrD3DCaps::getBackendFormatFromCompressionType(
    SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::CompressionType::kBC1_RGBA8_UNORM:
            if (this->isFormatTexturable(DXGI_FORMAT_BC1_UNORM)) {
                return GrBackendFormat::MakeDxgi(DXGI_FORMAT_BC1_UNORM);
            }
            return {};
        default:
            return {};
    }

    SkUNREACHABLE;
}

GrSwizzle GrD3DCaps::getReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));
    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

GrSwizzle GrD3DCaps::getWriteSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));
    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fWriteSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

uint64_t GrD3DCaps::computeFormatKey(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));

    return (uint64_t)dxgiFormat;
}

GrCaps::SupportedRead GrD3DCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {
    // TODO
    return {GrColorType::kUnknown, 0};
}

void GrD3DCaps::addExtraSamplerKey(GrProcessorKeyBuilder* b,
                                   GrSamplerState samplerState,
                                   const GrBackendFormat& format) const {
    // TODO
}

/**
 * TODO: Determine what goes in the ProgramDesc
 */
GrProgramDesc GrD3DCaps::makeDesc(const GrRenderTarget* rt,
                                  const GrProgramInfo& programInfo) const {
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, rt, programInfo, *this)) {
        SkASSERT(!desc.isValid());
        return desc;
    }

    GrProcessorKeyBuilder b(&desc.key());

    // TODO: add D3D-specific information

    return desc;
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrD3DCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        {GrColorType::kAlpha_8,        GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8_UNORM)           },
        {GrColorType::kBGR_565,        GrBackendFormat::MakeDxgi(DXGI_FORMAT_B5G6R5_UNORM)       },
        {GrColorType::kABGR_4444,      GrBackendFormat::MakeDxgi(DXGI_FORMAT_B4G4R4A4_UNORM)     },
        {GrColorType::kRGBA_8888,      GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8G8B8A8_UNORM)     },
        {GrColorType::kRGBA_8888_SRGB, GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)},
        {GrColorType::kRGB_888x,       GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8G8B8A8_UNORM)     },
        {GrColorType::kRG_88,          GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8G8_UNORM)         },
        {GrColorType::kBGRA_8888,      GrBackendFormat::MakeDxgi(DXGI_FORMAT_B8G8R8A8_UNORM)     },
        {GrColorType::kRGBA_1010102,   GrBackendFormat::MakeDxgi(DXGI_FORMAT_R10G10B10A2_UNORM)  },
        {GrColorType::kGray_8,         GrBackendFormat::MakeDxgi(DXGI_FORMAT_R8_UNORM)           },
        {GrColorType::kAlpha_F16,      GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16_FLOAT)          },
        {GrColorType::kRGBA_F16,       GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16G16B16A16_FLOAT) },
        {GrColorType::kRGBA_F16_Clamped, GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16G16B16A16_FLOAT)},
        {GrColorType::kAlpha_16,       GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16_UNORM)          },
        {GrColorType::kRG_1616,        GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16G16_UNORM)       },
        {GrColorType::kRGBA_16161616,  GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16G16B16A16_UNORM) },
        {GrColorType::kRG_F16,         GrBackendFormat::MakeDxgi(DXGI_FORMAT_R16G16_FLOAT)       },
        {GrColorType::kRGBA_8888,      GrBackendFormat::MakeDxgi(DXGI_FORMAT_BC1_UNORM)          },
    };

    return combos;
}
#endif
