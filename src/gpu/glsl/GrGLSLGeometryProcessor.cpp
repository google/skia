/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

#include <unordered_map>

void GrGLSLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);

    // FIXME This must always be called at the moment, even when fLocalCoordVar is uninitialized
    // and void because collectTransforms registers the uniforms for legacy coord transforms, which
    // still need to be added even if the FPs are sampled explicitly. When they are gone, we only
    // need to call this if the local coord isn't void (plus verify that FPs really don't need it).
    this->collectTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                            gpArgs.fLocalCoordVar, args.fFPCoordTransformHandler);

    if (args.fGP.willUseTessellationShaders()) {
        // Tessellation shaders are temporarily responsible for integrating their own code strings
        // while we work out full support.
        return;
    }

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
                vBuilder->codeAppend(", 0");
                [[fallthrough]];
            case kFloat2_GrSLType:
                vBuilder->codeAppend(", 0");
                [[fallthrough]];
            case kFloat3_GrSLType:
                vBuilder->codeAppend(", 1");
                [[fallthrough]];
            case kFloat4_GrSLType:
                vBuilder->codeAppend(");");
                break;
            default:
                SK_ABORT("Invalid position var type");
                break;
        }
    }
}

void GrGLSLGeometryProcessor::collectTransforms(GrGLSLVertexBuilder* vb,
                                                GrGLSLVaryingHandler* varyingHandler,
                                                GrGLSLUniformHandler* uniformHandler,
                                                const GrShaderVar& localCoordsVar,
                                                FPCoordTransformHandler* handler) {
    // We only require localCoordsVar to be valid if there is a coord transform that needs
    // it. CTs on FPs called with explicit coords do not require a local coord.
    auto getLocalCoords = [&localCoordsVar,
                           localCoords = SkString(),
                           localCoordLength = int()]() mutable {
        if (localCoords.isEmpty()) {
            localCoordLength = GrSLTypeVecLength(localCoordsVar.getType());
            SkASSERT(GrSLTypeIsFloatType(localCoordsVar.getType()));
            SkASSERT(localCoordLength == 2 || localCoordLength == 3);
            if (localCoordLength == 3) {
                localCoords = localCoordsVar.getName();
            } else {
                localCoords.printf("float3(%s, 1)", localCoordsVar.c_str());
            }
        }
        return std::make_tuple(localCoords, localCoordLength);
    };

    GrShaderVar transformVar;
    for (int i = 0; *handler; ++*handler, ++i) {
        auto [coordTransform, fp] = handler->get();
        // Add uniform for coord transform matrix.
        SkString matrix;
        if (!fp.isSampledWithExplicitCoords() || !coordTransform.isNoOp()) {
            SkString strUniName;
            strUniName.printf("CoordTransformMatrix_%d", i);
            auto flag = fp.isSampledWithExplicitCoords() ? kFragment_GrShaderFlag
                                                         : kVertex_GrShaderFlag;
            auto& uni = fInstalledTransforms.push_back();
            if (fp.isSampledWithExplicitCoords() && coordTransform.matrix().isScaleTranslate()) {
                uni.fType = kFloat4_GrSLType;
            } else {
                uni.fType = kFloat3x3_GrSLType;
            }
            const char* matrixName;
            uni.fHandle =
                    uniformHandler->addUniform(&fp, flag, uni.fType, strUniName.c_str(),
                                               &matrixName);
            matrix = matrixName;
            transformVar = uniformHandler->getUniformVariable(uni.fHandle);
        } else {
            // Install a coord transform that will be skipped.
            fInstalledTransforms.push_back();
            handler->omitCoordsForCurrCoordTransform();
            continue;
        }

        GrShaderVar fsVar;
        // Add varying if required and register varying and matrix uniform.
        if (!fp.isSampledWithExplicitCoords()) {
            auto [localCoordsStr, localCoordLength] = getLocalCoords();
            GrGLSLVarying v(kFloat2_GrSLType);
            if (coordTransform.matrix().hasPerspective() || localCoordLength == 3) {
                v = GrGLSLVarying(kFloat3_GrSLType);
            }
            SkString strVaryingName;
            strVaryingName.printf("TransformedCoords_%d", i);
            varyingHandler->addVarying(strVaryingName.c_str(), &v);

            SkASSERT(fInstalledTransforms.back().fType == kFloat3x3_GrSLType);
            if (fp.sampleMatrix().fKind != SkSL::SampleMatrix::Kind::kConstantOrUniform) {
                if (v.type() == kFloat2_GrSLType) {
                    vb->codeAppendf("%s = (%s * %s).xy;", v.vsOut(), matrix.c_str(),
                                    localCoordsStr.c_str());
                } else {
                    vb->codeAppendf("%s = %s * %s;", v.vsOut(), matrix.c_str(),
                                    localCoordsStr.c_str());
                }
            }
            fsVar = GrShaderVar(SkString(v.fsIn()), v.type(), GrShaderVar::TypeModifier::In);
            fTransformInfos.push_back({ v.vsOut(), v.type(), matrix, localCoordsStr, &fp });
        }
        handler->specifyCoordsForCurrCoordTransform(transformVar, fsVar);
    }
}

void GrGLSLGeometryProcessor::emitTransformCode(GrGLSLVertexBuilder* vb,
                                                GrGLSLUniformHandler* uniformHandler) {
    std::unordered_map<const GrFragmentProcessor*, const char*> localCoordsMap;
    for (const auto& tr : fTransformInfos) {
        switch (tr.fFP->sampleMatrix().fKind) {
            case SkSL::SampleMatrix::Kind::kConstantOrUniform: {
                SkString localCoords;
                localCoordsMap.insert({ tr.fFP, tr.fName });
                if (tr.fFP->sampleMatrix().fBase) {
                    SkASSERT(localCoordsMap[tr.fFP->sampleMatrix().fBase]);
                    localCoords = SkStringPrintf("float3(%s, 1)",
                                                 localCoordsMap[tr.fFP->sampleMatrix().fBase]);
                } else {
                    localCoords = tr.fLocalCoords.c_str();
                }
                vb->codeAppend("{\n");
                if (tr.fFP->sampleMatrix().fOwner) {
                    uniformHandler->writeUniformMappings(tr.fFP->sampleMatrix().fOwner, vb);
                }
                if (tr.fType == kFloat2_GrSLType) {
                    vb->codeAppendf("%s = (%s * %s * %s).xy", tr.fName,
                                    tr.fFP->sampleMatrix().fExpression.c_str(), tr.fMatrix.c_str(),
                                    localCoords.c_str());
                } else {
                    SkASSERT(tr.fType == kFloat3_GrSLType);
                    vb->codeAppendf("%s = %s * %s * %s", tr.fName,
                                    tr.fFP->sampleMatrix().fExpression.c_str(), tr.fMatrix.c_str(),
                                    localCoords.c_str());
                }
                vb->codeAppend(";\n");
                vb->codeAppend("}\n");
                break;
            }
            default:
                break;
        }
    }
}

void GrGLSLGeometryProcessor::setTransformDataHelper(const GrGLSLProgramDataManager& pdman,
                                                     const CoordTransformRange& transformRange) {
    int i = 0;
    for (auto [transform, fp] : transformRange) {
        if (fInstalledTransforms[i].fHandle.isValid()) {
            SkMatrix m = GetTransformMatrix(transform, SkMatrix::I());
            if (!SkMatrixPriv::CheapEqual(fInstalledTransforms[i].fCurrentValue, m)) {
                if (fInstalledTransforms[i].fType == kFloat4_GrSLType) {
                    float values[4] = {m.getScaleX(), m.getTranslateX(),
                                       m.getScaleY(), m.getTranslateY()};
                    SkASSERT(m.isScaleTranslate());
                    pdman.set4fv(fInstalledTransforms[i].fHandle.toIndex(), 1, values);
                } else {
                    SkASSERT(!m.isScaleTranslate() || !fp.isSampledWithExplicitCoords());
                    SkASSERT(fInstalledTransforms[i].fType == kFloat3x3_GrSLType);
                    pdman.setSkMatrix(fInstalledTransforms[i].fHandle.toIndex(), m);
                }
                fInstalledTransforms[i].fCurrentValue = m;
            }
        }
        ++i;
    }
    SkASSERT(i == fInstalledTransforms.count());
}

void GrGLSLGeometryProcessor::setTransform(const GrGLSLProgramDataManager& pdman,
                                           const UniformHandle& uniform,
                                           const SkMatrix& matrix,
                                           SkMatrix* state) const {
    if (!uniform.isValid() || (state && SkMatrixPriv::CheapEqual(*state, matrix))) {
        // No update needed
        return;
    }
    if (state) {
        *state = matrix;
    }
    if (matrix.isScaleTranslate()) {
        // ComputeMatrixKey and writeX() assume the uniform is a float4 (can't assert since nothing
        // is exposed on a handle, but should be caught lower down).
        float values[4] = {matrix.getScaleX(), matrix.getTranslateX(),
                           matrix.getScaleY(), matrix.getTranslateY()};
        pdman.set4fv(uniform, 1, values);
    } else {
        pdman.setSkMatrix(uniform, matrix);
    }
}

static void write_vertex_position(GrGLSLVertexBuilder* vertBuilder,
                                  GrGLSLUniformHandler* uniformHandler,
                                  const GrShaderVar& inPos,
                                  const SkMatrix& matrix,
                                  const char* matrixName,
                                  GrShaderVar* outPos,
                                  GrGLSLGeometryProcessor::UniformHandle* matrixUniform) {
    SkASSERT(inPos.getType() == kFloat3_GrSLType || inPos.getType() == kFloat2_GrSLType);
    SkString outName = vertBuilder->newTmpVarName(inPos.getName().c_str());

    if (matrix.isIdentity()) {
        // Direct assignment, we won't use a uniform for the matrix.
        outPos->set(inPos.getType(), outName.c_str());
        vertBuilder->codeAppendf("float%d %s = %s;", GrSLTypeVecLength(inPos.getType()),
                                                     outName.c_str(), inPos.getName().c_str());
    } else {
        SkASSERT(matrixUniform);

        bool useCompactTransform = matrix.isScaleTranslate();
        const char* mangledMatrixName;
        *matrixUniform = uniformHandler->addUniform(nullptr,
                                                        kVertex_GrShaderFlag,
                                                        useCompactTransform ? kFloat4_GrSLType
                                                                            : kFloat3x3_GrSLType,
                                                        matrixName,
                                                        &mangledMatrixName);

        if (inPos.getType() == kFloat3_GrSLType) {
            // A float3 stays a float3 whether or not the matrix adds perspective
            if (useCompactTransform) {
                vertBuilder->codeAppendf("float3 %s = %s.xz1 * %s + %s.yw0;\n",
                                         outName.c_str(), mangledMatrixName,
                                         inPos.getName().c_str(), mangledMatrixName);
            } else {
                vertBuilder->codeAppendf("float3 %s = %s * %s;\n", outName.c_str(),
                                         mangledMatrixName, inPos.getName().c_str());
            }
            outPos->set(kFloat3_GrSLType, outName.c_str());
        } else if (matrix.hasPerspective()) {
            // A float2 is promoted to a float3 if we add perspective via the matrix
            SkASSERT(!useCompactTransform);
            vertBuilder->codeAppendf("float3 %s = (%s * %s.xy1);",
                                     outName.c_str(), mangledMatrixName, inPos.getName().c_str());
            outPos->set(kFloat3_GrSLType, outName.c_str());
        } else {
            if (useCompactTransform) {
                vertBuilder->codeAppendf("float2 %s = %s.xz * %s + %s.yw;\n",
                                         outName.c_str(), mangledMatrixName,
                                         inPos.getName().c_str(), mangledMatrixName);
            } else {
                vertBuilder->codeAppendf("float2 %s = (%s * %s.xy1).xy;\n",
                                         outName.c_str(), mangledMatrixName,
                                         inPos.getName().c_str());
            }
            outPos->set(kFloat2_GrSLType, outName.c_str());
        }
    }
}

void GrGLSLGeometryProcessor::writeOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                                  GrGPArgs* gpArgs,
                                                  const char* posName) {
    // writeOutputPosition assumes the incoming pos name points to a float2 variable
    GrShaderVar inPos(posName, kFloat2_GrSLType);
    write_vertex_position(vertBuilder, nullptr, inPos, SkMatrix::I(), "viewMatrix",
                          &gpArgs->fPositionVar, nullptr);
}

void GrGLSLGeometryProcessor::writeOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                                  GrGLSLUniformHandler* uniformHandler,
                                                  GrGPArgs* gpArgs,
                                                  const char* posName,
                                                  const SkMatrix& mat,
                                                  UniformHandle* viewMatrixUniform) {
    GrShaderVar inPos(posName, kFloat2_GrSLType);
    write_vertex_position(vertBuilder, uniformHandler, inPos, mat, "viewMatrix",
                          &gpArgs->fPositionVar, viewMatrixUniform);
}

void GrGLSLGeometryProcessor::writeLocalCoord(GrGLSLVertexBuilder* vertBuilder,
                                              GrGLSLUniformHandler* uniformHandler,
                                              GrGPArgs* gpArgs,
                                              GrShaderVar localVar,
                                              const SkMatrix& localMatrix,
                                              UniformHandle* localMatrixUniform) {
    write_vertex_position(vertBuilder, uniformHandler, localVar, localMatrix, "localMatrix",
                          &gpArgs->fLocalCoordVar, localMatrixUniform);
}
