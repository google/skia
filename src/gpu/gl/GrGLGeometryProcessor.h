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
    void emitCode(EmitArgs&) SK_OVERRIDE;

    void setTransformData(const GrPrimitiveProcessor&,
                          const GrGLProgramDataManager&,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms);

protected:
    // Many GrGeometryProcessors do not need explicit local coords
    void emitTransforms(GrGLGPBuilder* gp,
                        const GrShaderVar& posVar,
                        const SkMatrix& localMatrix,
                        const TransformsIn& tin,
                        TransformsOut* tout) {
        this->emitTransforms(gp, posVar, posVar.c_str(), localMatrix, tin, tout);
    }

    void emitTransforms(GrGLGPBuilder*,
                        const GrShaderVar& posVar,
                        const char* localCoords,
                        const SkMatrix& localMatrix,
                        const TransformsIn&,
                        TransformsOut*);

    struct GrGPArgs {
        // The variable used by a GP to store its position. It can be
        // either a vec2 or a vec3 depending on the presence of perspective.
        GrShaderVar fPositionVar;
    };

    // Create the correct type of position variable given the CTM
    void setupPosition(GrGLGPBuilder* pb,
                       GrGPArgs* gpArgs,
                       const char* posName,
                       const SkMatrix& mat);

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

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
