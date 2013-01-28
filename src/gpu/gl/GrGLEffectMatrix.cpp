/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLEffectMatrix.h"
#include "GrTexture.h"

GrGLEffect::EffectKey GrGLEffectMatrix::GenKey(const SkMatrix& effectMatrix,
                                               const SkMatrix& coordChangeMatrix,
                                               const GrTexture* texture) {
    SkMatrix::TypeMask type0 = effectMatrix.getType();
    SkMatrix::TypeMask type1 = coordChangeMatrix.getType();

    static const int kNonTransMask = SkMatrix::kAffine_Mask |
                                     SkMatrix::kScale_Mask  |
                                     SkMatrix::kPerspective_Mask;
    int combinedTypes = type0 | type1;

    bool reverseY = (NULL != texture) && kBottomLeft_GrSurfaceOrigin == texture->origin();

    if (SkMatrix::kPerspective_Mask & combinedTypes) {
        return kGeneral_Key;
    } else if ((kNonTransMask & combinedTypes) || reverseY) {
        return kNoPersp_Key;
    } else if (kTrans_Key & combinedTypes) {
        return kTrans_Key;
    } else {
        GrAssert(effectMatrix.isIdentity() && coordChangeMatrix.isIdentity());
        return kIdentity_Key;
    }
}

GrSLType GrGLEffectMatrix::emitCode(GrGLShaderBuilder* builder,
                                    EffectKey key,
                                    const char* vertexCoords,
                                    const char** fsCoordName,
                                    const char** vsCoordName,
                                    const char* suffix) {
    GrSLType varyingType;
    const char* uniName;
    key &= kKeyMask;
    switch (key) {
        case kIdentity_Key:
            fUniType = kVoid_GrSLType;
            varyingType = kVec2f_GrSLType;
            break;
        case kTrans_Key:
            fUniType = kVec2f_GrSLType;
            uniName = "StageTranslate";
            varyingType = kVec2f_GrSLType;
            break;
        case kNoPersp_Key:
            fUniType = kMat33f_GrSLType;
            uniName = "StageMatrix";
            varyingType = kVec2f_GrSLType;
            break;
        case kGeneral_Key:
            fUniType = kMat33f_GrSLType;
            uniName = "StageMatrix";
            varyingType = kVec3f_GrSLType;
            break;
        default:
            GrCrash("Unexpected key.");
    }
    SkString suffixedUniName;
    if (NULL != suffix) {
        suffixedUniName.append(uniName);
        suffixedUniName.append(suffix);
        uniName = suffixedUniName.c_str();
    }
    if (kVoid_GrSLType != fUniType) {
        fUni = builder->addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                   fUniType,
                                   uniName,
                                   &uniName);
    }

    const char* varyingName = "StageCoord";
    SkString suffixedVaryingName;
    if (NULL != suffix) {
        suffixedVaryingName.append(varyingName);
        suffixedVaryingName.append(suffix);
        varyingName = suffixedVaryingName.c_str();
    }
    const char* vsVaryingName;
    const char* fsVaryingName;
    builder->addVarying(varyingType, varyingName, &vsVaryingName, &fsVaryingName);

    // varying = matrix * vertex-coords (logically)
    switch (fUniType) {
        case kVoid_GrSLType:
            GrAssert(kVec2f_GrSLType == varyingType);
            builder->fVSCode.appendf("\t%s = %s;\n", vsVaryingName, vertexCoords);
            break;
        case kVec2f_GrSLType:
            GrAssert(kVec2f_GrSLType == varyingType);
            builder->fVSCode.appendf("\t%s = %s + %s;\n", vsVaryingName, uniName, vertexCoords);
            break;
        case kMat33f_GrSLType: {
            GrAssert(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
            if (kVec2f_GrSLType == varyingType) {
                builder->fVSCode.appendf("\t%s = (%s * vec3(%s, 1)).xy;\n",
                                         vsVaryingName, uniName, vertexCoords);
            } else {
                builder->fVSCode.appendf("\t%s = %s * vec3(%s, 1);\n",
                                         vsVaryingName, uniName, vertexCoords);
            }
            break;
        }
        default:
            GrCrash("Unexpected uniform type.");
    }
    if (NULL != vsCoordName) {
        *vsCoordName = vsVaryingName;
    }
    if (NULL != fsCoordName) {
        *fsCoordName = fsVaryingName;
    }
    return varyingType;
}

/**
    * This is similar to emitCode except that it performs perspective division in the FS if the
    * texture coordinates have a w coordinate. The fsCoordName always refers to a vec2f.
    */
void GrGLEffectMatrix::emitCodeMakeFSCoords2D(GrGLShaderBuilder* builder,
                                              EffectKey key,
                                              const char* vertexCoords,
                                              const char** fsCoordName,
                                              const char** vsVaryingName,
                                              GrSLType* vsVaryingType,
                                              const char* suffix) {
    const char* fsVaryingName;

    GrSLType varyingType = this->emitCode(builder,
                                          key,
                                          vertexCoords,
                                          &fsVaryingName,
                                          vsVaryingName,
                                          suffix);
    if (kVec3f_GrSLType == varyingType) {

        const char* coordName = "coords2D";
        SkString suffixedCoordName;
        if (NULL != suffix) {
            suffixedCoordName.append(coordName);
            suffixedCoordName.append(suffix);
            coordName = suffixedCoordName.c_str();
        }
        builder->fFSCode.appendf("\tvec2 %s = %s.xy / %s.z;",
                                    coordName, fsVaryingName, fsVaryingName);
        if (NULL != fsCoordName) {
            *fsCoordName = coordName;
        }
    } else if(NULL != fsCoordName) {
        *fsCoordName = fsVaryingName;
    }
    if (NULL != vsVaryingType) {
        *vsVaryingType = varyingType;
    }
}

void GrGLEffectMatrix::setData(const GrGLUniformManager& uniformManager,
                              const SkMatrix& matrix,
                              const SkMatrix& coordChangeMatrix,
                              const GrTexture* texture) {
    GrAssert((GrGLUniformManager::kInvalidUniformHandle == fUni) ==
                (kVoid_GrSLType == fUniType));
    switch (fUniType) {
        case kVoid_GrSLType:
            GrAssert(matrix.isIdentity());
            GrAssert(coordChangeMatrix.isIdentity());
            GrAssert(NULL == texture || kTopLeft_GrSurfaceOrigin == texture->origin());
            return;
        case kVec2f_GrSLType: {
            GrAssert(SkMatrix::kTranslate_Mask == (matrix.getType() | coordChangeMatrix.getType()));
            GrAssert(NULL == texture || kTopLeft_GrSurfaceOrigin == texture->origin());
            SkScalar tx = matrix[SkMatrix::kMTransX] + coordChangeMatrix[SkMatrix::kMTransX];
            SkScalar ty = matrix[SkMatrix::kMTransY] + coordChangeMatrix[SkMatrix::kMTransY];
            if (fPrevMatrix.get(SkMatrix::kMTransX) != tx ||
                fPrevMatrix.get(SkMatrix::kMTransY) != ty) {
                uniformManager.set2f(fUni, tx, ty);
                fPrevMatrix.set(SkMatrix::kMTransX, tx);
                fPrevMatrix.set(SkMatrix::kMTransY, ty);
            }
            break;
        }
        case kMat33f_GrSLType: {
            SkMatrix combined;
            combined.setConcat(matrix, coordChangeMatrix);
            if (NULL != texture && kBottomLeft_GrSurfaceOrigin == texture->origin()) {
                // combined.postScale(1,-1);
                // combined.postTranslate(0,1);
                combined.set(SkMatrix::kMSkewY,
                    combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
                combined.set(SkMatrix::kMScaleY,
                    combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
                combined.set(SkMatrix::kMTransY,
                    combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
            }
            if (!fPrevMatrix.cheapEqualTo(combined)) {
                uniformManager.setSkMatrix(fUni, combined);
                fPrevMatrix = combined;
            }
            break;
        }
        default:
            GrCrash("Unexpected uniform type.");
    }
}
