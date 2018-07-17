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
    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        constexpr Attribute() = default;
        constexpr Attribute(const char* name, GrVertexAttribType type) : fName(name), fType(type) {}
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return SkToBool(fName); }

        constexpr const char* name() const { return fName; }
        constexpr GrVertexAttribType type() const { return fType; }

        inline constexpr size_t size() const;
        constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

        GrShaderVar asShaderVar() const {
            return {fName, GrVertexAttribTypeToSLType(fType), GrShaderVar::kIn_TypeModifier};
        }

    private:
        const char* fName = nullptr;
        GrVertexAttribType fType = kFloat_GrVertexAttribType;
    };

    GrPrimitiveProcessor(ClassID);

    int numVertexAttributes() const { return fVertexAttributeCnt; }
    const Attribute& vertexAttribute(int i) const;
    int numInstanceAttributes() const { return fInstanceAttributeCnt; }
    const Attribute& instanceAttribute(int i) const;

    bool hasVertexAttributes() const { return SkToBool(fVertexAttributeCnt); }
    bool hasInstanceAttributes() const { return SkToBool(fInstanceAttributeCnt); }

#ifdef SK_DEBUG
    /**
     * A common practice is to populate the the vertex/instance's memory using an implicit array of
     * structs. In this case, it is best to assert that:
     *     debugOnly_stride == sizeof(struct) and
     *     offsetof(struct, field[i]) == debugOnly_AttributeOffset(i)
     * In general having Op subclasses assert that attribute offsets and strides agree with their
     * tessellation code's expectations is good practice.
     * However, these functions walk the attributes to compute offsets and call virtual functions
     * to access the attributes. Thus, they are only available in debug builds.
     */
    size_t debugOnly_vertexStride() const;
    size_t debugOnly_instanceStride() const;
    size_t debugOnly_vertexAttributeOffset(int) const;
    size_t debugOnly_instanceAttributeOffset(int) const;
#endif

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

    virtual float getSampleShading() const { return 0.0; }

protected:
    void setVertexAttributeCnt(int cnt) { fVertexAttributeCnt = cnt; }
    void setInstanceAttributeCnt(int cnt) { fInstanceAttributeCnt = cnt; }

private:
    void addPendingIOs() const override { GrResourceIOProcessor::addPendingIOs(); }
    void removeRefs() const override { GrResourceIOProcessor::removeRefs(); }
    void pendingIOComplete() const override { GrResourceIOProcessor::pendingIOComplete(); }
    void notifyRefCntIsZero() const final {}

    virtual const Attribute& onVertexAttribute(int) const = 0;
    virtual const Attribute& onInstanceAttribute(int) const = 0;

    int fVertexAttributeCnt = 0;
    int fInstanceAttributeCnt = 0;
    typedef GrProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Returns the size of the attrib type in bytes.
 * This was moved from include/private/GrTypesPriv.h in service of Skia dependents that build
 * with C++11.
 */
static constexpr inline size_t GrVertexAttribTypeSize(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return sizeof(float);
        case kFloat2_GrVertexAttribType:
            return 2 * sizeof(float);
        case kFloat3_GrVertexAttribType:
            return 3 * sizeof(float);
        case kFloat4_GrVertexAttribType:
            return 4 * sizeof(float);
        case kHalf_GrVertexAttribType:
            return sizeof(float);
        case kHalf2_GrVertexAttribType:
            return 2 * sizeof(float);
        case kHalf3_GrVertexAttribType:
            return 3 * sizeof(float);
        case kHalf4_GrVertexAttribType:
            return 4 * sizeof(float);
        case kInt2_GrVertexAttribType:
            return 2 * sizeof(int32_t);
        case kInt3_GrVertexAttribType:
            return 3 * sizeof(int32_t);
        case kInt4_GrVertexAttribType:
            return 4 * sizeof(int32_t);
        case kByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kByte2_GrVertexAttribType:
            return 2 * sizeof(char);
        case kByte3_GrVertexAttribType:
            return 3 * sizeof(char);
        case kByte4_GrVertexAttribType:
            return 4 * sizeof(char);
        case kUByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte2_GrVertexAttribType:
            return 2 * sizeof(char);
        case kUByte3_GrVertexAttribType:
            return 3 * sizeof(char);
        case kUByte4_GrVertexAttribType:
            return 4 * sizeof(char);
        case kUByte_norm_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte4_norm_GrVertexAttribType:
            return 4 * sizeof(char);
        case kShort2_GrVertexAttribType:
            return 2 * sizeof(int16_t);
        case kUShort2_GrVertexAttribType: // fall through
        case kUShort2_norm_GrVertexAttribType:
            return 2 * sizeof(uint16_t);
        case kInt_GrVertexAttribType:
            return sizeof(int32_t);
        case kUint_GrVertexAttribType:
            return sizeof(uint32_t);
    }
    // GCC fails because SK_ABORT evaluates to non constexpr. clang and cl.exe think this is
    // unreachable and don't complain.
#if defined(__clang__) || !defined(__GNUC__)
    SK_ABORT("Unsupported type conversion");
#endif
    return 0;
}

constexpr size_t GrPrimitiveProcessor::Attribute::size() const {
    return GrVertexAttribTypeSize(fType);
}

#endif
