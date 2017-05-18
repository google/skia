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
    GrGeometryProcessor()
        : fWillUseGeoShader(false)
        , fLocalCoordsType(kUnused_LocalCoordsType)
        , fSampleShading(0.0) {}

    bool willUseGeoShader() const override { return fWillUseGeoShader; }

    bool hasExplicitLocalCoords() const override {
        return kHasExplicit_LocalCoordsType == fLocalCoordsType;
    }

    /**
     * Returns the minimum fraction of samples for which the fragment shader will be run. For
     * instance, if sampleShading is 0.5 in MSAA16 mode, the fragment shader will run a minimum of
     * 8 times per pixel. The default value is zero.
     */
    float getSampleShading() const override {
        return fSampleShading;
    }

protected:
    /**
     * Subclasses call this from their constructor to register vertex attributes.  Attributes
     * will be padded to the nearest 4 bytes for performance reasons.
     * TODO After deferred geometry, we should do all of this inline in GenerateGeometry alongside
     * the struct used to actually populate the attributes.  This is all extremely fragile, vertex
     * attributes have to be added in the order they will appear in the struct which maps memory.
     * The processor key should reflect the vertex attributes, or there lack thereof in the
     * GrGeometryProcessor.
     */
    const Attribute& addVertexAttrib(const char* name, GrVertexAttribType type,
                                     GrSLPrecision precision = kDefault_GrSLPrecision) {
        precision = (kDefault_GrSLPrecision == precision) ? kMedium_GrSLPrecision : precision;
        fAttribs.emplace_back(name, type, precision);
        fVertexStride += fAttribs.back().fOffset;
        return fAttribs.back();
    }

    void setWillUseGeoShader() { fWillUseGeoShader = true; }

    /**
     * If a GrFragmentProcessor in the GrPipeline needs localCoods, we will provide them in one of
     * three ways
     * 1) LocalCoordTransform * Position - in Shader
     * 2) LocalCoordTransform * ExplicitLocalCoords- in Shader
     * 3) A transformation on the CPU uploaded via vertex attribute
     */
    enum LocalCoordsType {
        kUnused_LocalCoordsType,
        kHasExplicit_LocalCoordsType,
        kHasTransformed_LocalCoordsType
    };

    void setHasExplicitLocalCoords() {
        SkASSERT(kUnused_LocalCoordsType == fLocalCoordsType);
        fLocalCoordsType = kHasExplicit_LocalCoordsType;
    }
    void setHasTransformedLocalCoords() {
        SkASSERT(kUnused_LocalCoordsType == fLocalCoordsType);
        fLocalCoordsType = kHasTransformed_LocalCoordsType;
    }

    void setSampleShading(float sampleShading) {
        fSampleShading = sampleShading;
    }

private:
    bool fWillUseGeoShader;
    LocalCoordsType fLocalCoordsType;
    float fSampleShading;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
