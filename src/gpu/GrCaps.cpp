/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrContextOptions.h"
#include "GrWindowRectangles.h"
#include "SkJSONWriter.h"

static const char* pixel_config_name(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig: return "Unknown";
        case kAlpha_8_GrPixelConfig: return "Alpha8";
        case kGray_8_GrPixelConfig: return "Gray8";
        case kRGB_565_GrPixelConfig: return "RGB565";
        case kRGBA_4444_GrPixelConfig: return "RGBA444";
        case kRGBA_8888_GrPixelConfig: return "RGBA8888";
        case kBGRA_8888_GrPixelConfig: return "BGRA8888";
        case kSRGBA_8888_GrPixelConfig: return "SRGBA8888";
        case kSBGRA_8888_GrPixelConfig: return "SBGRA8888";
        case kRGBA_8888_sint_GrPixelConfig: return "RGBA8888_sint";
        case kRGBA_float_GrPixelConfig: return "RGBAFloat";
        case kRG_float_GrPixelConfig: return "RGFloat";
        case kAlpha_half_GrPixelConfig: return "AlphaHalf";
        case kRGBA_half_GrPixelConfig: return "RGBAHalf";
    }
    SK_ABORT("Invalid pixel config");
    return "<invalid>";
}

GrCaps::GrCaps(const GrContextOptions& options) {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fSRGBSupport = false;
    fSRGBWriteControl = false;
    fSRGBDecodeDisableSupport = false;
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
    fPreferClientSideDynamicBuffers = false;
    fFullClearIsFree = false;
    fMustClearUploadedBufferData = false;
    fSampleShadingSupport = false;
    fFenceSyncSupport = false;
    fCrossContextTextureSupport = false;

    fInstancedSupport = InstancedSupport::kNone;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fAdvBlendEqBlacklist = 0;

    fMapBufferFlags = kNone_MapFlags;

    fMaxVertexAttributes = 0;
    fMaxRenderTargetSize = 1;
    fMaxTextureSize = 1;
    fMaxColorSampleCount = 0;
    fMaxStencilSampleCount = 0;
    fMaxRasterSamples = 0;
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
    fAvoidInstancedDrawsToFPTargets = false;
    fBlacklistCoverageCounting = false;
    fAvoidStencilBuffers = false;

    fPreferVRAMUseOverFlushes = true;
}

void GrCaps::applyOptionsOverrides(const GrContextOptions& options) {
    this->onApplyOptionsOverrides(options);
    fMaxTextureSize = SkTMin(fMaxTextureSize, options.fMaxTextureSizeOverride);
    fMaxTileSize = fMaxTextureSize;
#if GR_TEST_UTILS
    // If the max tile override is zero, it means we should use the max texture size.
    if (options.fMaxTileSizeOverride && options.fMaxTileSizeOverride < fMaxTextureSize) {
        fMaxTileSize = options.fMaxTileSizeOverride;
    }
#endif
    if (fMaxWindowRectangles > GrWindowRectangles::kMaxWindows) {
        SkDebugf("WARNING: capping window rectangles at %i. HW advertises support for %i.\n",
                 GrWindowRectangles::kMaxWindows, fMaxWindowRectangles);
        fMaxWindowRectangles = GrWindowRectangles::kMaxWindows;
    }
    fAvoidStencilBuffers = options.fAvoidStencilBuffers;
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
    writer->appendBool("sRGB Decode Disable", fSRGBDecodeDisableSupport);
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
    writer->appendBool("Prefer client-side dynamic buffers", fPreferClientSideDynamicBuffers);
    writer->appendBool("Full screen clear is free", fFullClearIsFree);
    writer->appendBool("Must clear buffer memory", fMustClearUploadedBufferData);
    writer->appendBool("Sample shading support", fSampleShadingSupport);
    writer->appendBool("Fence sync support", fFenceSyncSupport);
    writer->appendBool("Cross context texture support", fCrossContextTextureSupport);

    writer->appendBool("Blacklist Coverage Counting Path Renderer [workaround]",
                       fBlacklistCoverageCounting);
    writer->appendBool("Prefer VRAM Use over flushes [workaround]", fPreferVRAMUseOverFlushes);

    if (this->advancedBlendEquationSupport()) {
        writer->appendHexU32("Advanced Blend Equation Blacklist", fAdvBlendEqBlacklist);
    }

    writer->appendS32("Max Vertex Attributes", fMaxVertexAttributes);
    writer->appendS32("Max Texture Size", fMaxTextureSize);
    writer->appendS32("Max Render Target Size", fMaxRenderTargetSize);
    writer->appendS32("Max Color Sample Count", fMaxColorSampleCount);
    writer->appendS32("Max Stencil Sample Count", fMaxStencilSampleCount);
    writer->appendS32("Max Raster Samples", fMaxRasterSamples);
    writer->appendS32("Max Window Rectangles", fMaxWindowRectangles);
    writer->appendS32("Max Clip Analytic Fragment Processors", fMaxClipAnalyticFPs);

    static const char* kInstancedSupportNames[] = {
        "None",
        "Basic",
        "Multisampled",
        "Mixed Sampled",
    };
    GR_STATIC_ASSERT(0 == (int)InstancedSupport::kNone);
    GR_STATIC_ASSERT(1 == (int)InstancedSupport::kBasic);
    GR_STATIC_ASSERT(2 == (int)InstancedSupport::kMultisampled);
    GR_STATIC_ASSERT(3 == (int)InstancedSupport::kMixedSampled);
    GR_STATIC_ASSERT(4 == SK_ARRAY_COUNT(kInstancedSupportNames));

    writer->appendString("Instanced Support", kInstancedSupportNames[(int)fInstancedSupport]);

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

    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, false));
    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, true));
    SkASSERT(!this->isConfigTexturable(kUnknown_GrPixelConfig));

    writer->beginArray("configs");

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        writer->beginObject(nullptr, false);
        writer->appendString("name", pixel_config_name(config));
        writer->appendBool("renderable", this->isConfigRenderable(config, false));
        writer->appendBool("renderableMSAA", this->isConfigRenderable(config, true));
        writer->appendBool("texturable", this->isConfigTexturable(config));
        writer->endObject();
    }

    writer->endArray();

    this->onDumpJSON(writer);

    writer->appendName("shaderCaps");
    this->shaderCaps()->dumpJSON(writer);

    writer->endObject();
}
