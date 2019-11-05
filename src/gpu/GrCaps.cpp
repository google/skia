/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrSurface.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrWindowRectangles.h"
#include "src/utils/SkJSONWriter.h"

GrCaps::GrCaps(const GrContextOptions& options) {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fReuseScratchTextures = true;
    fReuseScratchBuffers = true;
    fGpuTracingSupport = false;
    fOversizedStencilSupport = false;
    fTextureBarrierSupport = false;
    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;
    fInstanceAttribSupport = false;
    fMixedSamplesSupport = false;
    fMSAAResolvesAutomatically = false;
    fUsePrimitiveRestart = false;
    fPreferClientSideDynamicBuffers = false;
    fPreferFullscreenClears = false;
    fMustClearUploadedBufferData = false;
    fShouldInitializeTextures = false;
    fSupportsAHardwareBufferImages = false;
    fFenceSyncSupport = false;
    fSemaphoreSupport = false;
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = false;
    fDynamicStateArrayGeometryProcessorTextureSupport = false;
    fPerformPartialClearsAsDraws = false;
    fPerformColorClearsAsDraws = false;
    fPerformStencilClearsAsDraws = false;
    fAllowCoverageCounting = false;
    fTransferBufferSupport = false;
    fWritePixelsRowBytesSupport = false;
    fReadPixelsRowBytesSupport = false;
    fDriverBlacklistCCPR = false;
    fDriverBlacklistMSAACCPR = false;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fAdvBlendEqBlacklist = 0;

    fMapBufferFlags = kNone_MapFlags;

    fMaxVertexAttributes = 0;
    fMaxRenderTargetSize = 1;
    fMaxPreferredRenderTargetSize = 1;
    fMaxTextureSize = 1;
    fMaxWindowRectangles = 0;
    fInternalMultisampleCount = 0;

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
        SkASSERT(!fDriverBlacklistMSAACCPR);
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
    if (options.fClearAllTextures) {
        fShouldInitializeTextures = true;
    }
#endif

    if (fMaxWindowRectangles > GrWindowRectangles::kMaxWindows) {
        SkDebugf("WARNING: capping window rectangles at %i. HW advertises support for %i.\n",
                 GrWindowRectangles::kMaxWindows, fMaxWindowRectangles);
        fMaxWindowRectangles = GrWindowRectangles::kMaxWindows;
    }

    fInternalMultisampleCount = options.fInternalMultisampleCount;

    fAvoidStencilBuffers = options.fAvoidStencilBuffers;

    fDriverBugWorkarounds.applyOverrides(options.fDriverBugWorkarounds);
}


#ifdef SK_ENABLE_DUMP_GPU
#include "src/gpu/GrTestUtils.h"

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
        if (GrCaps::kAsyncRead_MapFlag & flags) {
            str.append(" async_read");
        } else {
            str.append(" sync_read");
        }
        SkDEBUGCODE(flags &= ~GrCaps::kAsyncRead_MapFlag);
    }
    SkASSERT(0 == flags); // Make sure we handled all the flags.
    return str;
}

void GrCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("MIP Map Support", fMipMapSupport);
    writer->appendBool("NPOT Texture Tile Support", fNPOTTextureTileSupport);
    writer->appendBool("Reuse Scratch Textures", fReuseScratchTextures);
    writer->appendBool("Reuse Scratch Buffers", fReuseScratchBuffers);
    writer->appendBool("Gpu Tracing Support", fGpuTracingSupport);
    writer->appendBool("Oversized Stencil Support", fOversizedStencilSupport);
    writer->appendBool("Texture Barrier Support", fTextureBarrierSupport);
    writer->appendBool("Sample Locations Support", fSampleLocationsSupport);
    writer->appendBool("Multisample disable support", fMultisampleDisableSupport);
    writer->appendBool("Instance Attrib Support", fInstanceAttribSupport);
    writer->appendBool("Mixed Samples Support", fMixedSamplesSupport);
    writer->appendBool("MSAA Resolves Automatically", fMSAAResolvesAutomatically);
    writer->appendBool("Use primitive restart", fUsePrimitiveRestart);
    writer->appendBool("Prefer client-side dynamic buffers", fPreferClientSideDynamicBuffers);
    writer->appendBool("Prefer fullscreen clears (and stencil discard)", fPreferFullscreenClears);
    writer->appendBool("Must clear buffer memory", fMustClearUploadedBufferData);
    writer->appendBool("Should initialize textures", fShouldInitializeTextures);
    writer->appendBool("Supports importing AHardwareBuffers", fSupportsAHardwareBufferImages);
    writer->appendBool("Fence sync support", fFenceSyncSupport);
    writer->appendBool("Semaphore support", fSemaphoreSupport);
    writer->appendBool("Cross context texture support", fCrossContextTextureSupport);
    writer->appendBool("Half float vertex attribute support", fHalfFloatVertexAttributeSupport);
    writer->appendBool("Specify GeometryProcessor textures as a dynamic state array",
                       fDynamicStateArrayGeometryProcessorTextureSupport);
    writer->appendBool("Use draws for partial clears", fPerformPartialClearsAsDraws);
    writer->appendBool("Use draws for color clears", fPerformColorClearsAsDraws);
    writer->appendBool("Use draws for stencil clip clears", fPerformStencilClearsAsDraws);
    writer->appendBool("Allow coverage counting shortcuts", fAllowCoverageCounting);
    writer->appendBool("Supports transfer buffers", fTransferBufferSupport);
    writer->appendBool("Write pixels row bytes support", fWritePixelsRowBytesSupport);
    writer->appendBool("Read pixels row bytes support", fReadPixelsRowBytesSupport);
    writer->appendBool("Blacklist CCPR on current driver [workaround]", fDriverBlacklistCCPR);
    writer->appendBool("Blacklist MSAA version of CCPR on current driver [workaround]",
                       fDriverBlacklistMSAACCPR);
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
    writer->appendS32("Preferred Sample Count for Internal MSAA and Mixed Samples",
                      fInternalMultisampleCount);

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

bool GrCaps::canCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                            const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    if (dst->readOnly()) {
        return false;
    }
    // Currently we only ever do copies where the configs are the same. This check really should be
    // checking if the backend formats, color types, and swizzle are compatible. Our copy always
    // copies exact byte to byte from src to dst so when need to check the if we do this, the dst
    // has the expected values stored in the right places taking the swizzle into account. For now
    // we can be more restrictive and just make sure the configs are the same and if we generalize
    // copies and swizzles more in the future this can be updated.
    if (this->makeConfigSpecific(dst->config(), dst->backendFormat()) !=
        this->makeConfigSpecific(src->config(), src->backendFormat())) {
        return false;
    }
    return this->onCanCopySurface(dst, src, srcRect, dstPoint);
}

bool GrCaps::validateSurfaceParams(const SkISize& dimensions, const GrBackendFormat& format,
                                   GrPixelConfig config, GrRenderable renderable,
                                   int renderTargetSampleCnt, GrMipMapped mipped) const {
    if (!this->isFormatTexturable(format)) {
        return false;
    }

    if (GrMipMapped::kYes == mipped && !this->mipMapSupport()) {
        return false;
    }

    if (dimensions.width() < 1 || dimensions.height() < 1) {
        return false;
    }

    if (renderable == GrRenderable::kYes) {
        if (!this->isFormatRenderable(format, renderTargetSampleCnt)) {
            return false;
        }
        int maxRTSize = this->maxRenderTargetSize();
        if (dimensions.width() > maxRTSize || dimensions.height() > maxRTSize) {
            return false;
        }
    } else {
        // We currently do not support multisampled textures
        if (renderTargetSampleCnt != 1) {
            return false;
        }
        int maxSize = this->maxTextureSize();
        if (dimensions.width() > maxSize || dimensions.height() > maxSize) {
            return false;
        }
    }

    return true;
}

GrCaps::SupportedRead GrCaps::supportedReadPixelsColorType(GrColorType srcColorType,
                                                           const GrBackendFormat& srcFormat,
                                                           GrColorType dstColorType) const {
    SupportedRead read = this->onSupportedReadPixelsColorType(srcColorType, srcFormat,
                                                              dstColorType);

    // There are known problems with 24 vs 32 bit BPP with this color type. Just fail for now if
    // using a transfer buffer.
    if (GrColorType::kRGB_888x == read.fColorType) {
        read.fOffsetAlignmentForTransferBuffer = 0;
    }
    // It's very convenient to access 1 byte-per-channel 32 bit color types as uint32_t on the CPU.
    // Make those aligned reads out of the buffer even if the underlying API doesn't require it.
    auto componentFlags = GrColorTypeComponentFlags(read.fColorType);
    if ((componentFlags == kRGBA_SkColorTypeComponentFlags ||
         componentFlags == kRGB_SkColorTypeComponentFlags  ||
         componentFlags == kAlpha_SkColorTypeComponentFlag ||
         componentFlags == kGray_SkColorTypeComponentFlag) &&
        GrColorTypeBytesPerPixel(read.fColorType) == 4) {
        switch (read.fOffsetAlignmentForTransferBuffer & 0b11) {
            // offset alignment already a multiple of 4
            case 0:
                break;
            // offset alignment is a multiple of 2 but not 4.
            case 2:
                read.fOffsetAlignmentForTransferBuffer *= 2;
            // offset alignment is not a multiple of 2.
            default:
                read.fOffsetAlignmentForTransferBuffer *= 4;
        }
    }
    return read;
}

#ifdef SK_DEBUG
bool GrCaps::AreConfigsCompatible(GrPixelConfig genericConfig, GrPixelConfig specificConfig) {
    bool compatible = false;

    switch (genericConfig) {
        case kAlpha_8_GrPixelConfig:
            compatible = kAlpha_8_GrPixelConfig == specificConfig || // here bc of the mock context
                         kAlpha_8_as_Alpha_GrPixelConfig == specificConfig ||
                         kAlpha_8_as_Red_GrPixelConfig == specificConfig;
            break;
        case kGray_8_GrPixelConfig:
            compatible = kGray_8_GrPixelConfig == specificConfig ||  // here bc of the mock context
                         kGray_8_as_Lum_GrPixelConfig == specificConfig ||
                         kGray_8_as_Red_GrPixelConfig == specificConfig;
            break;
        case kAlpha_half_GrPixelConfig:
            compatible = kAlpha_half_GrPixelConfig == specificConfig || // bc of the mock context
                         kAlpha_half_as_Red_GrPixelConfig == specificConfig ||
                         kAlpha_half_as_Lum_GrPixelConfig == specificConfig;
            break;
        case kRGB_888_GrPixelConfig:
            compatible = kRGB_888_GrPixelConfig == specificConfig ||
                         kRGB_888X_GrPixelConfig == specificConfig;
            break;
        case kRGBA_8888_GrPixelConfig:
            compatible = kRGBA_8888_GrPixelConfig == specificConfig ||
                         kBGRA_8888_GrPixelConfig == specificConfig;
            break;
        default:
            compatible = genericConfig == specificConfig;
            break;
    }

    if (!compatible) {
        SkDebugf("Configs are not compatible: %d %d\n", genericConfig, specificConfig);
    }

    return compatible;
}
#endif

GrBackendFormat GrCaps::getDefaultBackendFormat(GrColorType grColorType,
                                                GrRenderable renderable) const {
    GrBackendFormat format = this->onGetDefaultBackendFormat(grColorType, renderable);
    if (!this->isFormatTexturableAndUploadable(grColorType, format)) {
        return {};
    }

    if (renderable == GrRenderable::kYes) {
        if (!this->isFormatAsColorTypeRenderable(grColorType, format)) {
            return {};
        }
    }

    return format;
}

