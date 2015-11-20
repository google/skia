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

    // By default we use the identity matrix
    void setTransformData(const GrPrimitiveProcessor&,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataMatrix(SkMatrix::I(), pdman, index, transforms);
    }

    // A helper which subclasses can use if needed
    template <class GeometryProcessor>
    void setTransformDataHelper(const GrPrimitiveProcessor& primProc,
                                const GrGLSLProgramDataManager& pdman,
                                int index,
                                const SkTArray<const GrCoordTransform*, true>& transforms) {
        const GeometryProcessor& gp = primProc.cast<GeometryProcessor>();
        this->setTransformDataMatrix(gp.localMatrix(), pdman, index, transforms);
    }

protected:
    // Emit a uniform matrix for each coord transform.
    void emitTransforms(GrGLSLGPBuilder* gp,
                        GrGLSLVertexBuilder* vb,
                        GrGLSLVaryingHandler* varyingHandler,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const TransformsIn& tin,
                        TransformsOut* tout) {
        this->emitTransforms(gp, vb, varyingHandler, posVar, localCoords, SkMatrix::I(), tin, tout);
    }

    // Emit pre-transformed coords as a vertex attribute per coord-transform.
    void emitTransforms(GrGLSLGPBuilder*,
                        GrGLSLVertexBuilder*,
                        GrGLSLVaryingHandler*,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const SkMatrix& localMatrix,
                        const TransformsIn&,
                        TransformsOut*);

    // caller has emitted transforms via attributes
    void emitTransforms(GrGLSLGPBuilder*,
                        GrGLSLVertexBuilder*,
                        GrGLSLVaryingHandler*,
                        const char* localCoords,
                        const TransformsIn& tin,
                        TransformsOut* tout);

    struct GrGPArgs {
        // The variable used by a GP to store its position. It can be
        // either a vec2 or a vec3 depending on the presence of perspective.
        GrShaderVar fPositionVar;
    };

    // Create the correct type of position variable given the CTM
    void setupPosition(GrGLSLGPBuilder*, GrGLSLVertexBuilder*, GrGPArgs*, const char* posName);
    void setupPosition(GrGLSLGPBuilder*,
                       GrGLSLVertexBuilder*,
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
    void setTransformDataMatrix(const SkMatrix& localMatrix,
                                const GrGLSLProgramDataManager& pdman,
                                int index,
                                const SkTArray<const GrCoordTransform*, true>& transforms) {
        SkSTArray<2, Transform, true>& procTransforms = fInstalledTransforms[index];
        int numTransforms = transforms.count();
        for (int t = 0; t < numTransforms; ++t) {
            SkASSERT(procTransforms[t].fHandle.isValid());
            const SkMatrix& transform = GetTransformMatrix(localMatrix, *transforms[t]);
            if (!procTransforms[t].fCurrentValue.cheapEqualTo(transform)) {
                pdman.setSkMatrix(procTransforms[t].fHandle.toIndex(), transform);
                procTransforms[t].fCurrentValue = transform;
            }
        }
    }

    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    typedef GrGLSLPrimitiveProcessor INHERITED;
};

#endif
