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

GrD3DCaps::GrD3DCaps(const GrContextOptions& contextOptions, ID3D12Device* device)
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

    this->init(contextOptions, device);
}

bool GrD3DCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    return false;
}

void GrD3DCaps::init(const GrContextOptions& contextOptions, ID3D12Device* device) {
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
    SkASSERT(flDesc.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);

    this->initGrCaps(device);
    this->initShaderCaps(device);

    // TODO: set up formats and stencil

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(device);
    }

    this->finishInitialization(contextOptions);
}

void GrD3DCaps::initGrCaps(ID3D12Device*) {

}

void GrD3DCaps::initShaderCaps(ID3D12Device*) {

}

void GrD3DCaps::applyDriverCorrectnessWorkarounds(ID3D12Device*) {
    // None so far
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
