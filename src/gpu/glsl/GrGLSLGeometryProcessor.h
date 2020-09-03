/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLGeometryProcessor_DEFINED
#define GrGLSLGeometryProcessor_DEFINED

#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"

class GrGLSLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLSLGeometryProcessor : public GrGLSLPrimitiveProcessor {
public:
    /* Any general emit code goes in the base class emitCode.  Subclasses override onEmitCode */
    void emitCode(EmitArgs&) final;

    // Generate the final code for assigning transformed coordinates to the varyings recorded in
    // the call to collectTransforms(). This must happen after FP code emission so that it has
    // access to any uniforms the FPs registered for uniform sample matrix invocations.
    void emitTransformCode(GrGLSLVertexBuilder* vb,
                           GrGLSLUniformHandler* uniformHandler) override;

protected:
    // A helper for setting the matrix on a uniform handle initialized through
    // writeOutputPosition or writeLocalCoord. Automatically handles elided uniforms,
    // scale+translate matrices, and state tracking (if provided state pointer is non-null).
    void setTransform(const GrGLSLProgramDataManager& pdman, const UniformHandle& uniform,
                      const SkMatrix& matrix, SkMatrix* state=nullptr) const;

    struct GrGPArgs {
        // Used to specify the output variable used by the GP to store its device position. It can
        // either be a float2 or a float3 (in order to handle perspective). The subclass sets this
        // in its onEmitCode().
        GrShaderVar fPositionVar;
        // Used to specify the variable storing the draw's local coordinates. It can be either a
        // float2, float3, or void. It can only be void when no FP needs local coordinates. This
        // variable can be an attribute or local variable, but should not itself be a varying.
        // GrGLSLGeometryProcessor automatically determines if this must be passed to a FS.
        GrShaderVar fLocalCoordVar;
    };

    // Helpers for adding code to write the transformed vertex position. The first simple version
    // just writes a variable named by 'posName' into the position output variable with the
    // assumption that the position is 2D. The second version transforms the input position by a
    // view matrix and the output variable is 2D or 3D depending on whether the view matrix is
    // perspective. Both versions declare the output position variable and will set
    // GrGPArgs::fPositionVar.
    void writeOutputPosition(GrGLSLVertexBuilder*, GrGPArgs*, const char* posName);
    void writeOutputPosition(GrGLSLVertexBuilder*,
                             GrGLSLUniformHandler* uniformHandler,
                             GrGPArgs*,
                             const char* posName,
                             const SkMatrix& mat,
                             UniformHandle* viewMatrixUniform);

    // Helper to transform an existing variable by a given local matrix (e.g. the inverse view
    // matrix). It will declare the transformed local coord variable and will set
    // GrGPArgs::fLocalCoordVar.
    void writeLocalCoord(GrGLSLVertexBuilder*, GrGLSLUniformHandler*, GrGPArgs*,
                         GrShaderVar localVar, const SkMatrix& localMatrix,
                         UniformHandle* localMatrixUniform);

    // GPs that use writeOutputPosition and/or writeLocalCoord must incorporate the matrix type
    // into their key, and should use this function or one of the other related helpers.
    static uint32_t ComputeMatrixKey(const SkMatrix& mat) {
        if (mat.isIdentity()) {
            return 0b00;
        } else if (mat.isScaleTranslate()) {
            return 0b01;
        } else if (!mat.hasPerspective()) {
            return 0b10;
        } else {
            return 0b11;
        }
    }
    static uint32_t ComputeMatrixKeys(const SkMatrix& viewMatrix, const SkMatrix& localMatrix) {
        return (ComputeMatrixKey(viewMatrix) << kMatrixKeyBits) | ComputeMatrixKey(localMatrix);
    }
    static uint32_t AddMatrixKeys(uint32_t flags, const SkMatrix& viewMatrix,
                                  const SkMatrix& localMatrix) {
        // Shifting to make room for the matrix keys shouldn't lose bits
        SkASSERT(((flags << (2 * kMatrixKeyBits)) >> (2 * kMatrixKeyBits)) == flags);
        return (flags << (2 * kMatrixKeyBits)) | ComputeMatrixKeys(viewMatrix, localMatrix);
    }
    static constexpr int kMatrixKeyBits = 2;

private:
    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    // Iterates over the FPs in 'handler' to register additional varyings and uniforms to support
    // VS-promoted local coord evaluation for the FPs. Subclasses must call this with
    // 'localCoordsVar' set to an SkSL variable expression of type 'float2' or 'float3' representing
    // the original local coordinates of the draw.
    //
    // This must happen before FP code emission so that the FPs can find the appropriate varying
    // handles they use in place of explicit coord sampling; it is automatically called after
    // onEmitCode() returns using the value stored in GpArgs::fLocalCoordVar.
    void collectTransforms(GrGLSLVertexBuilder* vb,
                           GrGLSLVaryingHandler* varyingHandler,
                           GrGLSLUniformHandler* uniformHandler,
                           const GrShaderVar& localCoordsVar,
                           FPCoordTransformHandler* handler);

    struct TransformInfo {
        // The vertex-shader output variable to assign the transformed coordinates to
        GrShaderVar                fOutputCoords;
        // The coordinate to be transformed
        GrShaderVar                fLocalCoords;
        // The leaf FP of a transform hierarchy to be evaluated in the vertex shader;
        // this FP will be const-uniform sampled, and all of its parents will have a sample matrix
        // type of none or const-uniform.
        const GrFragmentProcessor* fFP;
    };
    SkTArray<TransformInfo> fTransformInfos;

    using INHERITED = GrGLSLPrimitiveProcessor;
};

#endif
