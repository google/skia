/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

void GrGLSLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);

    GrGLSLVertexBuilder* vBuilder = args.fVertBuilder;
    if (!args.fGP.willUseGeoShader()) {
        // Emit the vertex position to the hardware in the normalized window coordinates it expects.
        SkASSERT(kFloat2_GrSLType == gpArgs.fPositionVar.getType() ||
                 kFloat3_GrSLType == gpArgs.fPositionVar.getType());
        vBuilder->emitNormalizedSkPosition(gpArgs.fPositionVar.c_str(), args.fRTAdjustName,
                                           gpArgs.fPositionVar.getType());
        if (kFloat2_GrSLType == gpArgs.fPositionVar.getType()) {
            args.fVaryingHandler->setNoPerspective();
        }
    } else {
        // Since we have a geometry shader, leave the vertex position in Skia device space for now.
        // The geometry Shader will operate in device space, and then convert the final positions to
        // normalized hardware window coordinates under the hood, once everything else has finished.
        // The subclass must call setNoPerspective on the varying handler, if applicable.
        vBuilder->codeAppendf("sk_Position = float4(%s", gpArgs.fPositionVar.c_str());
        switch (gpArgs.fPositionVar.getType()) {
            case kFloat_GrSLType:
                vBuilder->codeAppend(", 0"); // fallthru.
            case kFloat2_GrSLType:
                vBuilder->codeAppend(", 0"); // fallthru.
            case kFloat3_GrSLType:
                vBuilder->codeAppend(", 1"); // fallthru.
            case kFloat4_GrSLType:
                vBuilder->codeAppend(");");
                break;
            default:
                SK_ABORT("Invalid position var type");
                break;
        }
    }
}

void GrGLSLGeometryProcessor::emitTransforms(GrGLSLVertexBuilder* vb,
                                             GrGLSLVaryingHandler* varyingHandler,
                                             GrGLSLUniformHandler* uniformHandler,
                                             const GrShaderVar& localCoordsVar,
                                             const SkMatrix& localMatrix,
                                             FPCoordTransformHandler* handler) {
    SkASSERT(GrSLTypeIsFloatType(localCoordsVar.getType()));
    SkASSERT(2 == GrSLTypeVecLength(localCoordsVar.getType()) ||
             3 == GrSLTypeVecLength(localCoordsVar.getType()));

    bool threeComponentLocalCoords = 3 == GrSLTypeVecLength(localCoordsVar.getType());
    SkString localCoords;
    if (threeComponentLocalCoords) {
        localCoords = localCoordsVar.getName();
    } else {
        localCoords.printf("float3(%s, 1)", localCoordsVar.c_str());
    }
    int i = 0;
    while (const GrCoordTransform* coordTransform = handler->nextCoordTransform()) {
        SkString strUniName;
        strUniName.printf("CoordTransformMatrix_%d", i);
        const char* uniName;
        fInstalledTransforms.push_back().fHandle = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                                              kFloat3x3_GrSLType,
                                                                              strUniName.c_str(),
                                                                              &uniName).toIndex();
        GrSLType varyingType = kFloat2_GrSLType;
        if (localMatrix.hasPerspective() || coordTransform->getMatrix().hasPerspective()
            || threeComponentLocalCoords) {
            varyingType = kFloat3_GrSLType;
        }
        SkString strVaryingName;
        strVaryingName.printf("TransformedCoords_%d", i);
        GrGLSLVarying v(varyingType);
        varyingHandler->addVarying(strVaryingName.c_str(), &v);

        handler->specifyCoordsForCurrCoordTransform(SkString(v.fsIn()), varyingType);

        if (kFloat2_GrSLType == varyingType) {
            vb->codeAppendf("%s = (%s * %s).xy;", v.vsOut(), uniName, localCoords.c_str());
        } else {
            vb->codeAppendf("%s = %s * %s;", v.vsOut(), uniName, localCoords.c_str());
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
    gpArgs->fPositionVar.set(kFloat2_GrSLType, "pos2");
    vertBuilder->codeAppendf("float2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
}

void GrGLSLGeometryProcessor::writeOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                                  GrGLSLUniformHandler* uniformHandler,
                                                  GrGPArgs* gpArgs,
                                                  const char* posName,
                                                  const SkMatrix& mat,
                                                  UniformHandle* viewMatrixUniform) {
    if (mat.isIdentity()) {
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "pos2");
        vertBuilder->codeAppendf("float2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
    } else {
        const char* viewMatrixName;
        *viewMatrixUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                        kFloat3x3_GrSLType,
                                                        "uViewM",
                                                        &viewMatrixName);
        if (!mat.hasPerspective()) {
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "pos2");
            vertBuilder->codeAppendf("float2 %s = (%s * float3(%s, 1)).xy;",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        } else {
            gpArgs->fPositionVar.set(kFloat3_GrSLType, "pos3");
            vertBuilder->codeAppendf("float3 %s = %s * float3(%s, 1);",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        }
    }
}
