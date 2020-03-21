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
    using CoordTransformRange   = GrFragmentProcessor::PipelineCoordTransformRange;

    struct TransformVar {
        // The transform as a variable. This may be a kFloat3x3 matrix or a kFloat4 representing
        // {scaleX, transX, scaleY, transY}. For explicitly sampled FPs this is visible in the
        // FS. This is not available for NV_path_rendering with non-explicitly sampled FPs.
        GrShaderVar fTransform;
        // The transformed coordinate output by the vertex shader and consumed by the fragment
        // shader. Only valid for non-explicitly sampled FPs.
        GrShaderVar fVaryingPoint;
    };


    virtual ~GrGLSLPrimitiveProcessor() {}

    /**
     * This class provides access to the GrCoordTransforms across all GrFragmentProcessors in a
     * GrPipeline. It is also used by the primitive processor to specify the fragment shader
     * variable that will hold the transformed coords for each GrCoordTransform. It is required that
     * the primitive processor iterate over each coord transform and insert a shader var result for
     * each. The GrGLSLFragmentProcessors will reference these variables in their fragment code.
     */
    class FPCoordTransformHandler : public SkNoncopyable {
    public:
        FPCoordTransformHandler(const GrPipeline&, SkTArray<TransformVar>*);
        ~FPCoordTransformHandler() { SkASSERT(!fIter); }

        operator bool() const { return (bool)fIter; }

        // Gets the current coord transform and its owning GrFragmentProcessor.
        std::pair<const GrCoordTransform&, const GrFragmentProcessor&> get() const;

        FPCoordTransformHandler& operator++();

        // 'args' are constructor params to GrShaderVar.
        void specifyCoordsForCurrCoordTransform(GrShaderVar transformVar, GrShaderVar varyingVar) {
            SkASSERT(!fAddedCoord);
            fTransformedCoordVars->push_back({transformVar, varyingVar});
            SkDEBUGCODE(fAddedCoord = true;)
        }

        void omitCoordsForCurrCoordTransform() {
            SkASSERT(!fAddedCoord);
            fTransformedCoordVars->push_back();
            SkDEBUGCODE(fAddedCoord = true;)
        }

    private:
        GrFragmentProcessor::CoordTransformIter fIter;
        SkDEBUGCODE(bool                        fAddedCoord = false;)
        SkTArray<TransformVar>*                 fTransformedCoordVars;
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
                 const char* rtAdjustName,
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
            , fRTAdjustName(rtAdjustName)
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
        const char* fRTAdjustName;
        const SamplerHandle* fTexSamplers;
        FPCoordTransformHandler* fFPCoordTransformHandler;
    };

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(EmitArgs&) = 0;

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
    virtual void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                         const CoordTransformRange&) = 0;

    static SkMatrix GetTransformMatrix(const GrCoordTransform&, const SkMatrix& preMatrix);

protected:
    void setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                           GrGLSLUniformHandler* uniformHandler,
                           const char* outputName,
                           UniformHandle* colorUniform);
};

#endif
