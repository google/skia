/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrPrimitiveProcessor.h"

/**
 * A GrGeometryProcessor is a flexible method for rendering a primitive.  The GrGeometryProcessor
 * has complete control over vertex attributes and uniforms(aside from the render target) but it
 * must obey the same contract as any GrPrimitiveProcessor, specifically it must emit a color and
 * coverage into the fragment shader.  Where this color and coverage come from is completely the
 * responsibility of the GrGeometryProcessor.
 */
class GrGeometryProcessor : public GrPrimitiveProcessor {
public:
    GrGeometryProcessor(ClassID classID)
        : INHERITED(classID)
        , fWillUseGeoShader(false)
        , fSampleShading(0.0) {}

    bool willUseGeoShader() const final { return fWillUseGeoShader; }

    /**
     * Returns the minimum fraction of samples for which the fragment shader will be run. For
     * instance, if sampleShading is 0.5 in MSAA16 mode, the fragment shader will run a minimum of
     * 8 times per pixel. The default value is zero.
     */
    float getSampleShading() const final { return fSampleShading; }

protected:
    void setWillUseGeoShader() { fWillUseGeoShader = true; }
    void setSampleShading(float sampleShading) {
        fSampleShading = sampleShading;
    }

private:
    bool fWillUseGeoShader;
    float fSampleShading;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
