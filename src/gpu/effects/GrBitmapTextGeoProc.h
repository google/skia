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
                                       GrMaskFormat format, bool opaqueVertexColors,
                                       const SkMatrix& localMatrix) {
        return SkNEW_ARGS(GrBitmapTextGeoProc, (color, tex, p, format, opaqueVertexColors,
                                                localMatrix));
    }

    virtual ~GrBitmapTextGeoProc() {}

    const char* name() const override { return "Texture"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inTextureCoords() const { return fInTextureCoords; }
    GrMaskFormat maskFormat() const { return fMaskFormat; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override;

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps& caps) const override;

    void initBatchTracker(GrBatchTracker*, const GrPipelineInfo&) const override;
    bool onCanMakeEqual(const GrBatchTracker&,
                        const GrGeometryProcessor&,
                        const GrBatchTracker&) const override;

private:
    GrBitmapTextGeoProc(GrColor, GrTexture* texture, const GrTextureParams& params,
                        GrMaskFormat format, bool opaqueVertexColors, const SkMatrix& localMatrix);

    bool onIsEqual(const GrGeometryProcessor& other) const override;

    void onGetInvariantOutputColor(GrInitInvariantOutput*) const override;

    void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const override;

    GrTextureAccess  fTextureAccess;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInTextureCoords;
    GrMaskFormat     fMaskFormat;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

#endif
