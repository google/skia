/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGeometryProcessor.h"

#include "builders/GrGLProgramBuilder.h"

void GrGLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);
    vsBuilder->transformToNormalizedDeviceSpace(gpArgs.fPositionVar);
}

void GrGLGeometryProcessor::emitTransforms(GrGLGPBuilder* pb,
                                           const GrShaderVar& posVar,
                                           const char* localCoords,
                                           const SkMatrix& localMatrix,
                                           const TransformsIn& tin,
                                           TransformsOut* tout) {
    GrGLVertexBuilder* vb = pb->getVertexShaderBuilder();
    tout->push_back_n(tin.count());
    fInstalledTransforms.push_back_n(tin.count());
    for (int i = 0; i < tin.count(); i++) {
        const ProcCoords& coordTransforms = tin[i];
        fInstalledTransforms[i].push_back_n(coordTransforms.count());
        for (int t = 0; t < coordTransforms.count(); t++) {
            SkString strUniName("StageMatrix");
            strUniName.appendf("_%i_%i", i, t);
            GrSLType varyingType;

            GrCoordSet coordType = coordTransforms[t]->sourceCoords();
            uint32_t type = coordTransforms[t]->getMatrix().getType();
            if (kLocal_GrCoordSet == coordType) {
                type |= localMatrix.getType();
            }
            varyingType = SkToBool(SkMatrix::kPerspective_Mask & type) ? kVec3f_GrSLType :
                                                                         kVec2f_GrSLType;
            GrSLPrecision precision = coordTransforms[t]->precision();

            const char* uniName;
            fInstalledTransforms[i][t].fHandle =
                    pb->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                   kMat33f_GrSLType, precision,
                                   strUniName.c_str(),
                                   &uniName).toShaderBuilderIndex();

            SkString strVaryingName("MatrixCoord");
            strVaryingName.appendf("_%i_%i", i, t);

            GrGLVertToFrag v(varyingType);
            pb->addVarying(strVaryingName.c_str(), &v, precision);

            SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
            SkNEW_APPEND_TO_TARRAY(&(*tout)[i], GrGLProcessor::TransformedCoords,
                                   (SkString(v.fsIn()), varyingType));

            // varying = matrix * coords (logically)
            if (kDevice_GrCoordSet == coordType) {
                if (kVec2f_GrSLType == varyingType) {
                    if (kVec2f_GrSLType == posVar.getType()) {
                        vb->codeAppendf("%s = (%s * vec3(%s, 1)).xy;",
                                        v.vsOut(), uniName, posVar.c_str());
                    } else {
                        // The brackets here are just to scope the temp variable
                        vb->codeAppendf("{ vec3 temp = %s * %s;", uniName, posVar.c_str());
                        vb->codeAppendf("%s = vec2(temp.x/temp.z, temp.y/temp.z); }", v.vsOut());
                    }
                } else {
                    if (kVec2f_GrSLType == posVar.getType()) {
                        vb->codeAppendf("%s = %s * vec3(%s, 1);",
                                        v.vsOut(), uniName, posVar.c_str());
                    } else {
                        vb->codeAppendf("%s = %s * %s;", v.vsOut(), uniName, posVar.c_str());
                    }
                }
            } else {
                if (kVec2f_GrSLType == varyingType) {
                    vb->codeAppendf("%s = (%s * vec3(%s, 1)).xy;", v.vsOut(), uniName, localCoords);
                } else {
                    vb->codeAppendf("%s = %s * vec3(%s, 1);", v.vsOut(), uniName, localCoords);
                }
            }
        }
    }
}

void GrGLGeometryProcessor::emitTransforms(GrGLGPBuilder* pb,
                                           const char* localCoords,
                                           const TransformsIn& tin,
                                           TransformsOut* tout) {
    GrGLVertexBuilder* vb = pb->getVertexShaderBuilder();
    tout->push_back_n(tin.count());
    for (int i = 0; i < tin.count(); i++) {
        const ProcCoords& coordTransforms = tin[i];
        for (int t = 0; t < coordTransforms.count(); t++) {
            GrSLType varyingType = kVec2f_GrSLType;

            // Device coords aren't supported
            SkASSERT(kDevice_GrCoordSet != coordTransforms[t]->sourceCoords());
            GrSLPrecision precision = coordTransforms[t]->precision();

            SkString strVaryingName("MatrixCoord");
            strVaryingName.appendf("_%i_%i", i, t);

            GrGLVertToFrag v(varyingType);
            pb->addVarying(strVaryingName.c_str(), &v, precision);
            vb->codeAppendf("%s = %s;", v.vsOut(), localCoords);

            SkNEW_APPEND_TO_TARRAY(&(*tout)[i],
                                   GrGLProcessor::TransformedCoords,
                                   (SkString(v.fsIn()), varyingType));
        }
    }
}

void GrGLGeometryProcessor::setupPosition(GrGLGPBuilder* pb,
                                          GrGPArgs* gpArgs,
                                          const char* posName) {
    GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();
    gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
    vsBuilder->codeAppendf("vec2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
}

void GrGLGeometryProcessor::setupPosition(GrGLGPBuilder* pb,
                                          GrGPArgs* gpArgs,
                                          const char* posName,
                                          const SkMatrix& mat,
                                          UniformHandle* viewMatrixUniform) {
    GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();
    if (mat.isIdentity()) {
        gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
        vsBuilder->codeAppendf("vec2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
    } else {
        const char* viewMatrixName;
        *viewMatrixUniform = pb->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                            kMat33f_GrSLType, kHigh_GrSLPrecision,
                                            "uViewM",
                                            &viewMatrixName);
        if (!mat.hasPerspective()) {
            gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
            vsBuilder->codeAppendf("vec2 %s = vec2(%s * vec3(%s, 1));",
                                   gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        } else {
            gpArgs->fPositionVar.set(kVec3f_GrSLType, "pos3");
            vsBuilder->codeAppendf("vec3 %s = %s * vec3(%s, 1);",
                                   gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        }
    }
}
