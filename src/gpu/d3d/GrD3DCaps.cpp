/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"
#include "include/gpu/d3d/GrD3DTypes.h"

#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DTexture.h"
#include "src/gpu/d3d/GrD3DUtil.h"

GrD3DCaps::GrD3DCaps(const GrContextOptions& contextOptions, IDXGIAdapter1* adapter,
                     ID3D12Device* device)
        : INHERITED(contextOptions) {
    /**************************************************************************
     * GrCaps fields
     **************************************************************************/
    fMipmapSupport = true;   // always available in Direct3D
    fNPOTTextureTileSupport = true;  // available in feature level 10_0 and up
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out
    fDrawInstancedSupport = true;
    fNativeDrawIndirectSupport = true;

    fSemaphoreSupport = true;
    fFenceSyncSupport = true;
    // TODO: implement these
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = false;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;
    fTransferPixelsToRowBytesSupport = true;

    fTransferFromBufferToTextureSupport = true;
    fTransferFromSurfaceToBufferSupport = true;

    fMaxRenderTargetSize = 16384;  // minimum required by feature level 11_0
    fMaxTextureSize = 16384;       // minimum required by feature level 11_0

    fTransferBufferAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

    // TODO: implement
    fDynamicStateArrayGeometryProcessorTextureSupport = false;

    fShaderCaps = std::make_unique<GrShaderCaps>();

    this->init(contextOptions, adapter, device);
}

bool GrD3DCaps::canCopyTexture(DXGI_FORMAT dstFormat, int dstSampleCnt,
                               DXGI_FORMAT srcFormat, int srcSampleCnt) const {
    if ((dstSampleCnt > 1 || srcSampleCnt > 1) && dstSampleCnt != srcSampleCnt) {
        return false;
    }

    // D3D allows us to copy within the same format family but doesn't do conversions
    // so we require strict identity.
    return srcFormat == dstFormat;
}

bool GrD3DCaps::canCopyAsResolve(DXGI_FORMAT dstFormat, int dstSampleCnt,
                                 DXGI_FORMAT srcFormat, int srcSampleCnt) const {
    // The src surface must be multisampled.
    if (srcSampleCnt <= 1) {
        return false;
    }

    // The dst must not be multisampled.
    if (dstSampleCnt > 1) {
        return false;
    }

    // Surfaces must have the same format.
    // D3D12 can resolve between typeless and non-typeless formats, but we are not using
    // typeless formats. It's not possible to resolve within the same format family otherwise.
    if (srcFormat != dstFormat) {
        return false;
    }

    return true;
}

bool GrD3DCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    if (src->isProtected() == GrProtected::kYes && dst->isProtected() != GrProtected::kYes) {
        return false;
    }

    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        dstSampleCnt = rtProxy->numSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        srcSampleCnt = rtProxy->numSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    DXGI_FORMAT dstFormat, srcFormat;
    SkAssertResult(dst->backendFormat().asDxgiFormat(&dstFormat));
    SkAssertResult(src->backendFormat().asDxgiFormat(&srcFormat));

    return this->canCopyTexture(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt) ||
           this->canCopyAsResolve(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt);
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
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &flDesc,
                                                     sizeof(flDesc)));
    // This had better be true
    SkASSERT(flDesc.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);

    DXGI_ADAPTER_DESC adapterDesc;
    GR_D3D_CALL_ERRCHECK(adapter->GetDesc(&adapterDesc));

    D3D12_FEATURE_DATA_D3D12_OPTIONS optionsDesc;
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &optionsDesc,
                                                     sizeof(optionsDesc)));


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

    fStandardSwizzleLayoutSupport = (optionsDesc.StandardSwizzle64KBSupported);

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 optionsDesc2;
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &optionsDesc2,
                                                     sizeof(optionsDesc2)));
    fResolveSubresourceRegionSupport = (optionsDesc2.ProgrammableSamplePositionsTier !=
                                        D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED);

    this->initGrCaps(optionsDesc, device);
    this->initShaderCaps(adapterDesc.VendorId, optionsDesc);

    this->initFormatTable(adapterDesc, device);
    this->initStencilFormat(device);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(adapterDesc.VendorId);
    }

    this->finishInitialization(contextOptions);
}

void GrD3DCaps::initGrCaps(const D3D12_FEATURE_DATA_D3D12_OPTIONS& optionsDesc,
                           ID3D12Device* device) {
    // We assume a minimum of Shader Model 5.1, which allows at most 32 vertex inputs.
    fMaxVertexAttributes = 32;

    // Can use standard sample locations
    fSampleLocationsSupport = true;

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

    fTwoSidedStencilRefsAndMasksMustMatch = true;

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

    shaderCaps->fDualSourceBlendingSupport = true;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    // TODO(skia:12352) HLSL does not expose asinh/acosh/atanh
    shaderCaps->fInverseHyperbolicSupport = false;
    shaderCaps->fVertexIDSupport = true;
    shaderCaps->fInfinitySupport = true;
    shaderCaps->fNonconstantArrayIndexSupport = true;
    shaderCaps->fBitManipulationSupport = true;

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


bool stencil_format_supported(ID3D12Device* device, DXGI_FORMAT format) {
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupportDesc;
    formatSupportDesc.Format = format;
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                                     &formatSupportDesc,
                                                     sizeof(formatSupportDesc)));
    return SkToBool(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL & formatSupportDesc.Support1);
}

void GrD3DCaps::initStencilFormat(ID3D12Device* device) {
    if (stencil_format_supported(device, DXGI_FORMAT_D24_UNORM_S8_UINT)) {
        fPreferredStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    } else {
        SkASSERT(stencil_format_supported(device, DXGI_FORMAT_D32_FLOAT_S8X24_UINT));
        fPreferredStencilFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    }
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
        info.fFormatColorType = GrColorType::kRGBA_8888;
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
        info.fFormatColorType = GrColorType::kR_8;
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
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
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
        info.fFormatColorType = GrColorType::kBGRA_8888;
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
        info.fFormatColorType = GrColorType::kBGR_565;
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
        info.fFormatColorType = GrColorType::kRGBA_F16;
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
        info.fFormatColorType = GrColorType::kR_F16;
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
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
            }
        }
    }
    // Format: DXGI_FORMAT_R8G8_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fFormatColorType = GrColorType::kRG_88;
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
        info.fFormatColorType = GrColorType::kRGBA_1010102;
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
        info.fFormatColorType = GrColorType::kBGRA_4444;
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
                ctInfo.fReadSwizzle = GrSwizzle("argb");
                ctInfo.fWriteSwizzle = GrSwizzle("gbar");
            }
        }
    }
    // Format: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fFormatColorType = GrColorType::kRGBA_8888_SRGB;
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
        info.fFormatColorType = GrColorType::kR_16;
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
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
            }
        }
    }
    // Format: DXGI_FORMAT_R16G16_UNORM
    {
        constexpr DXGI_FORMAT format = DXGI_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(adapterDesc, device, format);
        info.fFormatColorType = GrColorType::kRG_1616;
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
        info.fFormatColorType = GrColorType::kRGBA_16161616;
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
        info.fFormatColorType = GrColorType::kRG_F16;
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

    if (SkToBool(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW & formatSupport.Support1)) {
        *flags = *flags | kUnorderedAccess_Flag;
    }
}

static bool multisample_count_supported(ID3D12Device* device, DXGI_FORMAT format, int sampleCount) {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msqLevels;
    msqLevels.Format = format;
    msqLevels.SampleCount = sampleCount;
    msqLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                                     &msqLevels, sizeof(msqLevels)));

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
    GR_D3D_CALL_ERRCHECK(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                                     &formatSupportDesc,
                                                     sizeof(formatSupportDesc)));

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

bool GrD3DCaps::isFormatTexturable(const GrBackendFormat& format, GrTextureType) const {
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

bool GrD3DCaps::isFormatUnorderedAccessible(DXGI_FORMAT format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kUnorderedAccess_Flag & info.fFlags);
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

GrColorType GrD3DCaps::getFormatColorType(DXGI_FORMAT format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    return info.fFormatColorType;
}

GrCaps::SupportedWrite GrD3DCaps::supportedWritePixelsColorType(
        GrColorType surfaceColorType, const GrBackendFormat& surfaceFormat,
        GrColorType srcColorType) const {
    DXGI_FORMAT dxgiFormat;
    if (!surfaceFormat.asDxgiFormat(&dxgiFormat)) {
        return { GrColorType::kUnknown, 0 };
    }

    // Any buffer data needs to be aligned to 512 bytes and that of a single texel.
    size_t offsetAlignment = SkAlignTo(GrDxgiFormatBytesPerBlock(dxgiFormat),
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
    if (auto tex = static_cast<const GrD3DTexture*>(surface->asTexture())) {
        // We can't directly read from a compressed format
        if (GrDxgiFormatIsCompressed(tex->dxgiFormat())) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        return SurfaceReadPixelsSupport::kSupported;
    } else if (auto rt = static_cast<const GrD3DRenderTarget*>(surface->asRenderTarget())) {
        if (rt->numSamples() > 1) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        return SurfaceReadPixelsSupport::kSupported;
    }
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

    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
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

GrSwizzle GrD3DCaps::onGetReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));
    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)dxgiFormat);
    return {};
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
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)dxgiFormat);
    return {};
}

uint64_t GrD3DCaps::computeFormatKey(const GrBackendFormat& format) const {
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));

    return (uint64_t)dxgiFormat;
}

GrCaps::SupportedRead GrD3DCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {
    DXGI_FORMAT dxgiFormat;
    if (!srcBackendFormat.asDxgiFormat(&dxgiFormat)) {
        return { GrColorType::kUnknown, 0 };
    }

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(srcBackendFormat);
    if (compression != SkImage::CompressionType::kNone) {
        return { SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                        : GrColorType::kRGBA_8888, 0 };
    }

    // Any subresource buffer data offset we copy to needs to be aligned to 512 bytes.
    size_t offsetAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

    const auto& info = this->getFormatInfo(dxgiFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return { srcColorType, offsetAlignment };
        }
    }
    return { GrColorType::kUnknown, 0 };
}

void GrD3DCaps::addExtraSamplerKey(skgpu::KeyBuilder* b,
                                   GrSamplerState samplerState,
                                   const GrBackendFormat& format) const {
    // TODO
}

/**
 * TODO: Determine what goes in the ProgramDesc
 */
GrProgramDesc GrD3DCaps::makeDesc(GrRenderTarget* rt,
                                  const GrProgramInfo& programInfo,
                                  ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);

    skgpu::KeyBuilder b(desc.key());

    GrD3DRenderTarget* d3dRT = (GrD3DRenderTarget*) rt;
    d3dRT->genKey(&b);

    GrStencilSettings stencil = programInfo.nonGLStencilSettings();
    stencil.genKey(&b, false);

    programInfo.pipeline().genKey(&b, *this);
    // The num samples is already added in the render target key so we don't need to add it here.

    // D3D requires the full primitive type as part of its key
    b.add32(programInfo.primitiveTypeKey());

    b.flush();
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
