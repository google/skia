/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLEffectMatrix_DEFINED
#define GrGLEffectMatrix_DEFINED

#include "GrGLEffect.h"
#include "SkMatrix.h"

class GrTexture;
class SkRandom;

/**
 * This is a helper to implement a texture matrix in a GrGLEffect.
 */
class GrGLEffectMatrix {
public:
    typedef GrGLEffect::EffectKey EffectKey;
    /**
     * The matrix uses kKeyBits of the effect's EffectKey. A GrGLEffect may place these bits at an
     * arbitrary shift in its final key. However, when GrGLEffectMatrix::emitCode*() code is called
     * the relevant bits must be in the lower kKeyBits of the key parameter.
     */
    enum {
        kKeyBits = 2,
        kKeyMask = (1 << kKeyBits) - 1,
    };

    GrGLEffectMatrix() : fUni(GrGLUniformManager::kInvalidUniformHandle) {
        fPrevMatrix = SkMatrix::InvalidMatrix();
    }

    /**
     * Generates the key for the portion of the code emitted by this class's emitCode() function.
     * Pass a texture to make GrGLEffectMatrix automatically adjust for the texture's origin. Pass
     * NULL when not using the EffectMatrix for a texture lookups, or if the GrGLEffect subclass
     * wants to handle origin adjustments in some other manner. coordChangeMatrix is the matrix
     * from GrEffectStage.
     */
    static EffectKey GenKey(const SkMatrix& effectMatrix,
                            const SkMatrix& coordChangeMatrix,
                            const GrTexture*);

    /**
     * Emits code to implement the matrix in the VS. A varying is added as an output of the VS and
     * input to the FS. The varying may be either a vec2f or vec3f depending upon whether
     * perspective interpolation is required or not. The names of the varying in the VS and FS are
     * are returned as output parameters and the type of the varying is the return value. The suffix
     * is an optional parameter that can be used to make all variables emitted by the object
     * unique within a stage. It is only necessary if multiple GrGLEffectMatrix objects are used by
     * a GrGLEffect.
     */
    GrSLType emitCode(GrGLShaderBuilder*,
                      EffectKey,
                      const char* vertexCoords,
                      const char** fsCoordName, /* optional */
                      const char** vsCoordName = NULL,
                      const char* suffix = NULL);

    /**
     * This is similar to emitCode except that it performs perspective division in the FS if the
     * texture coordinates have a w coordinate. The fsCoordName always refers to a vec2f.
     */
    void emitCodeMakeFSCoords2D(GrGLShaderBuilder*,
                                EffectKey,
                                const char* vertexCoords,
                                const char** fsCoordName, /* optional */
                                const char** vsVaryingName = NULL,
                                GrSLType* vsVaryingType = NULL,
                                const char* suffix = NULL);
    /**
     * Call from a GrGLEffect's subclass to update the texture matrix. The matrix,
     * coordChangeMatrix, and texture params should match those used with GenKey.
     */
    void setData(const GrGLUniformManager& uniformManager,
                 const SkMatrix& effectMatrix,
                 const SkMatrix& coordChangeMatrix,
                 const GrTexture*);

private:
    enum {
        kIdentity_Key   = 0,
        kTrans_Key      = 1,
        kNoPersp_Key    = 2,
        kGeneral_Key    = 3,
    };

    GrGLUniformManager::UniformHandle fUni;
    GrSLType                          fUniType;
    SkMatrix                          fPrevMatrix;
};

#endif
