/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLGeometryProcessor.h"

#include "GrCoordTransform.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

void GrGLSLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGLSLVertexBuilder* vBuilder = args.fVertBuilder;
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);
    vBuilder->transformToNormalizedDeviceSpace(gpArgs.fPositionVar, args.fRTAdjustName);
    if (kHighFloat2_GrSLType == gpArgs.fPositionVar.getType() ||
        kHalf2_GrSLType == gpArgs.fPositionVar.getType()) {
        args.fVaryingHandler->setNoPerspective();
    }
}

void GrGLSLGeometryProcessor::emitTransforms(GrGLSLVertexBuilder* vb,
                                             GrGLSLVaryingHandler* varyingHandler,
                                             GrGLSLUniformHandler* uniformHandler,
                                             const GrShaderVar& posVar,
                                             const char* localCoords,
                                             const SkMatrix& localMatrix,
                                             FPCoordTransformHandler* handler) {
    int i = 0;
    while (const GrCoordTransform* coordTransform = handler->nextCoordTransform()) {
        SkString strUniName;
        strUniName.printf("CoordTransformMatrix_%d", i);
        GrSLType varyingType;

        uint32_t type = coordTransform->getMatrix().getType();
        type |= localMatrix.getType();

        varyingType = SkToBool(SkMatrix::kPerspective_Mask & type) ? kHighFloat3_GrSLType :
                                                                     kHighFloat2_GrSLType;
        const char* uniName;


        fInstalledTransforms.push_back().fHandle = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                                              kHighFloat3x3_GrSLType,
                                                                              strUniName.c_str(),
                                                                              &uniName).toIndex();
        SkString strVaryingName;
        strVaryingName.printf("TransformedCoords_%d", i);

        GrGLSLVertToFrag v(varyingType);
        varyingHandler->addVarying(strVaryingName.c_str(), &v, kHigh_GrSLPrecision);

        SkASSERT(kHighFloat2_GrSLType == varyingType || kHighFloat3_GrSLType == varyingType);
        handler->specifyCoordsForCurrCoordTransform(SkString(v.fsIn()), varyingType);

        if (kHighFloat2_GrSLType == varyingType) {
            vb->codeAppendf("%s = (%s * highfloat3(%s, 1)).xy;", v.vsOut(), uniName, localCoords);
        } else {
            vb->codeAppendf("%s = %s * highfloat3(%s, 1);", v.vsOut(), uniName, localCoords);
        }
        ++i;
    }
}

void GrGLSLGeometryProcessor::setTransformDataHelper(const SkMatrix& localMatrix,
                                                     const GrGLSLProgramDataManager& pdman,
                                                     FPCoordTransformIter* transformIter) {
    int i = 0;
    while (const GrCoordTransform* coordTransform = transformIter->next()) {
        const SkMatrix& m = GetTransformMatrix(localMatrix, *coordTransform);
        if (!fInstalledTransforms[i].fCurrentValue.cheapEqualTo(m)) {
            pdman.setSkMatrix(fInstalledTransforms[i].fHandle.toIndex(), m);
            fInstalledTransforms[i].fCurrentValue = m;
        }
        ++i;
    }
    SkASSERT(i == fInstalledTransforms.count());
}

void GrGLSLGeometryProcessor::writeOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                                  GrGPArgs* gpArgs,
                                                  const char* posName) {
    gpArgs->fPositionVar.set(kHighFloat2_GrSLType, "pos2");
    vertBuilder->codeAppendf("highfloat2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
}

void GrGLSLGeometryProcessor::writeOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                                  GrGLSLUniformHandler* uniformHandler,
                                                  GrGPArgs* gpArgs,
                                                  const char* posName,
                                                  const SkMatrix& mat,
                                                  UniformHandle* viewMatrixUniform) {
    if (mat.isIdentity()) {
        gpArgs->fPositionVar.set(kHighFloat2_GrSLType, "pos2");
        vertBuilder->codeAppendf("highfloat2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
    } else {
        const char* viewMatrixName;
        *viewMatrixUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                        kHighFloat3x3_GrSLType,
                                                        "uViewM",
                                                        &viewMatrixName);
        if (!mat.hasPerspective()) {
            gpArgs->fPositionVar.set(kHighFloat2_GrSLType, "pos2");
            vertBuilder->codeAppendf("highfloat2 %s = (%s * highfloat3(%s, 1)).xy;",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        } else {
            gpArgs->fPositionVar.set(kHighFloat3_GrSLType, "pos3");
            vertBuilder->codeAppendf("highfloat3 %s = %s * highfloat3(%s, 1);",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        }
    }
}
