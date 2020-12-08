/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"

#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

#include <unordered_map>

void GrGLSLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);

    if (gpArgs.fLocalCoordVar.getType() != kVoid_GrSLType) {
        this->collectTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                                gpArgs.fLocalCoordVar, args.fFPCoordTransformHandler);
    } else {
        // Make sure no FPs needed the local coord variable.
        SkASSERT(!*args.fFPCoordTransformHandler);
    }

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
        vBuilder->emitNormalizedSkPosition(gpArgs.fPositionVar.c_str(),
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
    SkASSERT(localCoordsVar.getType() == kFloat2_GrSLType ||
             localCoordsVar.getType() == kFloat3_GrSLType);
    // Cached varyings produced by parent FPs. If parent FPs introduce transformations, but all
    // subsequent children are not transformed, they should share the same varying.
    std::unordered_map<const GrFragmentProcessor*, GrShaderVar> localCoordsMap;

    GrGLSLVarying baseLocalCoord;
    auto getBaseLocalCoord = [&baseLocalCoord, &localCoordsVar, vb, varyingHandler]() {
        SkASSERT(GrSLTypeIsFloatType(localCoordsVar.getType()));
        if (baseLocalCoord.type() == kVoid_GrSLType) {
            // Initialize to the GP provided coordinate
            SkString baseLocalCoordName = SkStringPrintf("LocalCoord");
            baseLocalCoord = GrGLSLVarying(localCoordsVar.getType());
            varyingHandler->addVarying(baseLocalCoordName.c_str(), &baseLocalCoord);
            vb->codeAppendf("%s = %s;\n", baseLocalCoord.vsOut(),
                            localCoordsVar.getName().c_str());
        }
        return GrShaderVar(SkString(baseLocalCoord.fsIn()), baseLocalCoord.type(),
                           GrShaderVar::TypeModifier::In);
    };

    for (int i = 0; *handler; ++*handler, ++i) {
        const auto& fp = handler->get();

        SkASSERT(fp.referencesSampleCoords());
        SkASSERT(!fp.isSampledWithExplicitCoords());

        // FPs that use local coordinates need a varying to convey the coordinate. This may be the
        // base GP's local coord if transforms have to be computed in the FS, or it may be a unique
        // varying that computes the equivalent transformation hierarchy in the VS.
        GrShaderVar varyingVar;

        // The FP's local coordinates are determined by the uniform transform hierarchy
        // from this FP to the root, and can be computed in the vertex shader.
        // If this hierarchy would be the identity transform, then we should use the
        // original local coordinate.
        // NOTE: The actual transform logic is handled in emitTransformCode(), this just
        // needs to determine if a unique varying should be added for the FP.
        GrShaderVar transformedLocalCoord;
        const GrFragmentProcessor* coordOwner = nullptr;

        const GrFragmentProcessor* node = &fp;
        while(node) {
            SkASSERT(!node->isSampledWithExplicitCoords() &&
                     !node->sampleUsage().hasVariableMatrix());

            if (node->sampleUsage().hasUniformMatrix()) {
                // We can stop once we hit an FP that adds transforms; this FP can reuse
                // that FPs varying (possibly vivifying it if this was the first use).
                transformedLocalCoord = localCoordsMap[node];
                coordOwner = node;
                break;
            } // else intervening FP is an identity transform so skip past it

            node = node->parent();
        }

        if (coordOwner) {
            // The FP will use coordOwner's varying; add varying if this was the first use
            if (transformedLocalCoord.getType() == kVoid_GrSLType) {
                GrGLSLVarying v(kFloat2_GrSLType);
                if (GrSLTypeVecLength(localCoordsVar.getType()) == 3 ||
                    coordOwner->hasPerspectiveTransform()) {
                    v = GrGLSLVarying(kFloat3_GrSLType);
                }
                SkString strVaryingName;
                strVaryingName.printf("TransformedCoords_%d", i);
                varyingHandler->addVarying(strVaryingName.c_str(), &v);

                fTransformInfos.push_back(
                        {GrShaderVar(v.vsOut(), v.type()), localCoordsVar, coordOwner});
                transformedLocalCoord =
                        GrShaderVar(SkString(v.fsIn()), v.type(), GrShaderVar::TypeModifier::In);
                localCoordsMap[coordOwner] = transformedLocalCoord;
            }

            varyingVar = transformedLocalCoord;
        } else {
            // The FP transform hierarchy is the identity, so use the original local coord
            varyingVar = getBaseLocalCoord();
        }

        SkASSERT(varyingVar.getType() != kVoid_GrSLType);
        handler->specifyCoordsForCurrCoordTransform(varyingVar);
    }
}

void GrGLSLGeometryProcessor::emitTransformCode(GrGLSLVertexBuilder* vb,
                                                GrGLSLUniformHandler* uniformHandler) {
    std::unordered_map<const GrFragmentProcessor*, GrShaderVar> localCoordsMap;
    for (const auto& tr : fTransformInfos) {
        // If we recorded a transform info, its sample matrix must be uniform
        SkASSERT(tr.fFP->sampleUsage().hasUniformMatrix());

        SkString localCoords;
        // Build a concatenated matrix expression that we apply to the root local coord.
        // If we have an expression cached from an early FP in the hierarchy chain, we can stop
        // there instead of going all the way to the GP.
        SkString transformExpression;

        const auto* base = tr.fFP;
        while(base) {
            GrShaderVar cachedBaseCoord = localCoordsMap[base];
            if (cachedBaseCoord.getType() != kVoid_GrSLType) {
                // Can stop here, as this varying already holds all transforms from higher FPs
                if (cachedBaseCoord.getType() == kFloat3_GrSLType) {
                    localCoords = cachedBaseCoord.getName();
                } else {
                    localCoords = SkStringPrintf("%s.xy1", cachedBaseCoord.getName().c_str());
                }
                break;
            } else if (base->sampleUsage().hasUniformMatrix()) {
                // The FP knows the matrix expression it's sampled with, but its parent defined
                // the uniform (when the expression is not a constant).
                GrShaderVar uniform = uniformHandler->liftUniformToVertexShader(
                        *base->parent(), SkString(base->sampleUsage().fExpression));

                // Accumulate the base matrix expression as a preConcat
                SkString matrix;
                if (uniform.getType() != kVoid_GrSLType) {
                    SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
                    matrix = uniform.getName();
                } else {
                    // No uniform found, so presumably this is a constant
                    matrix = SkString(base->sampleUsage().fExpression);
                }

                if (!transformExpression.isEmpty()) {
                    transformExpression.append(" * ");
                }
                transformExpression.appendf("(%s)", matrix.c_str());
            } else {
                // This intermediate FP is just a pass through and doesn't need to be built
                // in to the expression, but must visit its parents in case they add transforms
                SkASSERT(!base->sampleUsage().hasMatrix() && !base->sampleUsage().fExplicitCoords);
            }

            base = base->parent();
        }

        if (localCoords.isEmpty()) {
            // Must use GP's local coords
            if (tr.fLocalCoords.getType() == kFloat3_GrSLType) {
                localCoords = tr.fLocalCoords.getName();
            } else {
                localCoords = SkStringPrintf("%s.xy1", tr.fLocalCoords.getName().c_str());
            }
        }

        vb->codeAppend("{\n");
        if (tr.fOutputCoords.getType() == kFloat2_GrSLType) {
            vb->codeAppendf("%s = ((%s) * %s).xy", tr.fOutputCoords.getName().c_str(),
                                                   transformExpression.c_str(),
                                                   localCoords.c_str());
        } else {
            SkASSERT(tr.fOutputCoords.getType() == kFloat3_GrSLType);
            vb->codeAppendf("%s = (%s) * %s", tr.fOutputCoords.getName().c_str(),
                                              transformExpression.c_str(),
                                              localCoords.c_str());
        }
        vb->codeAppend(";\n");
        vb->codeAppend("}\n");

        localCoordsMap.insert({ tr.fFP, tr.fOutputCoords });
    }
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
