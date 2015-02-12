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
    // TODO the Hint can be handled in a much more clean way when we have deferred geometry or
    // atleast bundles
    GrGeometryProcessor(GrColor color,
                        const SkMatrix& viewMatrix = SkMatrix::I(),
                        const SkMatrix& localMatrix = SkMatrix::I(),
                        bool opaqueVertexColors = false)
        : INHERITED(viewMatrix, localMatrix, false)
        , fColor(color)
        , fOpaqueVertexColors(opaqueVertexColors)
        , fWillUseGeoShader(false)
        , fHasVertexColor(false)
        , fHasLocalCoords(false) {}

    bool willUseGeoShader() const { return fWillUseGeoShader; }

    /*
     * In an ideal world, two GrGeometryProcessors with the same class id and texture accesses
     * would ALWAYS be able to batch together.  If two GrGeometryProcesosrs are the same then we
     * will only keep one of them.  The remaining GrGeometryProcessor then updates its
     * GrBatchTracker to incorporate the draw information from the GrGeometryProcessor we discard.
     * Any bundles associated with the discarded GrGeometryProcessor will be attached to the
     * remaining GrGeometryProcessor.
     */
    bool canMakeEqual(const GrBatchTracker& mine,
                      const GrPrimitiveProcessor& that,
                      const GrBatchTracker& theirs) const SK_OVERRIDE {
        if (this->classID() != that.classID() || !this->hasSameTextureAccesses(that)) {
            return false;
        }

        // TODO let the GPs decide this
        if (!this->viewMatrix().cheapEqualTo(that.viewMatrix())) {
            return false;
        }

        // TODO remove the hint
        const GrGeometryProcessor& other = that.cast<GrGeometryProcessor>();
        if (fHasVertexColor && fOpaqueVertexColors != other.fOpaqueVertexColors) {
            return false;
        }

        // TODO this equality test should really be broken up, some of this can live on the batch
        // tracker test and some of this should be in bundles
        if (!this->onIsEqual(other)) {
            return false;
        }

        return this->onCanMakeEqual(mine, other, theirs);
    }
    
    // TODO we can remove color from the GrGeometryProcessor base class once we have bundles of
    // primitive data
    GrColor color() const { return fColor; }

    // TODO this is a total hack until the gp can do deferred geometry
    bool hasVertexColor() const { return fHasVertexColor; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE;
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE;

protected:
    /*
     * An optional simple helper function to determine by what means the GrGeometryProcessor should
     * use to provide color.  If we are given an override color(ie the given overridecolor is NOT
     * GrColor_ILLEGAL) then we must always emit that color(currently overrides are only supported
     * via uniform, but with deferred Geometry we could use attributes).  Otherwise, if our color is
     * ignored then we should not emit a color.  Lastly, if we don't have vertex colors then we must
     * emit a color via uniform
     * TODO this function changes quite a bit with deferred geometry.  There the GrGeometryProcessor
     * can upload a new color via attribute if needed.
     */
    static GrGPInput GetColorInputType(GrColor* color, GrColor primitiveColor,
                                       const GrPipelineInfo& init,
                                       bool hasVertexColor) {
        if (init.fColorIgnored) {
            *color = GrColor_ILLEGAL;
            return kIgnored_GrGPInput;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            *color = init.fOverrideColor;
            return kUniform_GrGPInput;
        }

        *color = primitiveColor;
        if (hasVertexColor) {
            return kAttribute_GrGPInput;
        } else {
            return kUniform_GrGPInput;
        }
    }

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
    void setHasVertexColor() { fHasVertexColor = true; }
    void setHasLocalCoords() { fHasLocalCoords = true; }

    virtual void onGetInvariantOutputColor(GrInitInvariantOutput*) const {}
    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const = 0;

private:
    virtual bool onCanMakeEqual(const GrBatchTracker& mine,
                                const GrGeometryProcessor& that,
                                const GrBatchTracker& theirs) const = 0;

    // TODO delete this when we have more advanced equality testing via bundles and the BT
    virtual bool onIsEqual(const GrGeometryProcessor&) const = 0;

    bool hasExplicitLocalCoords() const SK_OVERRIDE { return fHasLocalCoords; }

    GrColor fColor;
    bool fOpaqueVertexColors;
    bool fWillUseGeoShader;
    bool fHasVertexColor;
    bool fHasLocalCoords;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
