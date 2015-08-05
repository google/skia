/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLPrimitiveProcessor.h"

class GrGLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor : public GrGLPrimitiveProcessor {
public:
    /* Any general emit code goes in the base class emitCode.  Subclasses override onEmitCode */
    void emitCode(EmitArgs&) override;

    // By default we use the identity matrix
    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrGLProgramDataManager& pdman,
                                  int index,
                                  const SkTArray<const GrCoordTransform*, true>& transforms) {
        this->setTransformDataMatrix(SkMatrix::I(), pdman, index, transforms);
    }

    // A helper which subclasses can use if needed
    template <class GeometryProcessor>
    void setTransformDataHelper(const GrPrimitiveProcessor& primProc,
                                const GrGLProgramDataManager& pdman,
                                int index,
                                const SkTArray<const GrCoordTransform*, true>& transforms) {
        const GeometryProcessor& gp = primProc.cast<GeometryProcessor>();
        this->setTransformDataMatrix(gp.localMatrix(), pdman, index, transforms);
    }

protected:
    // Emit a uniform matrix for each coord transform.
    void emitTransforms(GrGLGPBuilder* gp,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const TransformsIn& tin,
                        TransformsOut* tout) {
        this->emitTransforms(gp, posVar, localCoords, SkMatrix::I(), tin, tout);
    }

    // Emit pre-transformed coords as a vertex attribute per coord-transform.
    void emitTransforms(GrGLGPBuilder*,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const SkMatrix& localMatrix,
                        const TransformsIn&,
                        TransformsOut*);

    // caller has emitted transforms via attributes
    void emitTransforms(GrGLGPBuilder*,
                        const char* localCoords,
                        const TransformsIn& tin,
                        TransformsOut* tout);

    struct GrGPArgs {
        // The variable used by a GP to store its position. It can be
        // either a vec2 or a vec3 depending on the presence of perspective.
        GrShaderVar fPositionVar;
    };

    // Create the correct type of position variable given the CTM
    void setupPosition(GrGLGPBuilder*, GrGPArgs*, const char* posName);
    void setupPosition(GrGLGPBuilder*, GrGPArgs*, const char* posName, const SkMatrix& mat,
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
                                const GrGLProgramDataManager& pdman,
                                int index,
                                const SkTArray<const GrCoordTransform*, true>& transforms) {
        SkSTArray<2, Transform, true>& procTransforms = fInstalledTransforms[index];
        int numTransforms = transforms.count();
        for (int t = 0; t < numTransforms; ++t) {
            SkASSERT(procTransforms[t].fHandle.isValid());
            const SkMatrix& transform = GetTransformMatrix(localMatrix, *transforms[t]);
            if (!procTransforms[t].fCurrentValue.cheapEqualTo(transform)) {
                pdman.setSkMatrix(procTransforms[t].fHandle.convertToUniformHandle(), transform);
                procTransforms[t].fCurrentValue = transform;
            }
        }
    }

    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
