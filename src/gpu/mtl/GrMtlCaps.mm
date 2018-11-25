/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlCaps.h"

#include "GrShaderCaps.h"

GrMtlCaps::GrMtlCaps(const GrContextOptions& contextOptions, const id<MTLDevice> device,
                     MTLFeatureSet featureSet)
        : INHERITED(contextOptions) {
    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->initFeatureSet(featureSet);
    this->initGrCaps(device);
    this->initShaderCaps();
    this->initConfigTable();

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

void GrMtlCaps::initFeatureSet(MTLFeatureSet featureSet) {
    // Mac OSX
#ifdef SK_BUILD_FOR_MAC
    if (MTLFeatureSet_OSX_GPUFamily1_v2 == featureSet) {
        fPlatform = Platform::kMac;
        fFamilyGroup = 1;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_OSX_GPUFamily1_v1 == featureSet) {
        fPlatform = Platform::kMac;
        fFamilyGroup = 1;
        fVersion = 1;
        return;
    }
#endif

    // iOS Family group 3
#ifdef SK_BUILD_FOR_IOS
    if (MTLFeatureSet_iOS_GPUFamily3_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 3;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily3_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 3;
        fVersion = 1;
        return;
    }

    // iOS Family group 2
    if (MTLFeatureSet_iOS_GPUFamily2_v3 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 3;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily2_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily2_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 1;
        return;
    }

    // iOS Family group 1
    if (MTLFeatureSet_iOS_GPUFamily1_v3 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 3;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily1_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily1_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 1;
        return;
    }
#endif
    // No supported feature sets were found
    SK_ABORT("Requested an unsupported feature set");
}

void GrMtlCaps::initGrCaps(const id<MTLDevice> device) {
    // Max vertex attribs is the same on all devices
    fMaxVertexAttributes = 31;

    // RenderTarget and Texture size
    if (this->isMac()) {
        fMaxRenderTargetSize = 16384;
    } else {
        if (3 == fFamilyGroup) {
            fMaxRenderTargetSize = 16384;
        } else {
            // Family group 1 and 2 support 8192 for version 2 and above, 4096 for v1
            if (1 == fVersion) {
                fMaxRenderTargetSize = 4096;
            } else {
                fMaxRenderTargetSize = 8192;
            }
        }
    }
    fMaxTextureSize = fMaxRenderTargetSize;

    // Init sample counts. All devices support 1 (i.e. 0 in skia).
    fSampleCounts.push(0);
    for (auto sampleCnt : {2, 4, 8}) {
        if ([device supportsTextureSampleCount:sampleCnt]) {
            fSampleCounts.push(sampleCnt);
        }
    }

    // Starting with the assumption that there isn't a reason to not map small buffers.
    fBufferMapThreshold = 0;

    // Buffers are always fully mapped.
    fMapBufferFlags = kCanMap_MapFlag;

    fOversizedStencilSupport = true;

    // Looks like there is a field called rasterSampleCount labeled as beta in the Metal docs. This
    // may be what we eventually need here, but it has no description.
    fSampleShadingSupport = false;

    fSRGBSupport = true;   // always available in Metal
    fSRGBWriteControl = false;
    fMipMapSupport = true;   // always available in Metal
    fNPOTTextureTileSupport = true;  // always available in Metal
    fDiscardRenderTargetSupport = true;

    fReuseScratchTextures = true; // Assuming this okay

    fTextureBarrierSupport = false; // Need to figure out if we can do this

    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;

    if (this->isMac() || 3 == fFamilyGroup) {
        fInstanceAttribSupport = true;
    }

    fUsesMixedSamples = false;
    fGpuTracingSupport = false;

    fFenceSyncSupport = true;   // always available in Metal
    fCrossContextTextureSupport = false;
}

int GrMtlCaps::getSampleCount(int requestedCount, GrPixelConfig config) const {
    int count = fSampleCounts.count();
    SkASSERT(count > 0); // We always add 0 as a valid sample count
    if (!this->isConfigRenderable(config, true)) {
        return 0;
    }

    for (int i = 0; i < count; ++i) {
        if (fSampleCounts[i] >= requestedCount) {
            return fSampleCounts[i];
        }
    }
    return fSampleCounts[count-1];
}

void GrMtlCaps::initShaderCaps() {
    GrShaderCaps* shaderCaps = fShaderCaps.get();

    // fConfigOutputSwizzle will default to RGBA so we only need to set it for alpha only config.
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        if (GrPixelConfigIsAlphaOnly(config)) {
            shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRR();
            shaderCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
        } else {
            if (kGray_8_GrPixelConfig == config) {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRA();
            } else {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGBA();
            }
        }
    }

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;
    // We haven't yet tested that using flat attributes perform well.
    shaderCaps->fPreferFlatInterpolation = true;

    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fGeometryShaderSupport = false;

    if ((this->isMac() && fVersion >= 2) ||
        (this->isIOS() && ((1 == fFamilyGroup && 4 == fVersion) ||
                           (2 == fFamilyGroup && 4 == fVersion) ||
                           (3 == fFamilyGroup && 3 == fVersion)))) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }

    if (this->isIOS()) {
        shaderCaps->fFBFetchSupport = true;
        shaderCaps->fFBFetchNeedsCustomOutput = true; // ??
        shaderCaps->fFBFetchColorName = ""; // Somehow add [[color(0)]] to arguments to frag shader
    }
    shaderCaps->fDstReadInShaderSupport = shaderCaps->fFBFetchSupport;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fTexelBufferSupport = false;
    shaderCaps->fTexelFetchSupport = false;
    shaderCaps->fVertexIDSupport = false;
    shaderCaps->fImageLoadStoreSupport = false;

    // Metal uses IEEE float and half floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    shaderCaps->fMaxVertexSamplers =
    shaderCaps->fMaxFragmentSamplers = 16;
    // For now just cap at the per stage max. If we hit this limit we can come back to adjust this
    shaderCaps->fMaxCombinedSamplers = shaderCaps->fMaxVertexSamplers;
}

void GrMtlCaps::initConfigTable() {
    ConfigInfo* info;
    // Alpha_8 uses R8Unorm
    info = &fConfigTable[kAlpha_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // Gray_8 uses R8Unorm
    info = &fConfigTable[kGray_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGB_565 uses B5G6R5Unorm, even though written opposite this format packs how we want
    info = &fConfigTable[kRGB_565_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = 0;
    } else {
        info->fFlags = ConfigInfo::kAllFlags;
    }

    // RGBA_4444 uses ABGR4Unorm
    info = &fConfigTable[kRGBA_4444_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = 0;
    } else {
        info->fFlags = ConfigInfo::kAllFlags;
    }

    // RGBA_8888 uses RGBA8Unorm
    info = &fConfigTable[kRGBA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // BGRA_8888 uses BGRA8Unorm
    info = &fConfigTable[kBGRA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // SRGBA_8888 uses RGBA8Unorm_sRGB
    info = &fConfigTable[kSRGBA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // SBGRA_8888 uses BGRA8Unorm_sRGB
    info = &fConfigTable[kSBGRA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGBA_8888_sint uses RGBA8Sint
    info = &fConfigTable[kRGBA_8888_sint_GrPixelConfig];
    info->fFlags = ConfigInfo::kMSAA_Flag;

    // RGBA_float uses RGBA32Float
    info = &fConfigTable[kRGBA_float_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = 0;
    }

    // RG_float uses RG32Float
    info = &fConfigTable[kRG_float_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kRenderable_Flag;
    }

    // Alpha_half uses R16Float
    info = &fConfigTable[kAlpha_half_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGBA_half uses RGBA16Float
    info = &fConfigTable[kRGBA_half_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;
}
