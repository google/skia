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

    const Attribute& inPosition() const { return kAttributes[0]; }
    const Attribute& inColor() const { return kAttributes[1]; }
    const Attribute& inShadowParams() const { return kAttributes[2]; }
    GrColor color() const { return fColor; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrRRectShadowGeoProc();

    GrColor          fColor;

    static constexpr Attribute kAttributes[] = {
        {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType},
        {"inColor", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType},
        {"inShadowParams", kFloat3_GrVertexAttribType, kHalf3_GrSLType},
    };

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};


#endif
