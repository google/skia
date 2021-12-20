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
        return arena->make([&](void* ptr) {
            return new (ptr) GrRRectShadowGeoProc(lutView);
        });
    }

    const char* name() const override { return "RRectShadow"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inShadowParams() const { return fInShadowParams; }
    GrColor color() const { return fColor; }

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    GrRRectShadowGeoProc(const GrSurfaceProxyView& lutView);

    const TextureSampler& onTextureSampler(int i) const override { return fLUTTextureSampler; }

    GrColor          fColor;
    TextureSampler   fLUTTextureSampler;

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInShadowParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

#endif
