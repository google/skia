/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLCoordTransform.h"
#include "GrDrawEffect.h"
#include "GrTexture.h"
#include "GrGLShaderBuilder.h"

GrGLCoordTransform::EffectKey GrGLCoordTransform::GenKey(const GrDrawEffect& drawEffect,
                                                         int transformIdx) {
    const GrCoordTransform& transform = (*drawEffect.effect())->coordTransform(transformIdx);
    EffectKey key = 0;
    SkMatrix::TypeMask type0 = transform.getMatrix().getType();
    SkMatrix::TypeMask type1;
    if (kLocal_GrCoordSet == transform.sourceCoords()) {
        type1 = drawEffect.getCoordChangeMatrix().getType();
    } else {
        if (drawEffect.programHasExplicitLocalCoords()) {
            // We only make the key indicate that device coords are referenced when the local coords
            // are not actually determined by positions. Otherwise the local coords var and position
            // var are identical.
            key |= kPositionCoords_Flag;
        }
        type1 = SkMatrix::kIdentity_Mask;
    }

    int combinedTypes = type0 | type1;

    bool reverseY = transform.reverseY();

    if (SkMatrix::kPerspective_Mask & combinedTypes) {
        key |= kGeneral_MatrixType;
    } else if (((SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask) & combinedTypes) || reverseY) {
        key |= kNoPersp_MatrixType;
    } else if (SkMatrix::kTranslate_Mask & combinedTypes) {
        key |= kTrans_MatrixType;
    } else {
        key |= kIdentity_MatrixType;
    }
    return key;
}

void GrGLCoordTransform::emitCode(GrGLShaderBuilder* builder,
                                  EffectKey key,
                                  TransformedCoords* transformedCoords,
                                  int suffix) {
    GrGLShaderBuilder::VertexBuilder* vertexBuilder = builder->getVertexBuilder();
    SkASSERT(NULL != vertexBuilder);

    GrSLType varyingType = kVoid_GrSLType;
    const char* uniName;
    switch (key & kMatrixTypeKeyMask) {
        case kIdentity_MatrixType:
            fUniType = kVoid_GrSLType;
            uniName = NULL;
            varyingType = kVec2f_GrSLType;
            break;
        case kTrans_MatrixType:
            fUniType = kVec2f_GrSLType;
            uniName = "StageTranslate";
            varyingType = kVec2f_GrSLType;
            break;
        case kNoPersp_MatrixType:
            fUniType = kMat33f_GrSLType;
            uniName = "StageMatrix";
            varyingType = kVec2f_GrSLType;
            break;
        case kGeneral_MatrixType:
            fUniType = kMat33f_GrSLType;
            uniName = "StageMatrix";
            varyingType = kVec3f_GrSLType;
            break;
        default:
            GrCrash("Unexpected key.");
    }
    SkString suffixedUniName;
    if (kVoid_GrSLType != fUniType) {
        if (0 != suffix) {
            suffixedUniName.append(uniName);
            suffixedUniName.appendf("_%i", suffix);
            uniName = suffixedUniName.c_str();
        }
        fUni = builder->addUniform(GrGLShaderBuilder::kVertex_Visibility,
                                   fUniType,
                                   uniName,
                                   &uniName);
    }

    const char* varyingName = "MatrixCoord";
    SkString suffixedVaryingName;
    if (0 != suffix) {
        suffixedVaryingName.append(varyingName);
        suffixedVaryingName.appendf("_%i", suffix);
        varyingName = suffixedVaryingName.c_str();
    }
    const char* vsVaryingName;
    const char* fsVaryingName;
    vertexBuilder->addVarying(varyingType, varyingName, &vsVaryingName, &fsVaryingName);

    const GrGLShaderVar& coords = (kPositionCoords_Flag & key) ?
                                      vertexBuilder->positionAttribute() :
                                      vertexBuilder->localCoordsAttribute();
    // varying = matrix * coords (logically)
    switch (fUniType) {
        case kVoid_GrSLType:
            SkASSERT(kVec2f_GrSLType == varyingType);
            vertexBuilder->vsCodeAppendf("\t%s = %s;\n", vsVaryingName, coords.c_str());
            break;
        case kVec2f_GrSLType:
            SkASSERT(kVec2f_GrSLType == varyingType);
            vertexBuilder->vsCodeAppendf("\t%s = %s + %s;\n",
                                         vsVaryingName, uniName, coords.c_str());
            break;
        case kMat33f_GrSLType: {
            SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
            if (kVec2f_GrSLType == varyingType) {
                vertexBuilder->vsCodeAppendf("\t%s = (%s * vec3(%s, 1)).xy;\n",
                                             vsVaryingName, uniName, coords.c_str());
            } else {
                vertexBuilder->vsCodeAppendf("\t%s = %s * vec3(%s, 1);\n",
                                             vsVaryingName, uniName, coords.c_str());
            }
            break;
        }
        default:
            GrCrash("Unexpected uniform type.");
    }
    SkASSERT(NULL != transformedCoords);
    transformedCoords->fName = fsVaryingName;
    transformedCoords->fType = varyingType;
    transformedCoords->fVSName = vsVaryingName;
}

void GrGLCoordTransform::setData(const GrGLUniformManager& uniformManager,
                                 const GrDrawEffect& drawEffect,
                                 int transformIdx) {
    SkASSERT(fUni.isValid() != (kVoid_GrSLType == fUniType));
    const GrCoordTransform& transform = (*drawEffect.effect())->coordTransform(transformIdx);
    const SkMatrix& matrix = transform.getMatrix();
    const SkMatrix& coordChangeMatrix = kLocal_GrCoordSet == transform.sourceCoords() ?
                                            drawEffect.getCoordChangeMatrix() :
                                            SkMatrix::I();
    switch (fUniType) {
        case kVoid_GrSLType:
            SkASSERT(matrix.isIdentity());
            SkASSERT(coordChangeMatrix.isIdentity());
            SkASSERT(!transform.reverseY());
            return;
        case kVec2f_GrSLType: {
            SkASSERT(SkMatrix::kTranslate_Mask == (matrix.getType() | coordChangeMatrix.getType()));
            SkASSERT(!transform.reverseY());
            SkScalar tx = matrix[SkMatrix::kMTransX] + (coordChangeMatrix)[SkMatrix::kMTransX];
            SkScalar ty = matrix[SkMatrix::kMTransY] + (coordChangeMatrix)[SkMatrix::kMTransY];
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
            if (transform.reverseY()) {
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
