/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrGeometryData.h"
#include "GrProcessor.h"
#include "GrShaderVar.h"

/**
 * A GrGeometryProcessor is used to perform computation in the vertex shader and
 * add support for custom vertex attributes. A GrGemeotryProcessor is typically
 * tied to the code that does a specific type of high-level primitive rendering
 * (e.g. anti-aliased circle rendering). The GrGeometryProcessor used for a draw is
 * specified using GrDrawState. There can only be one geometry processor active for
 * a draw. The custom vertex attributes required by the geometry processor must be
 * added to the vertex attribute array specified on the GrDrawState.
 * GrGeometryProcessor subclasses should be immutable after construction.
 */
class GrGeometryProcessor : public GrProcessor {
public:
    GrGeometryProcessor()
        : fVertexStride(0)
        , fWillUseGeoShader(false)
        , fHasVertexColor(false)
        , fHasVertexCoverage(false)
        , fHasLocalCoords(false) {}

    virtual const GrBackendGeometryProcessorFactory& getFactory() const = 0;

    /*
     * This is a safeguard to prevent GPs from going beyond platform specific attribute limits.
     * This number can almost certainly be raised if required.
     */
    static const int kMaxVertexAttribs = 6;

    struct GrAttribute {
        GrAttribute(const char* name, GrVertexAttribType type)
            : fName(name)
            , fType(type)
            , fOffset(SkAlign4(GrVertexAttribTypeSize(type))) {}
        const char* fName;
        GrVertexAttribType fType;
        size_t fOffset;
    };

    typedef SkTArray<GrAttribute, true> VertexAttribArray;

    const VertexAttribArray& getAttribs() const { return fAttribs; }

    // Returns the vertex stride of the GP.  A common use case is to request geometry from a
    // drawtarget based off of the stride, and to populate this memory using an implicit array of
    // structs.  In this case, it is best to assert the vertexstride == sizeof(VertexStruct).
    size_t getVertexStride() const { return fVertexStride; }

    bool willUseGeoShader() const { return fWillUseGeoShader; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two prcoessors are of the same subclass (i.e. they return the same object from
        from getFactory()).
        A return value of true from isEqual() should not be used to test whether the processors
        would generate the same shader code. To test for identical code generation use the
        processors' keys computed by the GrBackendEffectFactory. */
    bool isEqual(const GrGeometryProcessor& that) const {
        if (&this->getFactory() != &that.getFactory() || !this->hasSameTextureAccesses(that)) {
            return false;
        }
        return this->onIsEqual(that);
    }

    // TODO this is a total hack until the gp can own whether or not it uses uniform
    // color / coverage
    bool hasVertexColor() const { return fHasVertexColor; }
    bool hasVertexCoverage() const { return fHasVertexCoverage; }
    bool hasLocalCoords() const { return fHasLocalCoords; }

protected:
    /**
     * Subclasses call this from their constructor to register vertex attributes.  Attributes
     * will be padded to the nearest 4 bytes for performance reasons.
     * TODO After deferred geometry, we should do all of this inline in GenerateGeometry alongside
     * the struct used to actually populate the attributes
     */
    const GrAttribute& addVertexAttrib(const GrAttribute& attribute) {
        fVertexStride += attribute.fOffset;
        return fAttribs.push_back(attribute);
    }

    void setWillUseGeoShader() { fWillUseGeoShader = true; }

    // TODO hack see above
    void setHasVertexColor() { fHasVertexColor = true; }
    void setHasVertexCoverage() { fHasVertexCoverage = true; }
    void setHasLocalCoords() { fHasLocalCoords = true; }

private:
    virtual bool onIsEqual(const GrGeometryProcessor&) const = 0;

    SkSTArray<kMaxVertexAttribs, GrAttribute, true> fAttribs;
    size_t fVertexStride;
    bool fWillUseGeoShader;
    bool fHasVertexColor;
    bool fHasVertexCoverage;
    bool fHasLocalCoords;

    typedef GrProcessor INHERITED;
};

#endif
