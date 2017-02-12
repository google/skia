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
 * getPipelineAnalysisInput implementation. These seed values are processed by the subsequent
 * stages of the rendering pipeline and the output is then fed back into the GrDrawOp in
 * the applyPipelineOptimizations call, where the op can use the information to inform decisions
 * about GrPrimitiveProcessor creation.
 */

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
 * This class allows the GrPipeline to communicate information about the pipeline to a GrOp which
 * inform its decisions for GrPrimitiveProcessor setup. These are not properly part of the pipeline
 * because they reflect the specific inputs that the op provided to perform the analysis (e.g. that
 * the GrGeometryProcessor would output an opaque color).
 *
 * The pipeline analysis that produced this may have decided to elide some GrProcessors. However,
 * those elisions may depend upon changing the color output by the GrGeometryProcessor used by the
 * GrDrawOp. The op must check getOverrideColorIfSet() for this.
 */
class GrPipelineOptimizations {
public:
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
     * The above comment seems incorrect for the use case. This function is used to turn two
     * overlapping draws into a single draw (really to stencil multiple paths and do a single
     * cover). It seems that what really matters is whether the dst is read for color OR for
     * coverage.
     */
    bool willColorBlendWithDst() const { return SkToBool(kWillColorBlendWithDst_Flag & fFlags); }

private:
    enum {
        // If this is not set the primitive processor need not produce local coordinates
        kReadsLocalCoords_Flag = 0x1,

        // If this flag is set then the primitive processor may produce color*coverage as
        // its color output (and not output a separate coverage).
        kCanTweakAlphaForCoverage_Flag = 0x2,

        // If this flag is set the GrPrimitiveProcessor must produce fOverrideColor as its
        // output color. If not set fOverrideColor is to be ignored.
        kUseOverrideColor_Flag = 0x4,

        kWillColorBlendWithDst_Flag = 0x8,
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

    /* Sub-class should override and return true if this primitive processor implements the distance
     * vector field, a field of vectors to the nearest point in the edge of the shape.  */
    virtual bool implementsDistanceVector() const { return false; }

protected:
    GrPrimitiveProcessor() : fVertexStride(0) {}

    enum { kPreallocAttribCnt = 8 };
    SkSTArray<kPreallocAttribCnt, Attribute> fAttribs;
    size_t fVertexStride;

private:
    void notifyRefCntIsZero() const final {}
    virtual bool hasExplicitLocalCoords() const = 0;

    typedef GrProcessor INHERITED;
};

#endif
