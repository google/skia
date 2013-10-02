/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLCoordTransform_DEFINED
#define GrGLCoordTransform_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrCoordTransform.h"
#include "GrGLUniformManager.h"

class GrTexture;
class GrGLShaderBuilder;

/**
 * This is a helper class used by the framework to implement a coordinate transform that operates on
 * incoming coords in the vertex shader and writes them to a varying to be used in the fragment
 * shader. Effects should not use this class directly, but instead call GrEffect::addCoordTransform.
 * When the input coords are local coordinates this class accounts for the coord change matrix
 * communicated via GrDrawEffect. The input coords may also be positions and in this case the coord
 * change matrix is ignored. The GrGLCoordTransform may emit different code based on the type of
 * matrix and thus must contribute to the effect's key.
 *
 * This class cannot be used to apply a matrix to coordinates that come in the form of custom vertex
 * attributes.
 */
class GrGLCoordTransform {
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

    typedef GrBackendEffectFactory::EffectKey EffectKey;

    /**
     * A GrGLCoordTransform key is kKeyBits long. The framework automatically generates and includes
     * these in EffectKeys.
     */
    enum {
        kKeyBits = kKeyBitsPrivate,
        kKeyMask = (1 << kKeyBits) - 1,
    };

    GrGLCoordTransform() { fPrevMatrix = SkMatrix::InvalidMatrix(); }

    /**
     * Generates the key for the portion of the code emitted by this class's emitCode() function.
     */
    static EffectKey GenKey(const GrDrawEffect&, int transformIdx);

    /**
     * Stores the name and type of a transformed set of coordinates. This class is passed to
     * GrGLEffect::emitCode.
     */
    class TransformedCoords {
    public:
        const char* c_str() const { return fName.c_str(); }
        GrSLType type() const { return fType; }
        const SkString& getName() const { return fName; }
        // TODO: Remove the VS name when we have vertexless shaders, and gradients are reworked.
        const SkString& getVSName() const { return fVSName; }

    private:
        friend class GrGLCoordTransform;

        SkString fName;
        GrSLType fType;
        SkString fVSName;
    };

    /**
     * Emits code to implement the matrix in the VS. A varying is added as an output of the VS and
     * input to the FS. The varying may be either a vec2f or vec3f depending upon whether
     * perspective interpolation is required or not. The names of the varying in the VS and FS as
     * well as its type are written to the TransformedCoords* object. The suffix is an optional
     * parameter that can be used to make all variables emitted by the object unique within a stage.
     * It is only necessary if multiple GrGLCoordTransform objects are used by a single GrGLEffect.
     */
    void emitCode(GrGLShaderBuilder*, EffectKey, TransformedCoords*, int suffix = 0);

    /**
     * Call from a GrGLEffect's subclass to update the texture matrix. The matrix and reverseY value
     * should match those used with GenKey.
     */
    void setData(const GrGLUniformManager&, const GrDrawEffect&, int transformIdx);

    GrGLUniformManager::UniformHandle fUni;
    GrSLType                          fUniType;
    SkMatrix                          fPrevMatrix;
};

#endif
