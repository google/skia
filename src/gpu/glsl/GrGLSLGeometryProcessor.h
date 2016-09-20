/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLGeometryProcessor_DEFINED
#define GrGLSLGeometryProcessor_DEFINED

#include "GrGLSLPrimitiveProcessor.h"

class GrGLSLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLSLGeometryProcessor : public GrGLSLPrimitiveProcessor {
public:
    /* Any general emit code goes in the base class emitCode.  Subclasses override onEmitCode */
    void emitCode(EmitArgs&) override;

protected:
    // A helper which subclasses can use if needed and used above in the default setTransformData().
    void setTransformDataHelper(const SkMatrix& localMatrix,
                                const GrGLSLProgramDataManager& pdman,
                                FPCoordTransformIter*);

    // Emit a uniform matrix for each coord transform.
    void emitTransforms(GrGLSLVertexBuilder* vb,
                        GrGLSLVaryingHandler* varyingHandler,
                        GrGLSLUniformHandler* uniformHandler,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        FPCoordTransformHandler* handler) {
        this->emitTransforms(vb, varyingHandler, uniformHandler,
                             posVar, localCoords, SkMatrix::I(), handler);
    }

    // Emit pre-transformed coords as a vertex attribute per coord-transform.
    void emitTransforms(GrGLSLVertexBuilder*,
                        GrGLSLVaryingHandler*,
                        GrGLSLUniformHandler*,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const SkMatrix& localMatrix,
                        FPCoordTransformHandler*);

    struct GrGPArgs {
        // The variable used by a GP to store its position. It can be
        // either a vec2 or a vec3 depending on the presence of perspective.
        GrShaderVar fPositionVar;
    };

    // Create the correct type of position variable given the CTM
    void setupPosition(GrGLSLVertexBuilder*, GrGPArgs*, const char* posName);
    void setupPosition(GrGLSLVertexBuilder*,
                       GrGLSLUniformHandler* uniformHandler,
                       GrGPArgs*,
                       const char* posName,
                       const SkMatrix& mat,
                       UniformHandle* viewMatrixUniform);

    static uint32_t ComputePosKey(const SkMatrix& mat) {
        if (mat.isIdentity()) {
            return 0x0;
        } else if (!mat.hasPerspective()) {
            return 0x01;
        } else {
            return 0x02;
        }
    }

private:
    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    struct TransformUniform {
        UniformHandle  fHandle;
        SkMatrix       fCurrentValue = SkMatrix::InvalidMatrix();
    };

    SkTArray<TransformUniform, true> fInstalledTransforms;

    typedef GrGLSLPrimitiveProcessor INHERITED;
};

#endif
