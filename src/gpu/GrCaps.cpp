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
    SkFAIL("Invalid pixel config");
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

    fUseDrawInsteadOfClear = false;

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

    fSuppressPrints = options.fSuppressPrints;
    fWireframeMode = options.fWireframeMode;
    fBufferMapThreshold = options.fBufferMapThreshold;
    fAvoidInstancedDrawsToFPTargets = false;
    fAvoidStencilBuffers = false;

    fPreferVRAMUseOverFlushes = true;
}

void GrCaps::applyOptionsOverrides(const GrContextOptions& options) {
    this->onApplyOptionsOverrides(options);
    fMaxTextureSize = SkTMin(fMaxTextureSize, options.fMaxTextureSizeOverride);
    // If the max tile override is zero, it means we should use the max texture size.
    if (!options.fMaxTileSizeOverride || options.fMaxTileSizeOverride > fMaxTextureSize) {
        fMaxTileSize = fMaxTextureSize;
    } else {
        fMaxTileSize = options.fMaxTileSizeOverride;
    }
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

SkString GrCaps::dump() const {
    SkString r;
    static const char* gNY[] = {"NO", "YES"};
    r.appendf("MIP Map Support                    : %s\n", gNY[fMipMapSupport]);
    r.appendf("NPOT Texture Tile Support          : %s\n", gNY[fNPOTTextureTileSupport]);
    r.appendf("sRGB Support                       : %s\n", gNY[fSRGBSupport]);
    r.appendf("sRGB Write Control                 : %s\n", gNY[fSRGBWriteControl]);
    r.appendf("Discard Render Target Support      : %s\n", gNY[fDiscardRenderTargetSupport]);
    r.appendf("Reuse Scratch Textures             : %s\n", gNY[fReuseScratchTextures]);
    r.appendf("Reuse Scratch Buffers              : %s\n", gNY[fReuseScratchBuffers]);
    r.appendf("Gpu Tracing Support                : %s\n", gNY[fGpuTracingSupport]);
    r.appendf("Oversized Stencil Support          : %s\n", gNY[fOversizedStencilSupport]);
    r.appendf("Texture Barrier Support            : %s\n", gNY[fTextureBarrierSupport]);
    r.appendf("Sample Locations Support           : %s\n", gNY[fSampleLocationsSupport]);
    r.appendf("Multisample disable support        : %s\n", gNY[fMultisampleDisableSupport]);
    r.appendf("Instance Attrib Support            : %s\n", gNY[fInstanceAttribSupport]);
    r.appendf("Uses Mixed Samples                 : %s\n", gNY[fUsesMixedSamples]);
    r.appendf("Prefer client-side dynamic buffers : %s\n", gNY[fPreferClientSideDynamicBuffers]);
    r.appendf("Full screen clear is free          : %s\n", gNY[fFullClearIsFree]);
    r.appendf("Must clear buffer memory           : %s\n", gNY[fMustClearUploadedBufferData]);
    r.appendf("Sample shading support             : %s\n", gNY[fSampleShadingSupport]);
    r.appendf("Fence sync support                 : %s\n", gNY[fFenceSyncSupport]);
    r.appendf("Cross context texture support      : %s\n", gNY[fCrossContextTextureSupport]);

    r.appendf("Draw Instead of Clear [workaround] : %s\n", gNY[fUseDrawInsteadOfClear]);
    r.appendf("Prefer VRAM Use over flushes [workaround] : %s\n", gNY[fPreferVRAMUseOverFlushes]);

    if (this->advancedBlendEquationSupport()) {
        r.appendf("Advanced Blend Equation Blacklist  : 0x%x\n", fAdvBlendEqBlacklist);
    }

    r.appendf("Max Vertex Attributes              : %d\n", fMaxVertexAttributes);
    r.appendf("Max Texture Size                   : %d\n", fMaxTextureSize);
    r.appendf("Max Render Target Size             : %d\n", fMaxRenderTargetSize);
    r.appendf("Max Color Sample Count             : %d\n", fMaxColorSampleCount);
    r.appendf("Max Stencil Sample Count           : %d\n", fMaxStencilSampleCount);
    r.appendf("Max Raster Samples                 : %d\n", fMaxRasterSamples);
    r.appendf("Max Window Rectangles              : %d\n", fMaxWindowRectangles);

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

    r.appendf("Instanced Support                  : %s\n",
              kInstancedSupportNames[(int)fInstancedSupport]);

    static const char* kBlendEquationSupportNames[] = {
        "Basic",
        "Advanced",
        "Advanced Coherent",
    };
    GR_STATIC_ASSERT(0 == kBasic_BlendEquationSupport);
    GR_STATIC_ASSERT(1 == kAdvanced_BlendEquationSupport);
    GR_STATIC_ASSERT(2 == kAdvancedCoherent_BlendEquationSupport);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kBlendEquationSupportNames) == kLast_BlendEquationSupport + 1);

    r.appendf("Blend Equation Support             : %s\n",
              kBlendEquationSupportNames[fBlendEquationSupport]);
    r.appendf("Map Buffer Support                 : %s\n",
              map_flags_to_string(fMapBufferFlags).c_str());

    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, false));
    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, true));

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i)  {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        r.appendf("%s is renderable: %s, with MSAA: %s\n",
                  pixel_config_name(config),
                  gNY[this->isConfigRenderable(config, false)],
                  gNY[this->isConfigRenderable(config, true)]);
    }

    SkASSERT(!this->isConfigTexturable(kUnknown_GrPixelConfig));

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i)  {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        r.appendf("%s is uploadable to a texture: %s\n",
                  pixel_config_name(config),
                  gNY[this->isConfigTexturable(config)]);
    }

    return r;
}

void GrCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    static const char* gNY[] = { "NO", "YES" };
    writer->appendString("MIP Map Support", gNY[fMipMapSupport]);
    writer->appendString("NPOT Texture Tile Support", gNY[fNPOTTextureTileSupport]);
    writer->appendString("sRGB Support", gNY[fSRGBSupport]);
    writer->appendString("sRGB Write Control", gNY[fSRGBWriteControl]);
    writer->appendString("sRGB Decode Disable", gNY[fSRGBDecodeDisableSupport]);
    writer->appendString("Discard Render Target Support", gNY[fDiscardRenderTargetSupport]);
    writer->appendString("Reuse Scratch Textures", gNY[fReuseScratchTextures]);
    writer->appendString("Reuse Scratch Buffers", gNY[fReuseScratchBuffers]);
    writer->appendString("Gpu Tracing Support", gNY[fGpuTracingSupport]);
    writer->appendString("Oversized Stencil Support", gNY[fOversizedStencilSupport]);
    writer->appendString("Texture Barrier Support", gNY[fTextureBarrierSupport]);
    writer->appendString("Sample Locations Support", gNY[fSampleLocationsSupport]);
    writer->appendString("Multisample disable support", gNY[fMultisampleDisableSupport]);
    writer->appendString("Instance Attrib Support", gNY[fInstanceAttribSupport]);
    writer->appendString("Uses Mixed Samples", gNY[fUsesMixedSamples]);
    writer->appendString("Prefer client-side dynamic buffers", gNY[fPreferClientSideDynamicBuffers]);
    writer->appendString("Full screen clear is free", gNY[fFullClearIsFree]);
    writer->appendString("Must clear buffer memory", gNY[fMustClearUploadedBufferData]);
    writer->appendString("Sample shading support", gNY[fSampleShadingSupport]);
    writer->appendString("Fence sync support", gNY[fFenceSyncSupport]);
    writer->appendString("Cross context texture support", gNY[fCrossContextTextureSupport]);

    writer->appendString("Draw Instead of Clear [workaround]", gNY[fUseDrawInsteadOfClear]);
    writer->appendString("Prefer VRAM Use over flushes [workaround]", gNY[fPreferVRAMUseOverFlushes]);

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

    // TODO: Use real structured JSON for this data
    writer->beginArray("configsRenderable");

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        writer->appendString(SkStringPrintf("%s is renderable: %s, with MSAA: %s",
                                            pixel_config_name(config),
                                            gNY[this->isConfigRenderable(config, false)],
                                            gNY[this->isConfigRenderable(config, true)]).c_str());
    }

    writer->endArray();

    SkASSERT(!this->isConfigTexturable(kUnknown_GrPixelConfig));

    writer->beginArray("configsTexturable");

    for (size_t i = 1; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        writer->appendString(SkStringPrintf("%s is uploadable to a texture: %s",
                                            pixel_config_name(config),
                                            gNY[this->isConfigTexturable(config)]).c_str());
    }

    writer->endArray();

    this->onDumpJSON(writer);

    writer->endObject();
}
