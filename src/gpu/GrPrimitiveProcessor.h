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

class GrGLSLCaps;
class GrGLSLPrimitiveProcessor;

struct GrInitInvariantOutput;

// Describes the state of pixel local storage with respect to the current draw.
enum GrPixelLocalStorageState {
    // The draw is actively updating PLS.
    kDraw_GrPixelLocalStorageState,
    // The draw is a "finish" operation which is reading from PLS and writing color.
    kFinish_GrPixelLocalStorageState,
    // The draw does not use PLS.
    kDisabled_GrPixelLocalStorageState
};

/*
 * This class allows the GrPipeline to communicate information about the pipeline to a
 * GrBatch which should be forwarded to the GrPrimitiveProcessor(s) created by the batch.
 * These are not properly part of the pipeline because they assume the specific inputs
 * that the batch provided when it created the pipeline. Identical pipelines may be
 * created by different batches with different input assumptions and therefore different
 * computed optimizations. It is the batch-specific optimizations that allow the pipelines
 * to be equal.
 */
class GrXPOverridesForBatch {
public:
    /** Does the pipeline require the GrPrimitiveProcessor's color? */
    bool readsColor() const { return SkToBool(kReadsColor_Flag & fFlags); }

    /** Does the pipeline require the GrPrimitiveProcessor's coverage? */
    bool readsCoverage() const { return
        SkToBool(kReadsCoverage_Flag & fFlags); }

    /** Does the pipeline require access to (implicit or explicit) local coordinates? */
    bool readsLocalCoords() const {
        return SkToBool(kReadsLocalCoords_Flag & fFlags);
    }

    /** Does the pipeline allow the GrPrimitiveProcessor to combine color and coverage into one
        color output ? */
    bool canTweakAlphaForCoverage() const {
        return SkToBool(kCanTweakAlphaForCoverage_Flag & fFlags);
    }

    /** Does the pipeline require the GrPrimitiveProcessor to specify a specific color (and if
        so get the color)? */
    bool getOverrideColorIfSet(GrColor* overrideColor) const {
        if (SkToBool(kUseOverrideColor_Flag & fFlags)) {
            SkASSERT(SkToBool(kReadsColor_Flag & fFlags));
            if (overrideColor) {
                *overrideColor = fOverrideColor;
            }
            return true;
        }
        return false;
    }

    /**
     * Returns true if the pipeline's color output will be affected by the existing render target
     * destination pixel values (meaning we need to be careful with overlapping draws). Note that we
     * can conflate coverage and color, so the destination color may still bleed into pixels that
     * have partial coverage, even if this function returns false.
     *
     * The above comment seems incorrect for the use case. This funciton is used to turn two
     * overlapping draws into a single draw (really to stencil multiple paths and do a single
     * cover). It seems that what really matters is whether the dst is read for color OR for
     * coverage.
     */
    bool willColorBlendWithDst() const { return SkToBool(kWillColorBlendWithDst_Flag & fFlags); }

private:
    enum {
        // If this is not set the primitive processor need not produce a color output
        kReadsColor_Flag                = 0x1,

        // If this is not set the primitive processor need not produce a coverage output
        kReadsCoverage_Flag             = 0x2,

        // If this is not set the primitive processor need not produce local coordinates
        kReadsLocalCoords_Flag          = 0x4,

        // If this flag is set then the primitive processor may produce color*coverage as
        // its color output (and not output a separate coverage).
        kCanTweakAlphaForCoverage_Flag  = 0x8,

        // If this flag is set the GrPrimitiveProcessor must produce fOverrideColor as its
        // output color. If not set fOverrideColor is to be ignored.
        kUseOverrideColor_Flag          = 0x10,

        kWillColorBlendWithDst_Flag     = 0x20,
    };

    uint32_t    fFlags;
    GrColor     fOverrideColor;

    friend class GrPipeline; // To initialize this
};

/*
 * GrPrimitiveProcessor defines an interface which all subclasses must implement.  All
 * GrPrimitiveProcessors must proivide seed color and coverage for the Ganesh color / coverage
 * pipelines, and they must provide some notion of equality
 */
class GrPrimitiveProcessor : public GrProcessor {
public:
    // Only the GrGeometryProcessor subclass actually has a geo shader or vertex attributes, but
    // we put these calls on the base class to prevent having to cast
    virtual bool willUseGeoShader() const = 0;

    struct Attribute {
        Attribute()
            : fName(nullptr)
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

    int numAttribs() const { return fAttribs.count(); }
    const Attribute& getAttrib(int index) const { return fAttribs[index]; }

    // Returns the vertex stride of the GP.  A common use case is to request geometry from a
    // drawtarget based off of the stride, and to populate this memory using an implicit array of
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
    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const = 0;


    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps& caps) const = 0;

    virtual bool isPathRendering() const { return false; }

    /**
     * No Local Coord Transformation is needed in the shader, instead transformed local coords will
     * be provided via vertex attribute.
     */
    virtual bool hasTransformedLocalCoords() const = 0;

    virtual GrPixelLocalStorageState getPixelLocalStorageState() const {
        return kDisabled_GrPixelLocalStorageState;
    }

    /**
     * If non-null, overrides the dest color returned by GrGLSLFragmentShaderBuilder::dstColor().
     */
    virtual const char* getDestColorOverride() const { return nullptr; }

    virtual float getSampleShading() const {
        return 0.0;
    }

protected:
    GrPrimitiveProcessor() : fVertexStride(0) {}

    enum { kPreallocAttribCnt = 8 };
    SkSTArray<kPreallocAttribCnt, Attribute> fAttribs;
    size_t fVertexStride;

private:
    void notifyRefCntIsZero() const final {};
    virtual bool hasExplicitLocalCoords() const = 0;

    typedef GrProcessor INHERITED;
};

#endif
