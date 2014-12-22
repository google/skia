/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextGeoProc_DEFINED
#define GrBitmapTextGeoProc_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLBitmapTextGeoProc;
class GrInvariantOutput;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute.
 */
class GrBitmapTextGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(GrColor color, GrTexture* tex, const GrTextureParams& p,
                                       bool useColorAttrib, bool opaqueVertexColors,
                                       const SkMatrix& localMatrix) {
        return SkNEW_ARGS(GrBitmapTextGeoProc, (color, tex, p, useColorAttrib, opaqueVertexColors,
                                                localMatrix));
    }

    virtual ~GrBitmapTextGeoProc() {}

    virtual const char* name() const SK_OVERRIDE { return "Texture"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inColor() const { return fInColor; }
    const GrAttribute* inTextureCoords() const { return fInTextureCoords; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE;

    void initBatchTracker(GrBatchTracker*, const InitBT&) const SK_OVERRIDE;
    bool onCanMakeEqual(const GrBatchTracker&,
                        const GrGeometryProcessor&,
                        const GrBatchTracker&) const SK_OVERRIDE;

private:
    GrBitmapTextGeoProc(GrColor, GrTexture* texture, const GrTextureParams& params,
                        bool useColorAttrib, bool opaqueVertexColors, const SkMatrix& localMatrix);

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE;

    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const SK_OVERRIDE;

    GrTextureAccess    fTextureAccess;
    const GrAttribute* fInPosition;
    const GrAttribute* fInColor;
    const GrAttribute* fInTextureCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

#endif
