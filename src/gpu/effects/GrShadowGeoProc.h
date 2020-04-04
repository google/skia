/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowGeoProc_DEFINED
#define GrShadowGeoProc_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProcessor.h"

class GrGLRRectShadowGeoProc;
class GrSurfaceProxyView;

/**
 * The output color of this effect is a coverage mask for a rrect shadow,
 * assuming circular corner geometry.
 */
class GrRRectShadowGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const GrSurfaceProxyView& lutView) {
        return arena->make<GrRRectShadowGeoProc>(lutView);
    }

    const char* name() const override { return "RRectShadow"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inShadowParams() const { return fInShadowParams; }
    GrColor color() const { return fColor; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    friend class ::SkArenaAlloc; // for access to ctor

    GrRRectShadowGeoProc(const GrSurfaceProxyView& lutView);

    const TextureSampler& onTextureSampler(int i) const override { return fLUTTextureSampler; }

    GrColor          fColor;
    TextureSampler   fLUTTextureSampler;

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInShadowParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

#endif
