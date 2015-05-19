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
        : INHERITED(false)
        , fWillUseGeoShader(false)
        , fHasLocalCoords(false) {}

    bool willUseGeoShader() const { return fWillUseGeoShader; }

    // TODO delete when paths are in batch
    void initBatchTracker(GrBatchTracker*, const GrPipelineInfo&) const override {}

    // TODO delete this when paths are in batch
    bool canMakeEqual(const GrBatchTracker& mine,
                      const GrPrimitiveProcessor& that,
                      const GrBatchTracker& theirs) const override {
        SkFAIL("Unsupported\n");
        return false;
    }

    // TODO Delete when paths are in batch
    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        SkFAIL("Unsupported\n");
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        SkFAIL("Unsupported\n");
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
    const Attribute& addVertexAttrib(const Attribute& attribute) {
        SkASSERT(fNumAttribs < kMaxVertexAttribs);
        fVertexStride += attribute.fOffset;
        fAttribs[fNumAttribs] = attribute;
        return fAttribs[fNumAttribs++];
    }

    void setWillUseGeoShader() { fWillUseGeoShader = true; }

    // TODO hack see above
    void setHasLocalCoords() { fHasLocalCoords = true; }

private:
    bool hasExplicitLocalCoords() const override { return fHasLocalCoords; }

    bool fWillUseGeoShader;
    bool fHasLocalCoords;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
