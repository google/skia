/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLPrimitiveProcessor_DEFINED
#define GrGLSLPrimitiveProcessor_DEFINED

#include "GrFragmentProcessor.h"
#include "GrPrimitiveProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrPrimitiveProcessor;
class GrGLSLPPFragmentBuilder;
class GrGLSLGeometryBuilder;
class GrGLSLGPBuilder;
class GrGLSLVaryingHandler;
class GrGLSLVertexBuilder;
class GrShaderCaps;

class GrGLSLPrimitiveProcessor {
public:
    using FPCoordTransformIter = GrFragmentProcessor::CoordTransformIter;

    virtual ~GrGLSLPrimitiveProcessor() {}

    using UniformHandle      = GrGLSLProgramDataManager::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;
    using ImageStorageHandle = GrGLSLUniformHandler::ImageStorageHandle;

    /**
     * This class provides access to the GrCoordTransforms across all GrFragmentProcessors in a
     * GrPipeline. It is also used by the primitive processor to specify the fragment shader
     * variable that will hold the transformed coords for each GrCoordTransform. It is required that
     * the primitive processor iterate over each coord transform and insert a shader var result for
     * each. The GrGLSLFragmentProcessors will reference these variables in their fragment code.
     */
    class FPCoordTransformHandler : public SkNoncopyable {
    public:
        FPCoordTransformHandler(const GrPipeline& pipeline,
                                SkTArray<GrShaderVar>* transformedCoordVars)
                : fIter(pipeline)
                , fTransformedCoordVars(transformedCoordVars) {}

        ~FPCoordTransformHandler() { SkASSERT(!this->nextCoordTransform());}

        const GrCoordTransform* nextCoordTransform();

        // 'args' are constructor params to GrShaderVar.
        template<typename... Args>
        void specifyCoordsForCurrCoordTransform(Args&&... args) {
            SkASSERT(!fAddedCoord);
            fTransformedCoordVars->emplace_back(std::forward<Args>(args)...);
            SkDEBUGCODE(fAddedCoord = true;)
        }

    private:
        GrFragmentProcessor::CoordTransformIter fIter;
        SkDEBUGCODE(bool                        fAddedCoord = false;)
        SkDEBUGCODE(const GrCoordTransform*     fCurr = nullptr;)
        SkTArray<GrShaderVar>*                  fTransformedCoordVars;
    };

    struct EmitArgs {
        EmitArgs(GrGLSLVertexBuilder* vertBuilder,
                 GrGLSLGeometryBuilder* geomBuilder,
                 GrGLSLPPFragmentBuilder* fragBuilder,
                 GrGLSLVaryingHandler* varyingHandler,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrPrimitiveProcessor& gp,
                 const char* outputColor,
                 const char* outputCoverage,
                 const char* distanceVectorName,
                 const char* rtAdjustName,
                 const SamplerHandle* texSamplers,
                 const SamplerHandle* bufferSamplers,
                 const ImageStorageHandle* imageStorages,
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
            , fDistanceVectorName(distanceVectorName)
            , fRTAdjustName(rtAdjustName)
            , fTexSamplers(texSamplers)
            , fBufferSamplers(bufferSamplers)
            , fImageStorages(imageStorages)
            , fFPCoordTransformHandler(transformHandler) {}
        GrGLSLVertexBuilder* fVertBuilder;
        GrGLSLGeometryBuilder* fGeomBuilder;
        GrGLSLPPFragmentBuilder* fFragBuilder;
        GrGLSLVaryingHandler* fVaryingHandler;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrPrimitiveProcessor& fGP;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const char* fDistanceVectorName;
        const char* fRTAdjustName;
        const SamplerHandle* fTexSamplers;
        const SamplerHandle* fBufferSamplers;
        const ImageStorageHandle* fImageStorages;
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
     * The subclass may use the transform iterator to perform any setup required for the particular
     * set of fp transform matrices, such as uploading via uniforms. The iterator will iterate over
     * the transforms in the same order as the TransformHandler passed to emitCode.
     */
    virtual void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                         FPCoordTransformIter&&) = 0;

    static SkMatrix GetTransformMatrix(const SkMatrix& localMatrix, const GrCoordTransform&);

protected:
    void setupUniformColor(GrGLSLPPFragmentBuilder* fragBuilder,
                           GrGLSLUniformHandler* uniformHandler,
                           const char* outputName,
                           UniformHandle* colorUniform);
};

#endif
