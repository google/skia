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
 * functionality.  We also use the GrPrimitiveProcessor to make batching decisions.
 *
 * There are two feedback loops between the GrFragmentProcessors, the GrXferProcessor, and the
 * GrPrimitiveProcessor.  These loops run on the CPU and compute any invariant components which
 * might be useful for correctness / optimization decisions.  The GrPrimitiveProcessor seeds these
 * loops, one with initial color and one with initial coverage, in its
 * onComputeInvariantColor / Coverage calls.  These seed values are processed by the subsequent
 * stages of the rendering pipeline and the output is then fed back into the GrPrimitiveProcessor in
 * the initBatchTracker call, where the GrPrimitiveProcessor can then initialize the GrBatchTracker
 * struct with the appropriate values.
 *
 * We are evolving this system to move towards generating geometric meshes and their associated
 * vertex data after we have batched and reordered draws.  This system, known as 'deferred geometry'
 * will allow the GrPrimitiveProcessor much greater control over how data is transmitted to shaders.
 *
 * In a deferred geometry world, the GrPrimitiveProcessor can always 'batch'  To do this, each
 * primitive type is associated with one GrPrimitiveProcessor, who has complete control of how
 * it draws.  Each primitive draw will bundle all required data to perform the draw, and these
 * bundles of data will be owned by an instance of the associated GrPrimitiveProcessor.  Bundles
 * can be updated alongside the GrBatchTracker struct itself, ultimately allowing the
 * GrPrimitiveProcessor complete control of how it gets data into the fragment shader as long as
 * it emits the appropriate color, or none at all, as directed.
 */

/*
 * A struct for tracking batching decisions.  While this lives on GrOptState, it is managed
 * entirely by the derived classes of the GP.
 * // TODO this was an early attempt at handling out of order batching.  It should be
 * used carefully as it is being replaced by GrBatch
 */
class GrBatchTracker {
public:
    template <typename T> const T& cast() const {
        SkASSERT(sizeof(T) <= kMaxSize);
        return *reinterpret_cast<const T*>(fData.get());
    }

    template <typename T> T* cast() {
        SkASSERT(sizeof(T) <= kMaxSize);
        return reinterpret_cast<T*>(fData.get());
    }

    static const size_t kMaxSize = 32;

private:
    SkAlignedSStorage<kMaxSize> fData;
};

class GrGLSLCaps;
class GrGLPrimitiveProcessor;

struct GrInitInvariantOutput;

/*
 * This class allows the GrPipeline to communicate information about the pipeline to a
 * GrPrimitiveProcessor that will be used in conjunction with the GrPipeline.
 */
class GrPipelineInfo {
public:
    /** Does the pipeline require the GrPrimitiveProcessor's color? */
    bool readsColor() const { return SkToBool(kReadsColor_GrPipelineInfoFlag & fFlags); }

    /** Does the pipeline require the GrPrimitiveProcessor's coverage? */
    bool readsCoverage() const { return SkToBool(kReadsCoverage_GrPipelineInfoFlag & fFlags); }

    /** Does the pipeline require access to (implicit or explicit) local coordinates? */
    bool readsLocalCoords() const {
        return SkToBool(kReadsLocalCoords_GrPipelineInfoFlag & fFlags);
    }

    /** Does the pipeline allow the GrPrimitiveProcessor to combine color and coverage into one
        color output ? */
    bool canTweakAlphaForCoverage() const {
        return SkToBool(kCanTweakAlphaForCoverage_GrPipelineInfoFlag & fFlags);
    }

    /** Does the pipeline require the GrPrimitiveProcessor to specify a specific color (and if
        so get the color)? */
    bool getOverrideColorIfSet(GrColor* overrideColor) const {
        if (SkToBool(kUseOverrideColor_GrPipelineInfoFlag & fFlags)) {
            SkASSERT(SkToBool(kReadsColor_GrPipelineInfoFlag & fFlags));
            if (overrideColor) {
                *overrideColor = fOverrideColor;
            }
            return true;
        }
        return false;
    }

private:
    enum {
        // If this is not set the primitive processor need not produce a color output
        kReadsColor_GrPipelineInfoFlag                  = 0x1,

        // If this is not set the primitive processor need not produce a coverage output
        kReadsCoverage_GrPipelineInfoFlag               = 0x2,

        // If this is not set the primitive processor need not produce local coordinates
        kReadsLocalCoords_GrPipelineInfoFlag            = 0x4,

        // If this flag is set then the primitive processor may produce color*coverage as
        // its color output (and not output a separate coverage).
        kCanTweakAlphaForCoverage_GrPipelineInfoFlag    = 0x8,

        // If this flag is set the GrPrimitiveProcessor must produce fOverrideColor as its
        // output color. If not set fOverrideColor is to be ignored.
        kUseOverrideColor_GrPipelineInfoFlag            = 0x10,
    };

    uint32_t    fFlags;
    GrColor     fOverrideColor;

    friend class GrPipeline; // To initialize this
};

/*
 * This enum is shared by GrPrimitiveProcessors and GrGLPrimitiveProcessors to coordinate shaders
 * with vertex attributes / uniforms.
 */
enum GrGPInput {
    kAllOnes_GrGPInput,
    kAttribute_GrGPInput,
    kUniform_GrGPInput,
    kIgnored_GrGPInput,
};

/*
 * GrPrimitiveProcessor defines an interface which all subclasses must implement.  All
 * GrPrimitiveProcessors must proivide seed color and coverage for the Ganesh color / coverage
 * pipelines, and they must provide some notion of equality
 */
class GrPrimitiveProcessor : public GrProcessor {
public:
    virtual void initBatchTracker(GrBatchTracker*, const GrPipelineInfo&) const = 0;

    virtual bool canMakeEqual(const GrBatchTracker& mine,
                              const GrPrimitiveProcessor& that,
                              const GrBatchTracker& theirs) const = 0;

    virtual void getInvariantOutputColor(GrInitInvariantOutput* out) const = 0;
    virtual void getInvariantOutputCoverage(GrInitInvariantOutput* out) const = 0;

    // Only the GrGeometryProcessor subclass actually has a geo shader or vertex attributes, but
    // we put these calls on the base class to prevent having to cast
    virtual bool willUseGeoShader() const = 0;

    /*
     * This is a safeguard to prevent GrPrimitiveProcessor's from going beyond platform specific
     * attribute limits. This number can almost certainly be raised if required.
     */
    static const int kMaxVertexAttribs = 6;

    struct Attribute {
        Attribute()
            : fName(NULL)
            , fType(kFloat_GrVertexAttribType)
            , fOffset(0) {}
        Attribute(const char* name, GrVertexAttribType type,
                  GrSLPrecision precision = kDefault_GrSLPrecision)
            : fName(name)
            , fType(type)
            , fOffset(SkAlign4(GrVertexAttribTypeSize(type)))
            , fPrecision(precision) {}
        const char* fName;
        GrVertexAttribType fType;
        size_t fOffset;
        GrSLPrecision fPrecision;
    };

    int numAttribs() const { return fNumAttribs; }
    const Attribute& getAttrib(int index) const {
        SkASSERT(index < fNumAttribs);
        return fAttribs[index];
    }

    // Returns the vertex stride of the GP.  A common use case is to request geometry from a
    // drawtarget based off of the stride, and to populate this memory using an implicit array of
    // structs.  In this case, it is best to assert the vertexstride == sizeof(VertexStruct).
    size_t getVertexStride() const { return fVertexStride; }

    /**
     * Gets a transformKey from an array of coord transforms
     */
    uint32_t getTransformKey(const SkTArray<const GrCoordTransform*, true>&) const;

    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this geometry
     * processor's GL backend implementation.
     */
    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const = 0;


    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLSLCaps& caps) const = 0;

    bool isPathRendering() const { return fIsPathRendering; }

    /**
     * No Local Coord Transformation is needed in the shader, instead transformed local coords will
     * be provided via vertex attribute.
     */
    virtual bool hasTransformedLocalCoords() const = 0;

protected:
    GrPrimitiveProcessor(bool isPathRendering)
        : fNumAttribs(0)
        , fVertexStride(0)
        , fIsPathRendering(isPathRendering) {}

    Attribute fAttribs[kMaxVertexAttribs];
    int fNumAttribs;
    size_t fVertexStride;

private:
    virtual bool hasExplicitLocalCoords() const = 0;

    bool fIsPathRendering;

    typedef GrProcessor INHERITED;
};

#endif
