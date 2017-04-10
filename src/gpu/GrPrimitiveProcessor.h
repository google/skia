/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPrimitiveProcessor_DEFINED
#define GrPrimitiveProcessor_DEFINED

#include "GrColor.h"
#include "GrProcessor.h"
#include "GrShaderVar.h"

/*
 * The GrPrimitiveProcessor represents some kind of geometric primitive.  This includes the shape
 * of the primitive and the inherent color of the primitive.  The GrPrimitiveProcessor is
 * responsible for providing a color and coverage input into the Ganesh rendering pipeline.  Through
 * optimization, Ganesh may decide a different color, no color, and / or no coverage are required
 * from the GrPrimitiveProcessor, so the GrPrimitiveProcessor must be able to support this
 * functionality.
 *
 * There are two feedback loops between the GrFragmentProcessors, the GrXferProcessor, and the
 * GrPrimitiveProcessor. These loops run on the CPU and to determine known properties of the final
 * color and coverage inputs to the GrXferProcessor in order to perform optimizations that preserve
 * correctness. The GrDrawOp seeds these loops with initial color and coverage, in its
 * getProcessorAnalysisInputs implementation. These seed values are processed by the
 * subsequent
 * stages of the rendering pipeline and the output is then fed back into the GrDrawOp in
 * the applyPipelineOptimizations call, where the op can use the information to inform decisions
 * about GrPrimitiveProcessor creation.
 */

class GrGLSLPrimitiveProcessor;

/*
 * GrPrimitiveProcessor defines an interface which all subclasses must implement.  All
 * GrPrimitiveProcessors must proivide seed color and coverage for the Ganesh color / coverage
 * pipelines, and they must provide some notion of equality
 */
class GrPrimitiveProcessor : public GrResourceIOProcessor, public GrProgramElement {
public:
    // Only the GrGeometryProcessor subclass actually has a geo shader or vertex attributes, but
    // we put these calls on the base class to prevent having to cast
    virtual bool willUseGeoShader() const = 0;

    struct Attribute {
        Attribute()
            : fName(nullptr)
            , fType(kFloat_GrVertexAttribType)
            , fOffset(0) {}
        Attribute(const char* name, GrVertexAttribType type, GrSLPrecision precision)
            : fName(name)
            , fType(type)
            , fOffset(SkAlign4(GrVertexAttribTypeSize(type)))
            , fPrecision(precision) {}
        const char* fName;
        GrVertexAttribType fType;
        size_t fOffset;
        GrSLPrecision fPrecision;
    };

    int numAttribs() const { return fAttribs.count(); }
    const Attribute& getAttrib(int index) const { return fAttribs[index]; }

    // Returns the vertex stride of the GP.  A common use case is to request geometry from a
    // GrOpList based off of the stride, and to populate this memory using an implicit array of
    // structs.  In this case, it is best to assert the vertexstride == sizeof(VertexStruct).
    size_t getVertexStride() const { return fVertexStride; }

    /**
     * Computes a transformKey from an array of coord transforms. Will only look at the first
     * <numCoords> transforms in the array.
     *
     * TODO: A better name for this function  would be "compute" instead of "get".
     */
    uint32_t getTransformKey(const SkTArray<const GrCoordTransform*, true>& coords,
                             int numCoords) const;

    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this geometry
     * processor's GL backend implementation.
     *
     * TODO: A better name for this function  would be "compute" instead of "get".
     */
    virtual void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;


    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const = 0;

    virtual bool isPathRendering() const { return false; }

    /**
     * If non-null, overrides the dest color returned by GrGLSLFragmentShaderBuilder::dstColor().
     */
    virtual const char* getDestColorOverride() const { return nullptr; }

    virtual float getSampleShading() const {
        return 0.0;
    }

    /* Sub-class should override and return true if this primitive processor implements the distance
     * vector field, a field of vectors to the nearest point in the edge of the shape.  */
    virtual bool implementsDistanceVector() const { return false; }

protected:
    GrPrimitiveProcessor() : fVertexStride(0) {}

    enum { kPreallocAttribCnt = 8 };
    SkSTArray<kPreallocAttribCnt, Attribute> fAttribs;
    size_t fVertexStride;

private:
    void addPendingIOs() const override { GrResourceIOProcessor::addPendingIOs(); }
    void removeRefs() const override { GrResourceIOProcessor::removeRefs(); }
    void pendingIOComplete() const override { GrResourceIOProcessor::pendingIOComplete(); }
    void notifyRefCntIsZero() const final {}
    virtual bool hasExplicitLocalCoords() const = 0;

    typedef GrProcessor INHERITED;
};

#endif
