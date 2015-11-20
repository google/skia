/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLPrimitiveProcessor_DEFINED
#define GrGLSLPrimitiveProcessor_DEFINED

#include "GrPrimitiveProcessor.h"
#include "glsl/GrGLSLProcessorTypes.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLTextureSampler.h"

class GrBatchTracker;
class GrPrimitiveProcessor;
class GrGLSLCaps;
class GrGLSLFragmentBuilder;
class GrGLSLGPBuilder;
class GrGLSLVaryingHandler;
class GrGLSLVertexBuilder;

class GrGLSLPrimitiveProcessor {
public:
    virtual ~GrGLSLPrimitiveProcessor() {}

    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLSLTextureSampler::TextureSamplerArray TextureSamplerArray;

    typedef SkSTArray<2, const GrCoordTransform*, true> ProcCoords;
    typedef SkSTArray<8, ProcCoords> TransformsIn;
    typedef SkSTArray<8, GrGLSLTransformedCoordsArray> TransformsOut;

    struct EmitArgs {
        EmitArgs(GrGLSLGPBuilder* pb,
                 GrGLSLVertexBuilder* vertBuilder,
                 GrGLSLFragmentBuilder* fragBuilder,
                 GrGLSLVaryingHandler* varyingHandler,
                 const GrGLSLCaps* caps,
                 const GrPrimitiveProcessor& gp,
                 const char* outputColor,
                 const char* outputCoverage,
                 const TextureSamplerArray& samplers,
                 const TransformsIn& transformsIn,
                 TransformsOut* transformsOut)
            : fPB(pb)
            , fVertBuilder(vertBuilder)
            , fFragBuilder(fragBuilder)
            , fVaryingHandler(varyingHandler)
            , fGLSLCaps(caps)
            , fGP(gp)
            , fOutputColor(outputColor)
            , fOutputCoverage(outputCoverage)
            , fSamplers(samplers)
            , fTransformsIn(transformsIn)
            , fTransformsOut(transformsOut) {}
        GrGLSLGPBuilder* fPB;
        GrGLSLVertexBuilder* fVertBuilder;
        GrGLSLFragmentBuilder* fFragBuilder;
        GrGLSLVaryingHandler* fVaryingHandler;
        const GrGLSLCaps* fGLSLCaps;
        const GrPrimitiveProcessor& fGP;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const TextureSamplerArray& fSamplers;
        const TransformsIn& fTransformsIn;
        TransformsOut* fTransformsOut;
    };

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(EmitArgs&) = 0;


    /** A GrGLSLPrimitiveProcessor instance can be reused with any GrGLSLPrimitiveProcessor that
        produces the same stage key; this function reads data from a GrGLSLPrimitiveProcessor and
        uploads any uniform variables required  by the shaders created in emitCode(). The
        GrPrimitiveProcessor parameter is guaranteed to be of the same type that created this
        GrGLSLPrimitiveProcessor and to have an identical processor key as the one that created this
        GrGLSLPrimitiveProcessor.  */
    virtual void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&) = 0;

    static SkMatrix GetTransformMatrix(const SkMatrix& localMatrix, const GrCoordTransform&);

    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrGLSLProgramDataManager& pdman,
                                  int index,
                                  const SkTArray<const GrCoordTransform*, true>& transforms) = 0;

protected:
    void setupUniformColor(GrGLSLGPBuilder* pb,
                           GrGLSLFragmentBuilder* fragBuilder,
                           const char* outputName,
                           UniformHandle* colorUniform);

    struct Transform {
        Transform() : fType(kVoid_GrSLType) { fCurrentValue = SkMatrix::InvalidMatrix(); }
        UniformHandle  fHandle;
        SkMatrix       fCurrentValue;
        GrSLType       fType;
    };

    SkSTArray<8, SkSTArray<2, Transform, true> > fInstalledTransforms;
};

#endif
