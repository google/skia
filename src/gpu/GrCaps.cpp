/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCaps.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrWindowRectangles.h"
#include "src/utils/SkJSONWriter.h"

GrCaps::GrCaps(const GrContextOptions& options) {
    fMipmapSupport = false;
    fNPOTTextureTileSupport = false;
    fReuseScratchTextures = true;
    fReuseScratchBuffers = true;
    fGpuTracingSupport = false;
    fOversizedStencilSupport = false;
    fTextureBarrierSupport = false;
    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;
    fDrawInstancedSupport = false;
    fNativeDrawIndirectSupport = false;
    fUseClientSideIndirectBuffers = false;
    fMixedSamplesSupport = false;
    fConservativeRasterSupport = false;
    fWireframeSupport = false;
    fMSAAResolvesAutomatically = false;
    fUsePrimitiveRestart = false;
    fPreferClientSideDynamicBuffers = false;
    fPreferFullscreenClears = false;
    fTwoSidedStencilRefsAndMasksMustMatch = false;
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
    fAvoidLargeIndexBufferDraws = false;
    fPerformStencilClearsAsDraws = false;
    fAllowCoverageCounting = false;
    fTransferFromBufferToTextureSupport = false;
    fTransferFromSurfaceToBufferSupport = false;
    fWritePixelsRowBytesSupport = false;
    fReadPixelsRowBytesSupport = false;
    fShouldCollapseSrcOverToSrcWhenAble = false;
    fDriverDisableCCPR = false;
    fDriverDisableMSAACCPR = false;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fAdvBlendEqDisableFlags = 0;

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
    fRequiresManualFBBarrierAfterTessellatedStencilDraw = false;
    fNativeDrawIndexedIndirectIsBroken = false;
    fAvoidClearingFBOMipmapLevels = false;

    fPreferVRAMUseOverFlushes = true;

    fPreferTrianglesOverSampleMask = false;

    // Default to true, allow older versions of OpenGL to disable explicitly
    fClampToBorderSupport = true;

    fDriverBugWorkarounds = options.fDriverBugWorkarounds;
}

void GrCaps::finishInitialization(const GrContextOptions& options) {
    if (fMixedSamplesSupport) {
        // We need multisample disable and dual source blending in order to support mixed samples.
        fMixedSamplesSupport = this->multisampleDisableSupport() &&
                               this->shaderCaps()->dualSourceBlendingSupport();
    }

    if (!fNativeDrawIndirectSupport) {
        // We will implement indirect draws with a polyfill, so the commands need to reside in CPU
        // memory.
        fUseClientSideIndirectBuffers = true;
    }

    // Overrides happen last.
    this->applyOptionsOverrides(options);
}

void GrCaps::applyOptionsOverrides(const GrContextOptions& options) {
    fShaderCaps->applyOptionsOverrides(options);
    this->onApplyOptionsOverrides(options);
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(!fDriverDisableCCPR);
        SkASSERT(!fDriverDisableMSAACCPR);
        SkASSERT(!fAvoidStencilBuffers);
        SkASSERT(!fAvoidWritePixelsFastPath);
        SkASSERT(!fRequiresManualFBBarrierAfterTessellatedStencilDraw);
        SkASSERT(!fNativeDrawIndexedIndirectIsBroken);
        SkASSERT(!fAdvBlendEqDisableFlags);
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

    fMaxTextureSize = std::min(fMaxTextureSize, options.fMaxTextureSizeOverride);
    fMaxTileSize = fMaxTextureSize;
#if GR_TEST_UTILS
    // If the max tile override is zero, it means we should use the max texture size.
    if (options.fMaxTileSizeOverride && options.fMaxTileSizeOverride < fMaxTextureSize) {
        fMaxTileSize = options.fMaxTileSizeOverride;
    }
    if (options.fSuppressDualSourceBlending) {
        // GrShaderCaps::applyOptionsOverrides already handled the rest; here we just need to make
        // sure mixed samples gets disabled if dual source blending is suppressed.
        fMixedSamplesSupport = false;
    }
    if (options.fClearAllTextures) {
        fShouldInitializeTextures = true;
    }
#endif
    if (options.fSuppressMipmapSupport) {
        fMipmapSupport = false;
    }

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

    writer->appendBool("MIP Map Support", fMipmapSupport);
    writer->appendBool("NPOT Texture Tile Support", fNPOTTextureTileSupport);
    writer->appendBool("Reuse Scratch Textures", fReuseScratchTextures);
    writer->appendBool("Reuse Scratch Buffers", fReuseScratchBuffers);
    writer->appendBool("Gpu Tracing Support", fGpuTracingSupport);
    writer->appendBool("Oversized Stencil Support", fOversizedStencilSupport);
    writer->appendBool("Texture Barrier Support", fTextureBarrierSupport);
    writer->appendBool("Sample Locations Support", fSampleLocationsSupport);
    writer->appendBool("Multisample disable support", fMultisampleDisableSupport);
    writer->appendBool("Draw Instanced Support", fDrawInstancedSupport);
    writer->appendBool("Native Draw Indirect Support", fNativeDrawIndirectSupport);
    writer->appendBool("Use client side indirect buffers", fUseClientSideIndirectBuffers);
    writer->appendBool("Mixed Samples Support", fMixedSamplesSupport);
    writer->appendBool("Conservative Raster Support", fConservativeRasterSupport);
    writer->appendBool("Wireframe Support", fWireframeSupport);
    writer->appendBool("MSAA Resolves Automatically", fMSAAResolvesAutomatically);
    writer->appendBool("Use primitive restart", fUsePrimitiveRestart);
    writer->appendBool("Prefer client-side dynamic buffers", fPreferClientSideDynamicBuffers);
    writer->appendBool("Prefer fullscreen clears (and stencil discard)", fPreferFullscreenClears);
    writer->appendBool("Two-sided Stencil Refs And Masks Must Match",
                       fTwoSidedStencilRefsAndMasksMustMatch);
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
    writer->appendBool("Avoid Large IndexBuffer Draws", fAvoidLargeIndexBufferDraws);
    writer->appendBool("Use draws for stencil clip clears", fPerformStencilClearsAsDraws);
    writer->appendBool("Allow coverage counting shortcuts", fAllowCoverageCounting);
    writer->appendBool("Supports transfers from buffers to textures",
                       fTransferFromBufferToTextureSupport);
    writer->appendBool("Supports transfers from textures to buffers",
                       fTransferFromSurfaceToBufferSupport);
    writer->appendBool("Write pixels row bytes support", fWritePixelsRowBytesSupport);
    writer->appendBool("Read pixels row bytes support", fReadPixelsRowBytesSupport);
    writer->appendBool("Disable CCPR on current driver [workaround]", fDriverDisableCCPR);
    writer->appendBool("Disable MSAA version of CCPR on current driver [workaround]",
                       fDriverDisableMSAACCPR);
    writer->appendBool("Clamp-to-border", fClampToBorderSupport);

    writer->appendBool("Prefer VRAM Use over flushes [workaround]", fPreferVRAMUseOverFlushes);
    writer->appendBool("Prefer more triangles over sample mask [MSAA only]",
                       fPreferTrianglesOverSampleMask);
    writer->appendBool("Avoid stencil buffers [workaround]", fAvoidStencilBuffers);
    writer->appendBool("Avoid writePixels fast path [workaround]", fAvoidWritePixelsFastPath);
    writer->appendBool("Requires manual FB barrier after tessellated stencilDraw [workaround]",
                       fRequiresManualFBBarrierAfterTessellatedStencilDraw);
    writer->appendBool("Native draw indexed indirect is broken [workaround]",
                       fNativeDrawIndexedIndirectIsBroken);

    if (this->advancedBlendEquationSupport()) {
        writer->appendHexU32("Advanced Blend Equation Disable Flags", fAdvBlendEqDisableFlags);
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
    static_assert(0 == kBasic_BlendEquationSupport);
    static_assert(1 == kAdvanced_BlendEquationSupport);
    static_assert(2 == kAdvancedCoherent_BlendEquationSupport);
    static_assert(SK_ARRAY_COUNT(kBlendEquationSupportNames) == kLast_BlendEquationSupport + 1);

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

    if (dst->backendFormat() != src->backendFormat()) {
        return false;
    }
    return this->onCanCopySurface(dst, src, srcRect, dstPoint);
}

bool GrCaps::validateSurfaceParams(const SkISize& dimensions, const GrBackendFormat& format,
                                   GrRenderable renderable, int renderTargetSampleCnt,
                                   GrMipmapped mipped) const {
    if (!this->isFormatTexturable(format)) {
        return false;
    }

    if (GrMipmapped::kYes == mipped && !this->mipmapSupport()) {
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
    auto channelFlags = GrColorTypeChannelFlags(read.fColorType);
    if ((channelFlags == kRGBA_SkColorChannelFlags || channelFlags == kRGB_SkColorChannelFlags ||
         channelFlags == kAlpha_SkColorChannelFlag || channelFlags == kGray_SkColorChannelFlag) &&
        GrColorTypeBytesPerPixel(read.fColorType) == 4) {
        switch (read.fOffsetAlignmentForTransferBuffer & 0b11) {
            // offset alignment already a multiple of 4
            case 0:
                break;
            // offset alignment is a multiple of 2 but not 4.
            case 2:
                read.fOffsetAlignmentForTransferBuffer *= 2;
                break;
            // offset alignment is not a multiple of 2.
            default:
                read.fOffsetAlignmentForTransferBuffer *= 4;
                break;
        }
    }
    return read;
}

GrBackendFormat GrCaps::getDefaultBackendFormat(GrColorType colorType,
                                                GrRenderable renderable) const {
    auto format = this->onGetDefaultBackendFormat(colorType);
    if (!this->isFormatTexturable(format)) {
        return {};
    }
    if (!this->areColorTypeAndFormatCompatible(colorType, format)) {
        return {};
    }
    // Currently we require that it be possible to write pixels into the "default" format. Perhaps,
    // that could be a separate requirement from the caller. It seems less necessary if
    // renderability was requested.
    if (this->supportedWritePixelsColorType(colorType, format, colorType).fColorType ==
        GrColorType::kUnknown) {
        return {};
    }
    if (renderable == GrRenderable::kYes &&
        !this->isFormatAsColorTypeRenderable(colorType, format)) {
        return {};
    }
    return format;
}

bool GrCaps::areColorTypeAndFormatCompatible(GrColorType grCT,
                                             const GrBackendFormat& format) const {
    if (GrColorType::kUnknown == grCT) {
        return false;
    }

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);
    if (compression != SkImage::CompressionType::kNone) {
        return grCT == (SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                               : GrColorType::kRGBA_8888);
    }

    return this->onAreColorTypeAndFormatCompatible(grCT, format);
}

GrSwizzle GrCaps::getReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);
    if (compression != SkImage::CompressionType::kNone) {
        if (colorType == GrColorType::kRGB_888x || colorType == GrColorType::kRGBA_8888) {
            return GrSwizzle::RGBA();
        }
        SkDEBUGFAILF("Illegal color type (%d) and compressed format (%d) combination.", colorType,
                     compression);
        return {};
    }

    return this->onGetReadSwizzle(format, colorType);
}

bool GrCaps::isFormatCompressed(const GrBackendFormat& format) const {
    return GrBackendFormatToCompressionType(format) != SkImage::CompressionType::kNone;
}

GrDstSampleType GrCaps::getDstSampleTypeForProxy(const GrRenderTargetProxy* rt) const {
    SkASSERT(rt);
    if (this->textureBarrierSupport() && !rt->requiresManualMSAAResolve()) {
        return this->onGetDstSampleTypeForProxy(rt);
    }
    return GrDstSampleType::kAsTextureCopy;
}
