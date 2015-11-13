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
                                       GrMaskFormat format, const SkMatrix& localMatrix,
                                       bool usesLocalCoords) {
        return new GrBitmapTextGeoProc(color, tex, p, format, localMatrix, usesLocalCoords);
    }

    virtual ~GrBitmapTextGeoProc() {}

    const char* name() const override { return "Texture"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inTextureCoords() const { return fInTextureCoords; }
    GrMaskFormat maskFormat() const { return fMaskFormat; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    bool hasVertexColor() const { return SkToBool(fInColor); }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps& caps) const override;

private:
    GrBitmapTextGeoProc(GrColor, GrTexture* texture, const GrTextureParams& params,
                        GrMaskFormat format, const SkMatrix& localMatrix, bool usesLocalCoords);

    GrColor          fColor;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;
    GrTextureAccess  fTextureAccess;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInTextureCoords;
    GrMaskFormat     fMaskFormat;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

#endif
