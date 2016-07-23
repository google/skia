/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrContextOptions.h"

GrShaderCaps::GrShaderCaps() {
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fIntegerSupport = false;
    fTexelBufferSupport = false;
    fShaderPrecisionVaries = false;
}

static const char* shader_type_to_string(GrShaderType type) {
    switch (type) {
    case kVertex_GrShaderType:
        return "vertex";
    case kGeometry_GrShaderType:
        return "geometry";
    case kFragment_GrShaderType:
        return "fragment";
    }
    return "";
}

static const char* precision_to_string(GrSLPrecision p) {
    switch (p) {
    case kLow_GrSLPrecision:
        return "low";
    case kMedium_GrSLPrecision:
        return "medium";
    case kHigh_GrSLPrecision:
        return "high";
    }
    return "";
}

SkString GrShaderCaps::dump() const {
    SkString r;
    static const char* gNY[] = { "NO", "YES" };
    r.appendf("Shader Derivative Support          : %s\n", gNY[fShaderDerivativeSupport]);
    r.appendf("Geometry Shader Support            : %s\n", gNY[fGeometryShaderSupport]);
    r.appendf("Path Rendering Support             : %s\n", gNY[fPathRenderingSupport]);
    r.appendf("Dst Read In Shader Support         : %s\n", gNY[fDstReadInShaderSupport]);
    r.appendf("Dual Source Blending Support       : %s\n", gNY[fDualSourceBlendingSupport]);
    r.appendf("Integer Support                    : %s\n", gNY[fIntegerSupport]);
    r.appendf("Texel Buffer Support               : %s\n", gNY[fTexelBufferSupport]);

    r.appendf("Shader Float Precisions (varies: %s):\n", gNY[fShaderPrecisionVaries]);

    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        GrShaderType shaderType = static_cast<GrShaderType>(s);
        r.appendf("\t%s:\n", shader_type_to_string(shaderType));
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            if (fFloatPrecisions[s][p].supported()) {
                GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                r.appendf("\t\t%s: log_low: %d log_high: %d bits: %d\n",
                    precision_to_string(precision),
                    fFloatPrecisions[s][p].fLogRangeLow,
                    fFloatPrecisions[s][p].fLogRangeHigh,
                    fFloatPrecisions[s][p].fBits);
            }
        }
    }

    return r;
}

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
    fDualSourceBlendingSupport = fDualSourceBlendingSupport && !options.fSuppressDualSourceBlending;
    this->onApplyOptionsOverrides(options);
}

///////////////////////////////////////////////////////////////////////////////

GrCaps::GrCaps(const GrContextOptions& options) {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fSRGBSupport = false;
    fSRGBWriteControl = false;
    fTwoSidedStencilSupport = false;
    fStencilWrapOpsSupport = false;
    fDiscardRenderTargetSupport = false;
    fReuseScratchTextures = true;
    fReuseScratchBuffers = true;
    fGpuTracingSupport = false;
    fCompressedTexSubImageSupport = false;
    fOversizedStencilSupport = false;
    fTextureBarrierSupport = false;
    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;
    fUsesMixedSamples = false;
    fSupportsInstancedDraws = false;
    fFullClearIsFree = false;
    fMustClearUploadedBufferData = false;
    fSampleShadingSupport = false;

    fUseDrawInsteadOfClear = false;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fAdvBlendEqBlacklist = 0;

    fMapBufferFlags = kNone_MapFlags;

    fMaxVertexAttributes = 0;
    fMaxRenderTargetSize = 1;
    fMaxTextureSize = 1;
    fMaxColorSampleCount = 0;
    fMaxStencilSampleCount = 0;
    fMaxRasterSamples = 0;

    fSuppressPrints = options.fSuppressPrints;
    fImmediateFlush = options.fImmediateMode;
    fBufferMapThreshold = options.fBufferMapThreshold;
    fUseDrawInsteadOfPartialRenderTargetWrite = options.fUseDrawInsteadOfPartialRenderTargetWrite;
    fUseDrawInsteadOfAllRenderTargetWrites = false;

    fPreferVRAMUseOverFlushes = true;
}

void GrCaps::applyOptionsOverrides(const GrContextOptions& options) {
    fMaxTextureSize = SkTMin(fMaxTextureSize, options.fMaxTextureSizeOverride);
    // If the max tile override is zero, it means we should use the max texture size.
    if (!options.fMaxTileSizeOverride || options.fMaxTileSizeOverride > fMaxTextureSize) {
        fMaxTileSize = fMaxTextureSize;
    } else {
        fMaxTileSize = options.fMaxTileSizeOverride;
    }
    this->onApplyOptionsOverrides(options);
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
    r.appendf("Two Sided Stencil Support          : %s\n", gNY[fTwoSidedStencilSupport]);
    r.appendf("Stencil Wrap Ops  Support          : %s\n", gNY[fStencilWrapOpsSupport]);
    r.appendf("Discard Render Target Support      : %s\n", gNY[fDiscardRenderTargetSupport]);
    r.appendf("Reuse Scratch Textures             : %s\n", gNY[fReuseScratchTextures]);
    r.appendf("Reuse Scratch Buffers              : %s\n", gNY[fReuseScratchBuffers]);
    r.appendf("Gpu Tracing Support                : %s\n", gNY[fGpuTracingSupport]);
    r.appendf("Compressed Update Support          : %s\n", gNY[fCompressedTexSubImageSupport]);
    r.appendf("Oversized Stencil Support          : %s\n", gNY[fOversizedStencilSupport]);
    r.appendf("Texture Barrier Support            : %s\n", gNY[fTextureBarrierSupport]);
    r.appendf("Sample Locations Support           : %s\n", gNY[fSampleLocationsSupport]);
    r.appendf("Multisample disable support        : %s\n", gNY[fMultisampleDisableSupport]);
    r.appendf("Uses Mixed Samples                 : %s\n", gNY[fUsesMixedSamples]);
    r.appendf("Supports instanced draws           : %s\n", gNY[fSupportsInstancedDraws]);
    r.appendf("Full screen clear is free          : %s\n", gNY[fFullClearIsFree]);
    r.appendf("Must clear buffer memory           : %s\n", gNY[fMustClearUploadedBufferData]);
    r.appendf("Draw Instead of Clear [workaround] : %s\n", gNY[fUseDrawInsteadOfClear]);
    r.appendf("Draw Instead of TexSubImage [workaround] : %s\n",
              gNY[fUseDrawInsteadOfPartialRenderTargetWrite]);
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

    static const char* kConfigNames[] = {
        "Unknown",  // kUnknown_GrPixelConfig
        "Alpha8",   // kAlpha_8_GrPixelConfig,
        "Index8",   // kIndex_8_GrPixelConfig,
        "RGB565",   // kRGB_565_GrPixelConfig,
        "RGBA444",  // kRGBA_4444_GrPixelConfig,
        "RGBA8888", // kRGBA_8888_GrPixelConfig,
        "BGRA8888", // kBGRA_8888_GrPixelConfig,
        "SRGBA8888",// kSRGBA_8888_GrPixelConfig,
        "SBGRA8888",// kSBGRA_8888_GrPixelConfig,
        "ETC1",     // kETC1_GrPixelConfig,
        "LATC",     // kLATC_GrPixelConfig,
        "R11EAC",   // kR11_EAC_GrPixelConfig,
        "ASTC12x12",// kASTC_12x12_GrPixelConfig,
        "RGBAFloat",// kRGBA_float_GrPixelConfig
        "AlphaHalf",// kAlpha_half_GrPixelConfig
        "RGBAHalf", // kRGBA_half_GrPixelConfig
    };
    GR_STATIC_ASSERT(0  == kUnknown_GrPixelConfig);
    GR_STATIC_ASSERT(1  == kAlpha_8_GrPixelConfig);
    GR_STATIC_ASSERT(2  == kIndex_8_GrPixelConfig);
    GR_STATIC_ASSERT(3  == kRGB_565_GrPixelConfig);
    GR_STATIC_ASSERT(4  == kRGBA_4444_GrPixelConfig);
    GR_STATIC_ASSERT(5  == kRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(6  == kBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(7  == kSRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(8  == kSBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(9  == kETC1_GrPixelConfig);
    GR_STATIC_ASSERT(10  == kLATC_GrPixelConfig);
    GR_STATIC_ASSERT(11  == kR11_EAC_GrPixelConfig);
    GR_STATIC_ASSERT(12 == kASTC_12x12_GrPixelConfig);
    GR_STATIC_ASSERT(13 == kRGBA_float_GrPixelConfig);
    GR_STATIC_ASSERT(14 == kAlpha_half_GrPixelConfig);
    GR_STATIC_ASSERT(15 == kRGBA_half_GrPixelConfig);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kConfigNames) == kGrPixelConfigCnt);

    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, false));
    SkASSERT(!this->isConfigRenderable(kUnknown_GrPixelConfig, true));

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        r.appendf("%s is renderable: %s, with MSAA: %s\n",
                  kConfigNames[i],
                  gNY[this->isConfigRenderable(config, false)],
                  gNY[this->isConfigRenderable(config, true)]);
    }

    SkASSERT(!this->isConfigTexturable(kUnknown_GrPixelConfig));

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        r.appendf("%s is uploadable to a texture: %s\n",
                  kConfigNames[i],
                  gNY[this->isConfigTexturable(config)]);
    }

    return r;
}
