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
    GrGeometryProcessor() {}

    virtual const GrBackendGeometryProcessorFactory& getFactory() const = 0;

    /*
     * This only has a max because GLProgramsTest needs to generate test arrays, and these have to
     * be static
     * TODO make this truly dynamic
     */
    static const int kMaxVertexAttribs = 2;
    typedef SkTArray<GrShaderVar, true> VertexAttribArray;

    const VertexAttribArray& getVertexAttribs() const { return fVertexAttribs; }

    /** Returns true if this and other effect conservatively draw identically. It can only return
        true when the two effects are of the same subclass (i.e. they return the same object from
        from getFactory()).
        A return value of true from isEqual() should not be used to test whether the effects would
        generate the same shader code. To test for identical code generation use the effects' keys
        computed by the GrBackendEffectFactory. */
    bool isEqual(const GrGeometryProcessor& that) const {
        if (&this->getFactory() != &that.getFactory()) {
            return false;
        }
        bool result = this->onIsEqual(that);
#ifdef SK_DEBUG
        if (result) {
            this->assertTexturesEqual(that);
        }
#endif
        return result;
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

private:
    virtual bool onIsEqual(const GrGeometryProcessor&) const = 0;

    SkSTArray<kMaxVertexAttribs, GrShaderVar, true> fVertexAttribs;

    typedef GrProcessor INHERITED;
};

/**
 * This creates an effect outside of the effect memory pool. The effect's destructor will be called
 * at global destruction time. NAME will be the name of the created GrProcessor.
 */
#define GR_CREATE_STATIC_GEOMETRY_PROCESSOR(NAME, GP_CLASS, ARGS)                                 \
static SkAlignedSStorage<sizeof(GP_CLASS)> g_##NAME##_Storage;                                    \
static GrGeometryProcessor* NAME SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), GP_CLASS, ARGS);  \
static SkAutoTDestroy<GrGeometryProcessor> NAME##_ad(NAME);

#endif
