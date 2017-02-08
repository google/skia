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
    static sk_sp<GrGeometryProcessor> Make(const SkMatrix& localMatrix) {
        return sk_sp<GrGeometryProcessor>(new GrRRectShadowGeoProc(localMatrix));
    }

    ~GrRRectShadowGeoProc() override {}

    const char* name() const override { return "RRectShadow"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inShadowParams() const { return fInShadowParams; }
    GrColor color() const { return fColor; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrRRectShadowGeoProc(const SkMatrix& localMatrix);

    GrColor          fColor;
    SkMatrix         fLocalMatrix;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInShadowParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};


#endif
