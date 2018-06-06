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
#include <cassert>

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
    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        constexpr Attribute() = default;
        constexpr Attribute(const char* name, GrVertexAttribType type, size_t offset = 0)
                : fName(name), fType(type), fOffset(offset) {}
        /** Initialized to be at the next 4 byte offset after 'prev'. */
        constexpr Attribute(const char* name, GrVertexAttribType type, const Attribute& prev)
                : fName(name), fType(type), fOffset(prev.nextOffsetInRecord()) {
            assert(prev.isInitialized());
        }
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return SkToBool(fName); }

        constexpr const char* name() const { return fName; }
        constexpr GrVertexAttribType type() const { return fType; }
        constexpr size_t offset() const { return fOffset; }

        constexpr size_t size() const { return GrVertexAttribTypeSize(fType); }
        constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }
        constexpr size_t nextOffsetInRecord() const { return fOffset + this->sizeAlign4(); }

        GrShaderVar asShaderVar() const {
            return {fName, GrVertexAttribTypeToSLType(fType), GrShaderVar::kIn_TypeModifier};
        }

    private:
        const char* fName = nullptr;
        GrVertexAttribType fType = kFloat_GrVertexAttribType;
        size_t fOffset = 0;
    };

    GrPrimitiveProcessor(ClassID);

    int numVertexAttributes() const { return fVertexAttributeCnt; }
    const Attribute& vertexAttribute(int i) const;
    int numInstanceAttributes() const { return fInstanceAttributeCnt; }
    const Attribute& instanceAttribute(int i) const;

    bool hasVertexAttributes() const { return SkToBool(fVertexAttributeCnt); }
    bool hasInstanceAttributes() const { return SkToBool(fInstanceAttributeCnt); }

    /**
     * These return the strides of the vertex and instance buffers. Attributes are expected to be
     * laid out interleaved in their corresponding buffer (vertex or instance). fOffsetInRecord
     * indicates an attribute's location in bytes relative to the first attribute. (These are padded
     * to the nearest 4 bytes for performance reasons.)
     *
     * A common practice is to populate the buffer's memory using an implicit array of structs. In
     * this case, it is best to assert:
     *
     *     stride == sizeof(struct) and
     *     offsetof(struct, field[i]) == attrib[i].fOffsetInRecord
     *
     * NOTE: for instanced draws the vertex buffer has a single record that each instance reuses.
     */
    int getVertexStride() const { return fVertexStride; }
    int getInstanceStride() const { return fInstanceStride; }

    // Only the GrGeometryProcessor subclass actually has a geo shader or vertex attributes, but
    // we put these calls on the base class to prevent having to cast
    virtual bool willUseGeoShader() const = 0;

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

protected:
    void setVertexAttributeInfo(int attributeCnt, size_t vertexStride);
    void setInstanceAttributeInfo(int attributeCnt, size_t instanceStride);

private:
    void addPendingIOs() const override { GrResourceIOProcessor::addPendingIOs(); }
    void removeRefs() const override { GrResourceIOProcessor::removeRefs(); }
    void pendingIOComplete() const override { GrResourceIOProcessor::pendingIOComplete(); }
    void notifyRefCntIsZero() const final {}
    virtual bool hasExplicitLocalCoords() const = 0;

    virtual const Attribute& onVertexAttribute(int) const = 0;
    virtual const Attribute& onInstanceAttribute(int) const = 0;

    unsigned fVertexStride : 8;
    unsigned fInstanceStride : 8;
    unsigned fVertexAttributeCnt : 4;
    unsigned fInstanceAttributeCnt : 4;
    typedef GrProcessor INHERITED;
};

#endif
