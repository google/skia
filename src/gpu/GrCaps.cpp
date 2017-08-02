/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrContextOptions.h"
#include "GrWindowRectangles.h"

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

SkString GrCaps::dump(int indent) const {
    SkString r;
    r.appendf("{\n");
    indent += 2;

    static const char* gNY[] = {"NO", "YES"};
    auto appendFlag = [&](const char* desc, bool flag) {
        r.appendf("%*s\"%s\": \"%s\",\n", indent, "", desc, gNY[flag]);
    };
    appendFlag("MIP Map Support", fMipMapSupport);
    appendFlag("NPOT Texture Tile Support", fNPOTTextureTileSupport);
    appendFlag("sRGB Support", fSRGBSupport);
    appendFlag("sRGB Write Control", fSRGBWriteControl);
    appendFlag("sRGB Decode Disable", fSRGBDecodeDisableSupport);
    appendFlag("Discard Render Target Support", fDiscardRenderTargetSupport);
    appendFlag("Reuse Scratch Textures", fReuseScratchTextures);
    appendFlag("Reuse Scratch Buffers", fReuseScratchBuffers);
    appendFlag("Gpu Tracing Support", fGpuTracingSupport);
    appendFlag("Oversized Stencil Support", fOversizedStencilSupport);
    appendFlag("Texture Barrier Support", fTextureBarrierSupport);
    appendFlag("Sample Locations Support", fSampleLocationsSupport);
    appendFlag("Multisample disable support", fMultisampleDisableSupport);
    appendFlag("Instance Attrib Support", fInstanceAttribSupport);
    appendFlag("Uses Mixed Samples", fUsesMixedSamples);
    appendFlag("Prefer client-side dynamic buffers", fPreferClientSideDynamicBuffers);
    appendFlag("Full screen clear is free", fFullClearIsFree);
    appendFlag("Must clear buffer memory", fMustClearUploadedBufferData);
    appendFlag("Sample shading support", fSampleShadingSupport);
    appendFlag("Fence sync support", fFenceSyncSupport);
    appendFlag("Cross context texture support", fCrossContextTextureSupport);

    appendFlag("Draw Instead of Clear [workaround]", fUseDrawInsteadOfClear);
    appendFlag("Prefer VRAM Use over flushes [workaround]", fPreferVRAMUseOverFlushes);

    if (this->advancedBlendEquationSupport()) {
        r.appendf("%*s\"Advanced Blend Equation Blacklist\": \"0x%x\",\n", indent, "",
                  fAdvBlendEqBlacklist);
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
