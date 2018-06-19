/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowGeoProc_DEFINED
#define GrShadowGeoProc_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLRRectShadowGeoProc;

/**
 * The output color of this effect is a coverage mask for a rrect shadow,
 * assuming circular corner geometry.
 */
class GrRRectShadowGeoProc : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make() {
        return sk_sp<GrGeometryProcessor>(new GrRRectShadowGeoProc());
    }

    const char* name() const override { return "RRectShadow"; }

    const Attribute& inPosition() const { return kInPosition; }
    const Attribute& inColor() const { return kInColor; }
    const Attribute& inShadowParams() const { return kInShadowParams; }
    GrColor color() const { return fColor; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrRRectShadowGeoProc();

    const Attribute& onVertexAttribute(int i) const override {
        return IthAttribute(i, kInPosition, kInColor, kInShadowParams);
    }

    GrColor          fColor;

    static constexpr Attribute kInPosition = {"inPosition", kFloat2_GrVertexAttribType};
    static constexpr Attribute kInColor = {"inColor", kUByte4_norm_GrVertexAttribType};
    static constexpr Attribute kInShadowParams = {"inShadowParams", kHalf4_GrVertexAttribType};

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};


#endif
