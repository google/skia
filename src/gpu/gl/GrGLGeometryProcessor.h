/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLProcessor.h"

class GrBatchTracker;
class GrGLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor {
public:
    GrGLGeometryProcessor() : fViewMatrixName(NULL) { fViewMatrix = SkMatrix::InvalidMatrix(); }
    virtual ~GrGLGeometryProcessor() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;

    struct EmitArgs {
        EmitArgs(GrGLGPBuilder* pb,
                 const GrPrimitiveProcessor& gp,
                 const GrBatchTracker& bt,
                 const char* outputColor,
                 const char* outputCoverage,
                 const TextureSamplerArray& samplers)
            : fPB(pb)
            , fGP(gp)
            , fBT(bt)
            , fOutputColor(outputColor)
            , fOutputCoverage(outputCoverage)
            , fSamplers(samplers) {}
        GrGLGPBuilder* fPB;
        const GrPrimitiveProcessor& fGP;
        const GrBatchTracker& fBT;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const TextureSamplerArray& fSamplers;
    };

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(const EmitArgs&) = 0;

    /** A GrGLGeometryProcessor instance can be reused with any GrGLGeometryProcessor that produces
        the same stage key; this function reads data from a GrGLGeometryProcessor and uploads any
        uniform variables required  by the shaders created in emitCode(). The GrGeometryProcessor
        parameter is guaranteed to be of the same type that created this GrGLGeometryProcessor and
        to have an identical processor key as the one that created this GrGLGeometryProcessor.  */
    virtual void setData(const GrGLProgramDataManager&,
                         const GrPrimitiveProcessor&,
                         const GrBatchTracker&) = 0;

protected:
    /** a helper which can setup vertex, constant, or uniform color depending on inputType.
     *  This function will only do the minimum required to emit the correct shader code.  If
     *  inputType == attribute, then colorAttr must not be NULL.  Likewise, if inputType == Uniform
     *  then colorUniform must not be NULL.
     */
    void setupColorPassThrough(GrGLGPBuilder* pb,
                               GrGPInput inputType,
                               const char* inputName,
                               const GrGeometryProcessor::GrAttribute* colorAttr,
                               UniformHandle* colorUniform);

    const char* uViewM() const { return fViewMatrixName; }

    /** a helper function to setup the uniform handle for the uniform view matrix */
    void addUniformViewMatrix(GrGLGPBuilder*);


    /** a helper function to upload a uniform viewmatrix.
     * TODO we can remove this function when we have deferred geometry in place
     */
    void setUniformViewMatrix(const GrGLProgramDataManager&,
                              const SkMatrix& viewMatrix);

private:
    UniformHandle fViewMatrixUniform;
    SkMatrix fViewMatrix;
    const char* fViewMatrixName;

    typedef GrGLProcessor INHERITED;
};

#endif
