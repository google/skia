/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3D12.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"

#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DGpu.h"

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
    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &flDesc, sizeof(flDesc));
    SkASSERT(SUCCEEDED(hr));
    // This had better be true
    SkASSERT(flDesc.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);

    DXGI_ADAPTER_DESC adapterDesc;
    hr = adapter->GetDesc(&adapterDesc);
    SkASSERT(SUCCEEDED(hr));

    D3D12_FEATURE_DATA_D3D12_OPTIONS optionsDesc;
    hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &optionsDesc,
                                     sizeof(optionsDesc));
    SkASSERT(SUCCEEDED(hr));

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2Desc;
    hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &optionsDesc,
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

    // TODO: set up formats and stencil

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

bool GrD3DCaps::isFormatSRGB(const GrBackendFormat& format) const {
    // TODO
    return false;
}

SkImage::CompressionType GrD3DCaps::compressionType(const GrBackendFormat& format) const {
    // TODO
    return SkImage::CompressionType::kNone;
}

bool GrD3DCaps::isFormatTexturableAndUploadable(GrColorType ct,
                                                const GrBackendFormat& format) const {
    // TODO
    return false;
}

bool GrD3DCaps::isFormatTexturable(const GrBackendFormat& format) const {
    // TODO
    return false;
}

bool GrD3DCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                              int sampleCount) const {
    if (!this->isFormatRenderable(format, sampleCount)) {
        return false;
    }
    // TODO
    return false;
}

bool GrD3DCaps::isFormatRenderable(const GrBackendFormat& format, int sampleCount) const {
    // TODO
    return false;
}

int GrD3DCaps::getRenderTargetSampleCount(int requestedCount,
                                          const GrBackendFormat& format) const {
    // TODO
    return 0;
}

int GrD3DCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    // TODO
    return 0;
}

size_t GrD3DCaps::bytesPerPixel(const GrBackendFormat& format) const {
    // TODO
    return 0;
}

GrCaps::SupportedWrite GrD3DCaps::supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                                const GrBackendFormat& surfaceFormat,
                                                                GrColorType srcColorType) const {
    // TODO
    return {GrColorType::kUnknown, 0};
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
    // TODO
    return false;
}

bool GrD3DCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                  const GrBackendFormat& format) const {
    // TODO
    return false;
}

GrColorType GrD3DCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format,
                                                         bool isAlphaChannel) const {
    // TODO
    return GrColorType::kUnknown;
}

GrBackendFormat GrD3DCaps::onGetDefaultBackendFormat(GrColorType ct,
                                                     GrRenderable renderable) const {
    // TODO
    return GrBackendFormat();
}

GrBackendFormat GrD3DCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    // TODO
    return {};
}

GrSwizzle GrD3DCaps::getReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    // TODO
    return GrSwizzle::RGBA();
}

GrSwizzle GrD3DCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    // TODO
    return GrSwizzle::RGBA();
}

uint64_t GrD3DCaps::computeFormatKey(const GrBackendFormat& format) const {
    // TODO
    return (uint64_t)0;
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
        // TODO: fill in combos
    };

    return combos;
}
#endif
