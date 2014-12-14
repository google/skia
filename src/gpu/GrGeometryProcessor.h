/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrColor.h"
#include "GrGeometryData.h"
#include "GrProcessor.h"
#include "GrShaderVar.h"

/*
 * A struct for tracking batching decisions.  While this lives on GrOptState, it is managed
 * entirely by the derived classes of the GP.
 */
class GrBatchTracker {
public:
    template <typename T> const T& cast() const {
        SkASSERT(sizeof(T) <= kMaxSize);
        return *reinterpret_cast<const T*>(fData);
    }

    template <typename T> T* cast() {
        SkASSERT(sizeof(T) <= kMaxSize);
        return reinterpret_cast<T*>(fData);
    }

    static const size_t kMaxSize = 32;

private:
    uint8_t fData[kMaxSize];
};

class GrGLCaps;
class GrGLGeometryProcessor;
class GrOptDrawState;

struct GrInitInvariantOutput;

/*
 * GrGeometryProcessors and GrPathProcessors may effect invariantColor
 */
class GrPrimitiveProcessor : public GrProcessor {
public:
    // TODO GPs and PPs have to provide an initial coverage because the coverage invariant code is
    // broken right now
    virtual uint8_t coverage() const = 0;
    virtual void getInvariantOutputColor(GrInitInvariantOutput* out) const = 0;
    virtual void getInvariantOutputCoverage(GrInitInvariantOutput* out) const = 0;

private:
    typedef GrProcessor INHERITED;
};

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
class GrGeometryProcessor : public GrPrimitiveProcessor {
public:
    // TODO the Hint can be handled in a much more clean way when we have deferred geometry or
    // atleast bundles
    GrGeometryProcessor(GrColor color, bool opaqueVertexColors = false, uint8_t coverage = 0xff)
        : fVertexStride(0)
        , fColor(color)
        , fCoverage(coverage)
        , fOpaqueVertexColors(opaqueVertexColors)
        , fWillUseGeoShader(false)
        , fHasVertexColor(false)
        , fHasVertexCoverage(false)
        , fHasLocalCoords(false) {}

    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this geometry
     * processor's GL backend implementation.
     */
    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const = 0;


    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const = 0;

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
        if (this->classID() != that.classID() || !this->hasSameTextureAccesses(that)) {
            return false;
        }

        // TODO remove the hint
        if (fHasVertexColor && fOpaqueVertexColors != that.fOpaqueVertexColors) {
            return false;
        }

        if (!fHasVertexColor && this->color() != that.color()) {
            return false;
        }

        // TODO this is fragile, most gps set their coverage to 0xff so this is okay.  In the long
        // term this should move to subclasses which set explicit coverage
        if (!fHasVertexCoverage && this->coverage() != that.coverage()) {
            return false;
        }
        return this->onIsEqual(that);
    }

    struct InitBT {
        bool fOutputColor;
        bool fOutputCoverage;
        GrColor fColor;
        GrColor fCoverage;
    };

    virtual void initBatchTracker(GrBatchTracker*, const InitBT&) const {}

    GrColor color() const { return fColor; }
    uint8_t coverage() const SK_OVERRIDE { return fCoverage; }

    // TODO this is a total hack until the gp can own whether or not it uses uniform
    // color / coverage
    bool hasVertexColor() const { return fHasVertexColor; }
    bool hasVertexCoverage() const { return fHasVertexCoverage; }
    bool hasLocalCoords() const { return fHasLocalCoords; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE;
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE;

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

    virtual void onGetInvariantOutputColor(GrInitInvariantOutput*) const {}
    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const = 0;

private:
    virtual bool onIsEqual(const GrGeometryProcessor&) const = 0;

    SkSTArray<kMaxVertexAttribs, GrAttribute, true> fAttribs;
    size_t fVertexStride;
    GrColor fColor;
    uint8_t fCoverage;
    bool fOpaqueVertexColors;
    bool fWillUseGeoShader;
    bool fHasVertexColor;
    bool fHasVertexCoverage;
    bool fHasLocalCoords;

    typedef GrProcessor INHERITED;
};

/*
 * The path equivalent of the GP.  For now this just manages color. In the long term we plan on
 * extending this class to handle all nvpr uniform / varying / program work.
 */
class GrPathProcessor : public GrPrimitiveProcessor {
public:
    static GrPathProcessor* Create(GrColor color) {
        return SkNEW_ARGS(GrPathProcessor, (color));
    }

    const char* name() const SK_OVERRIDE { return "PathProcessor"; }
    uint8_t coverage() const SK_OVERRIDE { return 0xff; }
    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE;
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE;

private:
    GrPathProcessor(GrColor color) : fColor(color) {}
    GrColor fColor;

    typedef GrProcessor INHERITED;
};
#endif
