/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrProcessor.h"
#include "GrShaderVar.h"

/**
 * A GrGeomteryProcessor is used to perform computation in the vertex shader and
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
        : fWillUseGeoShader(false) {}

    virtual const GrBackendGeometryProcessorFactory& getFactory() const = 0;

    /*
     * This only has a max because GLProgramsTest needs to generate test arrays, and these have to
     * be static
     * TODO make this truly dynamic
     */
    static const int kMaxVertexAttribs = 2;
    typedef SkTArray<GrShaderVar, true> VertexAttribArray;

    const VertexAttribArray& getVertexAttribs() const { return fVertexAttribs; }

    bool willUseGeoShader() const { return fWillUseGeoShader; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two prcoessors are of the same subclass (i.e. they return the same object from
        from getFactory()).
        A return value of true from isEqual() should not be used to test whether the prcoessors
        would generate the same shader code. To test for identical code generation use the
        processors' keys computed by the GrBackendEffectFactory. */
    bool isEqual(const GrGeometryProcessor& that) const {
        if (&this->getFactory() != &that.getFactory() || !this->hasSameTextureAccesses(that)) {
            return false;
        }
        return this->onIsEqual(that);
    }

protected:
    /**
     * Subclasses call this from their constructor to register vertex attributes (at most
     * kMaxVertexAttribs). This must only be called from the constructor because GrProcessors are
     * immutable.
     */
    const GrShaderVar& addVertexAttrib(const GrShaderVar& var) {
        SkASSERT(fVertexAttribs.count() < kMaxVertexAttribs);
        return fVertexAttribs.push_back(var);
    }

    void setWillUseGeoShader() { fWillUseGeoShader = true; }

private:
    virtual bool onIsEqual(const GrGeometryProcessor&) const = 0;

    SkSTArray<kMaxVertexAttribs, GrShaderVar, true> fVertexAttribs;
    bool fWillUseGeoShader;

    typedef GrProcessor INHERITED;
};

#endif
