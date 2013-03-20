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

/**
 * This is a helper to implement a matrix in a GrGLEffect that operates on incoming coords in the
 * vertex shader and writes them to an attribute to be used in the fragment shader. When the input
 * coords in the vertex shader are local coordinates this class accounts for the coord change matrix
 * communicated via GrDrawEffect. The input coords may also be positions and in this case the coord
 * change matrix is ignored. The GrGLEffectMatrix will emit different code based on the type of
 * matrix and thus must contribute to the effect's key.
 *
 * This class cannot be used to apply a matrix to coordinates that come in the form of custom vertex
 * attributes.
 */
class GrGLEffectMatrix {
private:
    // We specialize the generated code for each of these matrix types.
    enum MatrixTypes {
        kIdentity_MatrixType    = 0,
        kTrans_MatrixType       = 1,
        kNoPersp_MatrixType     = 2,
        kGeneral_MatrixType     = 3,
    };
    // The key for is made up of a matrix type and a bit that indicates the source of the input
    // coords.
    enum {
        kMatrixTypeKeyBits      = 2,
        kMatrixTypeKeyMask      = (1 << kMatrixTypeKeyBits) - 1,
        kPositionCoords_Flag    = (1 << kMatrixTypeKeyBits),
        kKeyBitsPrivate         = kMatrixTypeKeyBits + 1,
    };

public:

    typedef GrEffect::CoordsType CoordsType;

    typedef GrGLEffect::EffectKey EffectKey;

    /**
     * The matrix uses kKeyBits of the effect's EffectKey. A GrGLEffect may place these bits at an
     * arbitrary shift in its final key. However, when GrGLEffectMatrix::emitCode*() code is called
     * the relevant bits must be in the lower kKeyBits of the key parameter.
     */
    enum {
        kKeyBits = kKeyBitsPrivate,
        kKeyMask = (1 << kKeyBits) - 1,
    };

    GrGLEffectMatrix(CoordsType coordsType)
        : fUni(GrGLUniformManager::kInvalidUniformHandle)
        , fCoordsType(coordsType) {
        GrAssert(GrEffect::kLocal_CoordsType == coordsType ||
                 GrEffect::kPosition_CoordsType == coordsType);
        fPrevMatrix = SkMatrix::InvalidMatrix();
    }

    /**
     * Generates the key for the portion of the code emitted by this class's emitCode() function.
     * Pass a texture to make GrGLEffectMatrix automatically adjust for the texture's origin. Pass
     * NULL when not using the EffectMatrix for a texture lookup, or if the GrGLEffect subclass
     * wants to handle origin adjustments in some other manner. The coords type param must match the
     * param that would be used to initialize GrGLEffectMatrix for the generating GrEffect.
     */
    static EffectKey GenKey(const SkMatrix& effectMatrix,
                            const GrDrawEffect&,
                            CoordsType,
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
                      const char** fsCoordName, /* optional */
                      const char** vsCoordName = NULL,
                      const char* suffix = NULL);

    /**
     * This is similar to emitCode except that it performs perspective division in the FS if the
     * texture coordinates have a w coordinate. The fsCoordName always refers to a vec2f.
     */
    void emitCodeMakeFSCoords2D(GrGLShaderBuilder*,
                                EffectKey,
                                const char** fsCoordName, /* optional */
                                const char** vsVaryingName = NULL,
                                GrSLType* vsVaryingType = NULL,
                                const char* suffix = NULL);
    /**
     * Call from a GrGLEffect's subclass to update the texture matrix. The effectMatrix and texture
     * params should match those used with GenKey.
     */
    void setData(const GrGLUniformManager& uniformManager,
                 const SkMatrix& effectMatrix,
                 const GrDrawEffect& drawEffect,
                 const GrTexture*);

    GrGLUniformManager::UniformHandle fUni;
    GrSLType                          fUniType;
    SkMatrix                          fPrevMatrix;
    CoordsType                        fCoordsType;
};

#endif
