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
    static sk_sp<GrGeometryProcessor> Make(bool stroke, const SkMatrix& localMatrix) {
        return sk_sp<GrGeometryProcessor>(new GrRRectShadowGeoProc(stroke, localMatrix));
    }

    virtual ~GrRRectShadowGeoProc() {}

    const char* name() const override { return "ShadowEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inCircleEdge() const { return fInCircleEdge; }
//    const Attribute* inShadowParams() const { return fInShadowParams; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool stroked() const { return fStroke; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

    bool implementsDistanceVector() const override { return true; }

private:
    GrRRectShadowGeoProc(bool stroke, const SkMatrix& localMatrix);

    GrColor          fColor;
    SkMatrix         fLocalMatrix;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInCircleEdge;
//    const Attribute* fInShadowParams;
    bool             fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};


#endif
