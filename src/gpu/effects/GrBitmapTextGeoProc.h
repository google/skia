/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextGeoProc_DEFINED
#define GrBitmapTextGeoProc_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProcessor.h"

class GrGLBitmapTextGeoProc;
class GrInvariantOutput;
class GrSurfaceProxyView;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute.
 */
class GrBitmapTextGeoProc : public GrGeometryProcessor {
public:
    static constexpr int kMaxTextures = 4;

    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const GrShaderCaps& caps,
                                     const SkPMColor4f& color,
                                     bool wideColor,
                                     const GrSurfaceProxyView* views,
                                     int numActiveViews,
                                     GrSamplerState p,
                                     GrMaskFormat format,
                                     const SkMatrix& localMatrix,
                                     bool usesW) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrBitmapTextGeoProc(caps, color, wideColor, views, numActiveViews,
                                                 p, format, localMatrix, usesW);
        });
    }

    ~GrBitmapTextGeoProc() override {}

    const char* name() const override { return "BitmapText"; }

    void addNewViews(const GrSurfaceProxyView*, int numActiveViews, GrSamplerState);

    void addToKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps& caps) const override;

private:
    class Impl;

    GrBitmapTextGeoProc(const GrShaderCaps&, const SkPMColor4f&, bool wideColor,
                        const GrSurfaceProxyView* views, int numViews, GrSamplerState params,
                        GrMaskFormat format, const SkMatrix& localMatrix, bool usesW);

    bool hasVertexColor() const { return fInColor.isInitialized(); }

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    SkPMColor4f      fColor;
    SkMatrix         fLocalMatrix;
    bool             fUsesW;
    SkISize          fAtlasDimensions;  // dimensions for all textures used with fTextureSamplers[].
    TextureSampler   fTextureSamplers[kMaxTextures];
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    GrMaskFormat     fMaskFormat;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

#endif
