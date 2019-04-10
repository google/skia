/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrSurface.h"
#include "GrSurfaceProxy.h"
#include "GrTypesPriv.h"
#include "GrWindowRectangles.h"
#include "SkJSONWriter.h"

GrCaps::GrCaps(const GrContextOptions& options) {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fSRGBSupport = false;
    fSRGBWriteControl = false;
    fDiscardRenderTargetSupport = false;
    fReuseScratchTextures = true;
    fReuseScratchBuffers = true;
    fGpuTracingSupport = false;
    fOversizedStencilSupport = false;
    fTextureBarrierSupport = false;
    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;
    fInstanceAttribSupport = false;
    fUsesMixedSamples = false;
    fUsePrimitiveRestart = false;
    fPreferClientSideDynamicBuffers = false;
    fPreferFullscreenClears = false;
    fMustClearUploadedBufferData = false;
    fShouldInitializeTextures = false;
    fSupportsAHardwareBufferImages = false;
    fFenceSyncSupport = false;
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = false;
    fDynamicStateArrayGeometryProcessorTextureSupport = false;
    fPerformPartialClearsAsDraws = false;
    fPerformColorClearsAsDraws = false;
    fPerformStencilClearsAsDraws = false;
    fAllowCoverageCounting = false;
    fTransferBufferSupport = false;
    fDriverBlacklistCCPR = false;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fAdvBlendEqBlacklist = 0;

    fMapBufferFlags = kNone_MapFlags;

    fMaxVertexAttributes = 0;
    fMaxRenderTargetSize = 1;
    fMaxPreferredRenderTargetSize = 1;
    fMaxTextureSize = 1;
    fMaxWindowRectangles = 0;

    // An default count of 4 was chosen because of the common pattern in Blink of:
    //   isect RR
    //   diff  RR
    //   isect convex_poly
    //   isect convex_poly
    // when drawing rounded div borders.
    fMaxClipAnalyticFPs = 4;

    fSuppressPrints = options.fSuppressPrints;
#if GR_TEST_UTILS
    fWireframeMode = options.fWireframeMode;
#else
    fWireframeMode = false;
#endif
    fBufferMapThreshold = options.fBufferMapThreshold;
    fAvoidStencilBuffers = false;
    fAvoidWritePixelsFastPath = false;

    fPreferVRAMUseOverFlushes = true;

    fPreferTrianglesOverSampleMask = false;

    // Default to true, allow older versions of OpenGL to disable explicitly
    fClampToBorderSupport = true;

    fDriverBugWorkarounds = options.fDriverBugWorkarounds;
}

void GrCaps::applyOptionsOverrides(const GrContextOptions& options) {
    this->onApplyOptionsOverrides(options);
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(!fDriverBlacklistCCPR);
        SkASSERT(!fAvoidStencilBuffers);
        SkASSERT(!fAdvBlendEqBlacklist);
        SkASSERT(!fPerformColorClearsAsDraws);
        SkASSERT(!fPerformStencilClearsAsDraws);
        // Don't check the partial-clear workaround, since that is a backend limitation, not a
        // driver workaround (it just so happens the fallbacks are the same).
    }
    if (GrContextOptions::Enable::kNo == options.fUseDrawInsteadOfClear) {
        fPerformColorClearsAsDraws = false;
        fPerformStencilClearsAsDraws = false;
    } else if (GrContextOptions::Enable::kYes == options.fUseDrawInsteadOfClear) {
        fPerformColorClearsAsDraws = true;
        fPerformStencilClearsAsDraws = true;
    }

    fAllowCoverageCounting = !options.fDisableCoverageCountingPaths;

    fMaxTextureSize = SkTMin(fMaxTextureSize, options.fMaxTextureSizeOverride);
    fMaxTileSize = fMaxTextureSize;
#if GR_TEST_UTILS
    // If the max tile override is zero, it means we should use the max texture size.
    if (options.fMaxTileSizeOverride && options.fMaxTileSizeOverride < fMaxTextureSize) {
        fMaxTileSize = options.fMaxTileSizeOverride;
    }
    if (options.fSuppressGeometryShaders) {
        fShaderCaps->fGeometryShaderSupport = false;
    }
#endif
    if (fMaxWindowRectangles > GrWindowRectangles::kMaxWindows) {
        SkDebugf("WARNING: capping window rectangles at %i. HW advertises support for %i.\n",
                 GrWindowRectangles::kMaxWindows, fMaxWindowRectangles);
        fMaxWindowRectangles = GrWindowRectangles::kMaxWindows;
    }
    fAvoidStencilBuffers = options.fAvoidStencilBuffers;

    fDriverBugWorkarounds.applyOverrides(options.fDriverBugWorkarounds);
}


#ifdef SK_ENABLE_DUMP_GPU
static const char* pixel_config_name(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig: return "Unknown";
        case kAlpha_8_GrPixelConfig: return "Alpha8";
        case kAlpha_8_as_Alpha_GrPixelConfig: return "Alpha8_asAlpha";
        case kAlpha_8_as_Red_GrPixelConfig: return "Alpha8_asRed";
        case kGray_8_GrPixelConfig: return "Gray8";
        case kGray_8_as_Lum_GrPixelConfig: return "Gray8_asLum";
        case kGray_8_as_Red_GrPixelConfig: return "Gray8_asRed";
        case kRGB_565_GrPixelConfig: return "RGB565";
        case kRGBA_4444_GrPixelConfig: return "RGBA444";
        case kRGBA_8888_GrPixelConfig: return "RGBA8888";
        case kRGB_888_GrPixelConfig: return "RGB888";
        case kRGB_888X_GrPixelConfig: return "RGB888X";
        case kRG_88_GrPixelConfig: return "RG88";
        case kBGRA_8888_GrPixelConfig: return "BGRA8888";
        case kSRGBA_8888_GrPixelConfig: return "SRGBA8888";
        case kSBGRA_8888_GrPixelConfig: return "SBGRA8888";
        case kRGBA_1010102_GrPixelConfig: return "RGBA1010102";
        case kRGBA_float_GrPixelConfig: return "RGBAFloat";
        case kRG_float_GrPixelConfig: return "RGFloat";
        case kAlpha_half_GrPixelConfig: return "AlphaHalf";
        case kAlpha_half_as_Red_GrPixelConfig: return "AlphaHalf_asRed";
        case kRGBA_half_GrPixelConfig: return "RGBAHalf";
        case kRGBA_half_Clamped_GrPixelConfig: return "RGBAHalfClamped";
        case kRGB_ETC1_GrPixelConfig: return "RGBETC1";
    }
    SK_ABORT("Invalid pixel config");
    return "<invalid>";
}

static SkString map_flags_to_string(uint32_t flags) {
    SkString str;
    if (GrCaps::kNone_MapFlags == flags) {
        str = "none";
    } else {
        SkASSERT(GrCaps::kCanMap_MapFlag & flags);
        SkDEBUGCODE(flags &= ~GrCaps::kCanMap_MapFlag);
        str = "can_map";

        if (GrCaps::kSubset_MapFlag & flags) {
            str.append(" partial");
        } else {
            str.append(" full");
        }
        SkDEBUGCODE(flags &= ~GrCaps::kSubset_MapFlag);
    }
    SkASSERT(0 == flags); // Make sure we handled all the flags.
    return str;
}

void GrCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("MIP Map Support", fMipMapSupport);
    writer->appendBool("NPOT Texture Tile Support", fNPOTTextureTileSupport);
    writer->appendBool("sRGB Support", fSRGBSupport);
    writer->appendBool("sRGB Write Control", fSRGBWriteControl);
    writer->appendBool("Discard Render Target Support", fDiscardRenderTargetSupport);
    writer->appendBool("Reuse Scratch Textures", fReuseScratchTextures);
    writer->appendBool("Reuse Scratch Buffers", fReuseScratchBuffers);
    writer->appendBool("Gpu Tracing Support", fGpuTracingSupport);
    writer->appendBool("Oversized Stencil Support", fOversizedStencilSupport);
    writer->appendBool("Texture Barrier Support", fTextureBarrierSupport);
    writer->appendBool("Sample Locations Support", fSampleLocationsSupport);
    writer->appendBool("Multisample disable support", fMultisampleDisableSupport);
    writer->appendBool("Instance Attrib Support", fInstanceAttribSupport);
    writer->appendBool("Uses Mixed Samples", fUsesMixedSamples);
    writer->appendBool("Use primitive restart", fUsePrimitiveRestart);
    writer->appendBool("Prefer client-side dynamic buffers", fPreferClientSideDynamicBuffers);
    writer->appendBool("Prefer fullscreen clears", fPreferFullscreenClears);
    writer->appendBool("Must clear buffer memory", fMustClearUploadedBufferData);
    writer->appendBool("Should initialize textures", fShouldInitializeTextures);
    writer->appendBool("Supports importing AHardwareBuffers", fSupportsAHardwareBufferImages);
    writer->appendBool("Fence sync support", fFenceSyncSupport);
    writer->appendBool("Cross context texture support", fCrossContextTextureSupport);
    writer->appendBool("Half float vertex attribute support", fHalfFloatVertexAttributeSupport);
    writer->appendBool("Specify GeometryProcessor textures as a dynamic state array",
                       fDynamicStateArrayGeometryProcessorTextureSupport);
    writer->appendBool("Use draws for partial clears", fPerformPartialClearsAsDraws);
    writer->appendBool("Use draws for color clears", fPerformColorClearsAsDraws);
    writer->appendBool("Use draws for stencil clip clears", fPerformStencilClearsAsDraws);
    writer->appendBool("Allow coverage counting shortcuts", fAllowCoverageCounting);
    writer->appendBool("Supports transfer buffers", fTransferBufferSupport);
    writer->appendBool("Blacklist CCPR on current driver [workaround]", fDriverBlacklistCCPR);
    writer->appendBool("Clamp-to-border", fClampToBorderSupport);

    writer->appendBool("Prefer VRAM Use over flushes [workaround]", fPreferVRAMUseOverFlushes);
    writer->appendBool("Prefer more triangles over sample mask [MSAA only]",
                       fPreferTrianglesOverSampleMask);
    writer->appendBool("Avoid stencil buffers [workaround]", fAvoidStencilBuffers);

    if (this->advancedBlendEquationSupport()) {
        writer->appendHexU32("Advanced Blend Equation Blacklist", fAdvBlendEqBlacklist);
    }

    writer->appendS32("Max Vertex Attributes", fMaxVertexAttributes);
    writer->appendS32("Max Texture Size", fMaxTextureSize);
    writer->appendS32("Max Render Target Size", fMaxRenderTargetSize);
    writer->appendS32("Max Preferred Render Target Size", fMaxPreferredRenderTargetSize);
    writer->appendS32("Max Window Rectangles", fMaxWindowRectangles);
    writer->appendS32("Max Clip Analytic Fragment Processors", fMaxClipAnalyticFPs);

    static const char* kBlendEquationSupportNames[] = {
        "Basic",
        "Advanced",
        "Advanced Coherent",
    };
    GR_STATIC_ASSERT(0 == kBasic_BlendEquationSupport);
    GR_STATIC_ASSERT(1 == kAdvanced_BlendEquationSupport);
    GR_STATIC_ASSERT(2 == kAdvancedCoherent_BlendEquationSupport);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kBlendEquationSupportNames) == kLast_BlendEquationSupport + 1);

    writer->appendString("Blend Equation Support",
                         kBlendEquationSupportNames[fBlendEquationSupport]);
    writer->appendString("Map Buffer Support", map_flags_to_string(fMapBufferFlags).c_str());

    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig));
    SkASSERT(!this->isConfigTexturable(kUnknown_GrPixelConfig));

    writer->beginArray("configs");

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        writer->beginObject(nullptr, false);
        writer->appendString("name", pixel_config_name(config));
        writer->appendS32("max sample count", this->maxRenderTargetSampleCount(config));
        writer->appendBool("texturable", this->isConfigTexturable(config));
        writer->endObject();
    }

    writer->endArray();

    this->onDumpJSON(writer);

    writer->appendName("shaderCaps");
    this->shaderCaps()->dumpJSON(writer);

    writer->endObject();
}
#else
void GrCaps::dumpJSON(SkJSONWriter* writer) const { }
#endif

bool GrCaps::surfaceSupportsWritePixels(const GrSurface* surface) const {
    return surface->readOnly() ? false : this->onSurfaceSupportsWritePixels(surface);
}

size_t GrCaps::transferFromOffsetAlignment(GrColorType bufferColorType) const {
    if (!this->transferBufferSupport()) {
        return 0;
    }
    size_t result = this->onTransferFromOffsetAlignment(bufferColorType);
    if (!result) {
        return 0;
    }
    // It's very convenient to access 1 byte-per-channel 32 bitvRGB/RGBA color types as uint32_t.
    // Make those aligned reads out of the buffer even if the underlying API doesn't require it.
    auto componentFlags = GrColorTypeComponentFlags(bufferColorType);
    if ((componentFlags == kRGBA_SkColorTypeComponentFlags ||
         componentFlags == kRGB_SkColorTypeComponentFlags) &&
        GrColorTypeBytesPerPixel(bufferColorType) == 4) {
        switch (result & 0b11) {
            // offset alignment already a multiple of 4
            case 0:     return result;
            // offset alignment is a multiple of 2 but not 4.
            case 2:     return 2 * result;
            // offset alignment is not a multiple of 2.
            default:    return 4 * result;
        }
    }
    return result;
}

bool GrCaps::canCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                            const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    return dst->readOnly() ? false : this->onCanCopySurface(dst, src, srcRect, dstPoint);
}

bool GrCaps::validateSurfaceDesc(const GrSurfaceDesc& desc, GrMipMapped mipped) const {
    if (!this->isConfigTexturable(desc.fConfig)) {
        return false;
    }

    if (GrMipMapped::kYes == mipped && !this->mipMapSupport()) {
        return false;
    }

    if (desc.fWidth < 1 || desc.fHeight < 1) {
        return false;
    }

    if (SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag)) {
        if (0 == this->getRenderTargetSampleCount(desc.fSampleCnt, desc.fConfig)) {
            return false;
        }
        int maxRTSize = this->maxRenderTargetSize();
        if (desc.fWidth > maxRTSize || desc.fHeight > maxRTSize) {
            return false;
        }
    } else {
        // We currently do not support multisampled textures
        if (desc.fSampleCnt > 1) {
            return false;
        }
        int maxSize = this->maxTextureSize();
        if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
            return false;
        }
    }

    return true;
}

GrBackendFormat GrCaps::getBackendFormatFromColorType(SkColorType ct) const {
    return this->getBackendFormatFromGrColorType(SkColorTypeToGrColorType(ct), GrSRGBEncoded::kNo);
}
