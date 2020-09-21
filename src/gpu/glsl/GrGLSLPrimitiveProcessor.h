/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLPrimitiveProcessor_DEFINED
#define GrGLSLPrimitiveProcessor_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrPrimitiveProcessor;
class GrGLSLFPFragmentBuilder;
class GrGLSLGeometryBuilder;
class GrGLSLGPBuilder;
class GrGLSLVaryingHandler;
class GrGLSLVertexBuilder;
class GrShaderCaps;

class GrGLSLPrimitiveProcessor {
public:
    using UniformHandle         = GrGLSLProgramDataManager::UniformHandle;
    using SamplerHandle         = GrGLSLUniformHandler::SamplerHandle;

    virtual ~GrGLSLPrimitiveProcessor() {}

    /**
     * This class provides access to each GrFragmentProcessor in a GrPipeline that requires varying
     * local coords to be produced by the primitive processor. It is also used by the primitive
     * processor to specify the fragment shader variable that will hold the transformed coords for
     * each of those GrFragmentProcessors. It is required that the primitive processor iterate over
     * each fragment processor and insert a shader var result for each. The GrGLSLFragmentProcessors
     * will reference these variables in their fragment code.
     */
    class FPCoordTransformHandler : public SkNoncopyable {
    public:
        FPCoordTransformHandler(const GrPipeline&, SkTArray<GrShaderVar>*);
        ~FPCoordTransformHandler() { SkASSERT(!fIter); }

        operator bool() const { return (bool)fIter; }

        // Gets the current GrFragmentProcessor
        const GrFragmentProcessor& get() const;

        FPCoordTransformHandler& operator++();

        void specifyCoordsForCurrCoordTransform(GrShaderVar varyingVar) {
            SkASSERT(!fAddedCoord);
            fTransformedCoordVars->push_back(varyingVar);
            SkDEBUGCODE(fAddedCoord = true;)
        }

    private:
        GrFragmentProcessor::CIter fIter;
        SkDEBUGCODE(bool           fAddedCoord = false;)
        SkTArray<GrShaderVar>*     fTransformedCoordVars;
    };

    struct EmitArgs {
        EmitArgs(GrGLSLVertexBuilder* vertBuilder,
                 GrGLSLGeometryBuilder* geomBuilder,
                 GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLVaryingHandler* varyingHandler,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrPrimitiveProcessor& gp,
                 const char* outputColor,
                 const char* outputCoverage,
                 const SamplerHandle* texSamplers,
                 FPCoordTransformHandler* transformHandler)
            : fVertBuilder(vertBuilder)
            , fGeomBuilder(geomBuilder)
            , fFragBuilder(fragBuilder)
            , fVaryingHandler(varyingHandler)
            , fUniformHandler(uniformHandler)
            , fShaderCaps(caps)
            , fGP(gp)
            , fOutputColor(outputColor)
            , fOutputCoverage(outputCoverage)
            , fTexSamplers(texSamplers)
            , fFPCoordTransformHandler(transformHandler) {}
        GrGLSLVertexBuilder* fVertBuilder;
        GrGLSLGeometryBuilder* fGeomBuilder;
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLVaryingHandler* fVaryingHandler;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrPrimitiveProcessor& fGP;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const SamplerHandle* fTexSamplers;
        FPCoordTransformHandler* fFPCoordTransformHandler;
    };

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(EmitArgs&) = 0;

    /**
     * Called after all effect emitCode() functions, to give the processor a chance to write out
     * additional transformation code now that all uniforms have been emitted.
     */
    virtual void emitTransformCode(GrGLSLVertexBuilder* vb,
                                   GrGLSLUniformHandler* uniformHandler) {}
    /**
     * A GrGLSLPrimitiveProcessor instance can be reused with any GrGLSLPrimitiveProcessor that
     * produces the same stage key; this function reads data from a GrGLSLPrimitiveProcessor and
     * uploads any uniform variables required  by the shaders created in emitCode(). The
     * GrPrimitiveProcessor parameter is guaranteed to be of the same type and to have an
     * identical processor key as the GrPrimitiveProcessor that created this
     * GrGLSLPrimitiveProcessor.
     * The subclass should use the transform range to perform any setup required for the coord
     * transforms of the FPs that are part of the same program, such as updating matrix uniforms.
     * The range will iterate over the transforms in the same order as the TransformHandler passed
     * to emitCode.
     */
    virtual void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&) = 0;

protected:
    void setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                           GrGLSLUniformHandler* uniformHandler,
                           const char* outputName,
                           UniformHandle* colorUniform);
};

#endif
