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
    struct Attribute {
        enum class InputRate : bool {
            kPerVertex,
            kPerInstance
        };

        const char*          fName;
        GrVertexAttribType   fType;
        int                  fOffsetInRecord;
        GrSLPrecision        fPrecision;
        InputRate            fInputRate;
    };

    int numAttribs() const { return fAttribs.count(); }
    const Attribute& getAttrib(int index) const { return fAttribs[index]; }

    bool hasVertexAttribs() const { return SkToBool(fVertexStride); }
    bool hasInstanceAttribs() const { return SkToBool(fInstanceStride); }

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
    /**
     * Subclasses call these from their constructor to register vertex and instance attributes.
     */
    const Attribute& addVertexAttrib(const char* name, GrVertexAttribType type,
                                     GrSLPrecision precision = kDefault_GrSLPrecision) {
        precision = (kDefault_GrSLPrecision == precision) ? kMedium_GrSLPrecision : precision;
        fAttribs.push_back() = {name, type, fVertexStride, precision,
                                Attribute::InputRate::kPerVertex};
        fVertexStride += static_cast<int>(SkAlign4(GrVertexAttribTypeSize(type)));
        return fAttribs.back();
    }
    const Attribute& addInstanceAttrib(const char* name, GrVertexAttribType type,
                                       GrSLPrecision precision = kDefault_GrSLPrecision) {
        precision = (kDefault_GrSLPrecision == precision) ? kMedium_GrSLPrecision : precision;
        fAttribs.push_back() = {name, type, fInstanceStride, precision,
                                Attribute::InputRate::kPerInstance};
        fInstanceStride += static_cast<int>(SkAlign4(GrVertexAttribTypeSize(type)));
        return fAttribs.back();
    }

private:
    void addPendingIOs() const override { GrResourceIOProcessor::addPendingIOs(); }
    void removeRefs() const override { GrResourceIOProcessor::removeRefs(); }
    void pendingIOComplete() const override { GrResourceIOProcessor::pendingIOComplete(); }
    void notifyRefCntIsZero() const final {}
    virtual bool hasExplicitLocalCoords() const = 0;

    SkSTArray<8, Attribute>   fAttribs;
    int                       fVertexStride = 0;
    int                       fInstanceStride = 0;

    typedef GrProcessor INHERITED;
};

#endif
