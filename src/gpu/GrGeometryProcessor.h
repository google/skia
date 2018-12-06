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
        , fWillUseGeoShader(false) {}

    bool willUseGeoShader() const final { return fWillUseGeoShader; }

protected:
    void setWillUseGeoShader() { fWillUseGeoShader = true; }

    // GPs that need to use either half-float or ubyte colors can just call this to get a correctly
    // configured Attribute struct
    static Attribute MakeColorAttribute(const char* name, bool wideColor) {
        return { name,
                 wideColor ? kHalf4_GrVertexAttribType : kUByte4_norm_GrVertexAttribType,
                 kHalf4_GrSLType };
    }

private:
    bool fWillUseGeoShader;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
