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

protected:
    // A helper which subclasses can use to upload coord transform matrices in setData().
    void setTransformDataHelper(const SkMatrix& localMatrix,
                                const GrGLSLProgramDataManager& pdman,
                                const CoordTransformRange&);

    // Emit transformed local coords from the vertex shader as a uniform matrix and varying per
    // coord-transform. localCoordsVar must be a 2- or 3-component vector. If it is 3 then it is
    // assumed to be a 2D homogeneous coordinate.
    void emitTransforms(GrGLSLVertexBuilder*,
                        GrGLSLVaryingHandler*,
                        GrGLSLUniformHandler*,
                        const GrShaderVar& localCoordsVar,
                        const SkMatrix& localMatrix,
                        FPCoordTransformHandler*);

    // Version of above that assumes identity for the local matrix.
    void emitTransforms(GrGLSLVertexBuilder* vb,
                        GrGLSLVaryingHandler* varyingHandler,
                        GrGLSLUniformHandler* uniformHandler,
                        const GrShaderVar& localCoordsVar,
                        FPCoordTransformHandler* handler) {
        this->emitTransforms(vb, varyingHandler, uniformHandler, localCoordsVar, SkMatrix::I(),
                             handler);
    }

    struct GrGPArgs {
        // Used to specify the output variable used by the GP to store its device position. It can
        // either be a float2 or a float3 (in order to handle perspective). The subclass sets this
        // in its onEmitCode().
        GrShaderVar fPositionVar;
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
        GrSLType       fType = kVoid_GrSLType;
        SkMatrix       fCurrentValue = SkMatrix::InvalidMatrix();
    };

    SkTArray<TransformUniform, true> fInstalledTransforms;

    typedef GrGLSLPrimitiveProcessor INHERITED;
};

#endif
