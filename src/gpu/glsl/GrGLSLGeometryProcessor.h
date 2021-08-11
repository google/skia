/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLGeometryProcessor_DEFINED
#define GrGLSLGeometryProcessor_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

#include <unordered_map>

class GrGeometryProcessor;
class GrGLSLFPFragmentBuilder;
class GrGLSLGeometryBuilder;
class GrGLSLVaryingHandler;
class GrGLSLVertexBuilder;
class GrShaderCaps;

class GrGeometryProcessor::ProgramImpl {
public:
    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
    using SamplerHandle = GrGLSLUniformHandler::SamplerHandle;
    /**
     * Struct of optional varying that replaces the input coords and bool indicating whether the FP
     * should take a coord param as an argument. The latter may be false if the coords are simply
     * unused or if the GP has lifted their computation to a varying emitted by the VS.
     */
    struct FPCoords {GrShaderVar coordsVarying; bool hasCoordsParam;};
    using FPCoordsMap = std::unordered_map<const GrFragmentProcessor*, FPCoords>;

    virtual ~ProgramImpl() = default;

    struct EmitArgs {
        EmitArgs(GrGLSLVertexBuilder* vertBuilder,
                 GrGLSLGeometryBuilder* geomBuilder,
                 GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLVaryingHandler* varyingHandler,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrGeometryProcessor& geomProc,
                 const char* outputColor,
                 const char* outputCoverage,
                 const SamplerHandle* texSamplers)
                : fVertBuilder(vertBuilder)
                , fGeomBuilder(geomBuilder)
                , fFragBuilder(fragBuilder)
                , fVaryingHandler(varyingHandler)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fGeomProc(geomProc)
                , fOutputColor(outputColor)
                , fOutputCoverage(outputCoverage)
                , fTexSamplers(texSamplers) {}
        GrGLSLVertexBuilder* fVertBuilder;
        GrGLSLGeometryBuilder* fGeomBuilder;
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLVaryingHandler* fVaryingHandler;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrGeometryProcessor& fGeomProc;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const SamplerHandle* fTexSamplers;
    };

    /**
     * Emits the code from this geometry processor into the shaders. For any FP in the pipeline that
     * has its input coords implemented by the GP as a varying, the varying will be accessible in
     * the returned map and should be used when the FP code is emitted.
     **/
    FPCoordsMap emitCode(EmitArgs&, const GrPipeline& pipeline);

    /**
     * Called after all effect emitCode() functions, to give the processor a chance to write out
     * additional transformation code now that all uniforms have been emitted.
     * It generates the final code for assigning transformed coordinates to the varyings recorded
     * in the call to collectTransforms(). This must happen after FP code emission so that it has
     * access to any uniforms the FPs registered for uniform sample matrix invocations.
     */
    void emitTransformCode(GrGLSLVertexBuilder* vb,
                           GrGLSLUniformHandler* uniformHandler);

    /**
     * A ProgramImpl instance can be reused with any GrGeometryProcessor that produces the same key.
     * This function reads data from a GrGeometryProcessor and updates any uniform variables
     * required by the shaders created in emitCode(). The GrGeometryProcessor parameter is
     * guaranteed to be of the same type and to have an identical processor key as the
     * GrGeometryProcessor that created this ProgramImpl.
     */
    virtual void setData(const GrGLSLProgramDataManager&,
                         const GrShaderCaps&,
                         const GrGeometryProcessor&) = 0;

    // We use these methods as a temporary back door to inject OpenGL tessellation code. Once
    // tessellation is supported by SkSL we can remove these.
    virtual SkString getTessControlShaderGLSL(const GrGeometryProcessor&,
                                              const char* versionAndExtensionDecls,
                                              const GrGLSLUniformHandler&,
                                              const GrShaderCaps&) const {
        SK_ABORT("Not implemented.");
    }
    virtual SkString getTessEvaluationShaderGLSL(const GrGeometryProcessor&,
                                                 const char* versionAndExtensionDecls,
                                                 const GrGLSLUniformHandler&,
                                                 const GrShaderCaps&) const {
        SK_ABORT("Not implemented.");
    }

    // GPs that use writeOutputPosition and/or writeLocalCoord must incorporate the matrix type
    // into their key, and should use this function or one of the other related helpers.
    static uint32_t ComputeMatrixKey(const GrShaderCaps& caps, const SkMatrix& mat) {
        if (!caps.reducedShaderMode()) {
            if (mat.isIdentity()) {
                return 0b00;
            }
            if (mat.isScaleTranslate()) {
                return 0b01;
            }
        }
        if (!mat.hasPerspective()) {
            return 0b10;
        }
        return 0b11;
    }

    static uint32_t ComputeMatrixKeys(const GrShaderCaps& shaderCaps,
                                      const SkMatrix& viewMatrix,
                                      const SkMatrix& localMatrix) {
        return (ComputeMatrixKey(shaderCaps, viewMatrix ) << kMatrixKeyBits) |
                ComputeMatrixKey(shaderCaps, localMatrix);
    }

    static uint32_t AddMatrixKeys(const GrShaderCaps& shaderCaps,
                                  uint32_t flags,
                                  const SkMatrix& viewMatrix,
                                  const SkMatrix& localMatrix) {
        // Shifting to make room for the matrix keys shouldn't lose bits
        SkASSERT(((flags << (2 * kMatrixKeyBits)) >> (2 * kMatrixKeyBits)) == flags);
        return (flags << (2 * kMatrixKeyBits)) |
               ComputeMatrixKeys(shaderCaps, viewMatrix, localMatrix);
    }
    static constexpr int kMatrixKeyBits = 2;

protected:
    void setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                           GrGLSLUniformHandler* uniformHandler,
                           const char* outputName,
                           UniformHandle* colorUniform);

    // A helper for setting the matrix on a uniform handle initialized through
    // writeOutputPosition or writeLocalCoord. Automatically handles elided uniforms,
    // scale+translate matrices, and state tracking (if provided state pointer is non-null).
    static void SetTransform(const GrGLSLProgramDataManager&,
                             const GrShaderCaps&,
                             const UniformHandle& uniform,
                             const SkMatrix& matrix,
                             SkMatrix* state = nullptr);

    struct GrGPArgs {
        // Used to specify the output variable used by the GP to store its device position. It can
        // either be a float2 or a float3 (in order to handle perspective). The subclass sets this
        // in its onEmitCode().
        GrShaderVar fPositionVar;
        // Used to specify the variable storing the draw's local coordinates. It can be either a
        // float2, float3, or void. It can only be void when no FP needs local coordinates. This
        // variable can be an attribute or local variable, but should not itself be a varying.
        // ProgramImpl automatically determines if this must be passed to a FS.
        GrShaderVar fLocalCoordVar;
    };

    // Helpers for adding code to write the transformed vertex position. The first simple version
    // just writes a variable named by 'posName' into the position output variable with the
    // assumption that the position is 2D. The second version transforms the input position by a
    // view matrix and the output variable is 2D or 3D depending on whether the view matrix is
    // perspective. Both versions declare the output position variable and will set
    // GrGPArgs::fPositionVar.
    static void WriteOutputPosition(GrGLSLVertexBuilder*, GrGPArgs*, const char* posName);
    static void WriteOutputPosition(GrGLSLVertexBuilder*,
                                    GrGLSLUniformHandler*,
                                    const GrShaderCaps&,
                                    GrGPArgs*,
                                    const char* posName,
                                    const SkMatrix& viewMatrix,
                                    UniformHandle* viewMatrixUniform);

    // Helper to transform an existing variable by a given local matrix (e.g. the inverse view
    // matrix). It will declare the transformed local coord variable and will set
    // GrGPArgs::fLocalCoordVar.
    static void WriteLocalCoord(GrGLSLVertexBuilder*,
                                GrGLSLUniformHandler*,
                                const GrShaderCaps&,
                                GrGPArgs*,
                                GrShaderVar localVar,
                                const SkMatrix& localMatrix,
                                UniformHandle* localMatrixUniform);

private:
    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    // Iterates over the FPs beginning with the passed iter to register additional varyings and
    // uniforms to support VS-promoted local coord evaluation for the FPs.
    //
    // This must happen before FP code emission so that the FPs can find the appropriate varying
    // handles they use in place of explicit coord sampling; it is automatically called after
    // onEmitCode() returns using the value stored in GpArgs::fLocalCoordVar and
    // GpArgs::fPositionVar.
    FPCoordsMap collectTransforms(GrGLSLVertexBuilder* vb,
                                  GrGLSLVaryingHandler* varyingHandler,
                                  GrGLSLUniformHandler* uniformHandler,
                                  const GrShaderVar& localCoordsVar,
                                  const GrShaderVar& positionVar,
                                  const GrPipeline& pipeline);
    struct TransformInfo {
        // The varying that conveys the coordinates to one or more FPs in the FS.
        GrGLSLVarying varying;
        // The coordinate to be transformed. varying is computed from this.
        GrShaderVar   inputCoords;
        // Used to sort so that ancestor FP varyings are initialized before descendant FP varyings.
        int           traversalOrder;
    };
    // Populated by collectTransforms() for use in emitTransformCode(). When we lift the computation
    // of a FP's input coord to a varying we propagate that varying up the FP tree to the highest
    // node that shares the same coordinates. This allows multiple FPs in a subtree to share a
    // varying.
    std::unordered_map<const GrFragmentProcessor*, TransformInfo> fTransformVaryingsMap;
};

#endif
