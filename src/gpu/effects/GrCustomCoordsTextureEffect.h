/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomCoordsTextureEffect_DEFINED
#define GrCustomCoordsTextureEffect_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLCustomCoordsTextureEffect;
class GrInvariantOutput;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute.
 */
class GrCustomCoordsTextureEffect : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(GrTexture* tex, const GrTextureParams& p, bool hasColor) {
        return SkNEW_ARGS(GrCustomCoordsTextureEffect, (tex, p, hasColor));
    }

    virtual ~GrCustomCoordsTextureEffect() {}

    virtual const char* name() const SK_OVERRIDE { return "Texture"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inColor() const { return fInColor; }
    const GrAttribute* inTextureCoords() const { return fInTextureCoords; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE;

private:
    GrCustomCoordsTextureEffect(GrTexture* texture, const GrTextureParams& params, bool hasColor);

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE;

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GrTextureAccess    fTextureAccess;
    const GrAttribute* fInPosition;
    const GrAttribute* fInColor;
    const GrAttribute* fInTextureCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

#endif
