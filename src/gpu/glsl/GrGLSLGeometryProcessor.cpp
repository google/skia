/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLGeometryProcessor.h"

#include "GrCoordTransform.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProcessorTypes.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

void GrGLSLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGLSLVertexBuilder* vBuilder = args.fVertBuilder;
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);
    vBuilder->transformToNormalizedDeviceSpace(gpArgs.fPositionVar);
    if (kVec2f_GrSLType == gpArgs.fPositionVar.getType()) {
        args.fVaryingHandler->setNoPerspective();
    }
}

void GrGLSLGeometryProcessor::emitTransforms(GrGLSLVertexBuilder* vb,
                                             GrGLSLVaryingHandler* varyingHandler,
                                             GrGLSLUniformHandler* uniformHandler,
                                             const GrShaderVar& posVar,
                                             const char* localCoords,
                                             const SkMatrix& localMatrix,
                                             const TransformsIn& tin,
                                             TransformsOut* tout) {
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
                    uniformHandler->addUniform(kVertex_GrShaderFlag,
                                               kMat33f_GrSLType, precision,
                                               strUniName.c_str(),
                                               &uniName).toIndex();

            SkString strVaryingName("MatrixCoord");
            strVaryingName.appendf("_%i_%i", i, t);

            GrGLSLVertToFrag v(varyingType);
            varyingHandler->addVarying(strVaryingName.c_str(), &v, precision);

            SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
            (*tout)[i].emplace_back(SkString(v.fsIn()), varyingType);

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

void GrGLSLGeometryProcessor::emitTransforms(GrGLSLVertexBuilder* vb,
                                             GrGLSLVaryingHandler* varyingHandler,
                                             const char* localCoords,
                                             const TransformsIn& tin,
                                             TransformsOut* tout) {
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

            GrGLSLVertToFrag v(varyingType);
            varyingHandler->addVarying(strVaryingName.c_str(), &v, precision);
            vb->codeAppendf("%s = %s;", v.vsOut(), localCoords);

            (*tout)[i].emplace_back(SkString(v.fsIn()), varyingType);
        }
    }
}

void GrGLSLGeometryProcessor::setupPosition(GrGLSLVertexBuilder* vertBuilder,
                                            GrGPArgs* gpArgs,
                                            const char* posName) {
    gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
    vertBuilder->codeAppendf("vec2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
}

void GrGLSLGeometryProcessor::setupPosition(GrGLSLVertexBuilder* vertBuilder,
                                            GrGLSLUniformHandler* uniformHandler,
                                            GrGPArgs* gpArgs,
                                            const char* posName,
                                            const SkMatrix& mat,
                                            UniformHandle* viewMatrixUniform) {
    if (mat.isIdentity()) {
        gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
        vertBuilder->codeAppendf("vec2 %s = %s;", gpArgs->fPositionVar.c_str(), posName);
    } else {
        const char* viewMatrixName;
        *viewMatrixUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                        kMat33f_GrSLType, kHigh_GrSLPrecision,
                                                        "uViewM",
                                                        &viewMatrixName);
        if (!mat.hasPerspective()) {
            gpArgs->fPositionVar.set(kVec2f_GrSLType, "pos2");
            vertBuilder->codeAppendf("vec2 %s = vec2(%s * vec3(%s, 1));",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        } else {
            gpArgs->fPositionVar.set(kVec3f_GrSLType, "pos3");
            vertBuilder->codeAppendf("vec3 %s = %s * vec3(%s, 1);",
                                     gpArgs->fPositionVar.c_str(), viewMatrixName, posName);
        }
    }
}
