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
    SkASSERT(localCoordsVar.getType() == kFloat2_GrSLType ||
             localCoordsVar.getType() == kFloat3_GrSLType ||
             localCoordsVar.getType() == kVoid_GrSLType /* until coord transforms are gone */);
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
        auto [coordTransform, fp] = handler->get();

        // FPs that use the legacy coord transform system will need a uniform registered for them
        // to hold the coord transform's matrix.
        GrShaderVar transformVar;
        // FPs that use local coordinates need a varying to convey the coordinate. This may be the
        // base GP's local coord if transforms have to be computed in the FS, or it may be a unique
        // varying that computes the equivalent transformation hierarchy in the VS.
        GrShaderVar varyingVar;

        // If this is true, the FP's signature takes a float2 local coordinate. Otherwise, it
        // doesn't use local coordinates, or it can be lifted to a varying and referenced directly.
        bool localCoordComputedInFS = fp.isSampledWithExplicitCoords();
        if (!coordTransform.isNoOp()) {
            // Legacy coord transform that actually is doing something. This matrix is the last
            // transformation to affect the local coordinate.
            SkString strUniName;
            strUniName.printf("CoordTransformMatrix_%d", i);
            auto flag = localCoordComputedInFS ? kFragment_GrShaderFlag
                                               : kVertex_GrShaderFlag;
            auto& uni = fInstalledTransforms.push_back();
            if (fp.isSampledWithExplicitCoords() && coordTransform.matrix().isScaleTranslate()) {
                uni.fType = kFloat4_GrSLType;
            } else {
                uni.fType = kFloat3x3_GrSLType;
            }
            uni.fHandle =
                    uniformHandler->addUniform(&fp, flag, uni.fType, strUniName.c_str());
            transformVar = uniformHandler->getUniformVariable(uni.fHandle);
        } else {
            // Must stay parallel with calls to handler
            fInstalledTransforms.push_back();
        }

        // If the FP references local coords, we need to make sure the vertex shader sets up the
        // right transforms or pass-through variables for the FP to evaluate in the fragment shader
        if (fp.referencesSampleCoords()) {
            if (localCoordComputedInFS) {
                // If the FP local coords are evaluated in the fragment shader, we only need to
                // produce the original local coordinate to pass into the root; any other situation,
                // the FP will have a 2nd parameter to its function and the caller sends the coords
                if (!fp.parent()) {
                    varyingVar = getBaseLocalCoord();
                }
            } else {
                // The FP's local coordinates are determined by the const/uniform transform
                // hierarchy from this FP to the root, and can be computed in the vertex shader.
                // If this hierarchy would be the identity transform, then we should use the
                // original local coordinate.
                // NOTE: The actual transform logic is handled in emitTransformCode(), this just
                // needs to determine if a unique varying should be added for the FP.
                GrShaderVar transformedLocalCoord;
                const GrFragmentProcessor* coordOwner = nullptr;

                const GrFragmentProcessor* node = &fp;
                while(node) {
                    SkASSERT(!node->isSampledWithExplicitCoords() &&
                             (node->sampleMatrix().isNoOp() ||
                              node->sampleMatrix().isConstUniform()));

                    if (node->sampleMatrix().isConstUniform()) {
                        // We can stop once we hit an FP that adds transforms; this FP can reuse
                        // that FPs varying (possibly vivifying it if this was the first use).
                        transformedLocalCoord = localCoordsMap[node];
                        coordOwner = node;
                        break;
                    } // else intervening FP is an identity transform so skip past it

                    node = node->parent();
                }

                // Legacy coord transform workaround (if the transform hierarchy appears identity
                // but we have GrCoordTransform that does something, we still need to record a
                // varying for it).
                if (!coordOwner && !coordTransform.isNoOp()) {
                    coordOwner = &fp;
                }

                if (coordOwner) {
                    // The FP will use coordOwner's varying; add varying if this was the first use
                    if (transformedLocalCoord.getType() == kVoid_GrSLType) {
                        GrGLSLVarying v(kFloat2_GrSLType);
                        if (coordTransform.matrix().hasPerspective() ||
                            GrSLTypeVecLength(localCoordsVar.getType()) == 3 ||
                            coordOwner->hasPerspectiveTransform()) {
                            v = GrGLSLVarying(kFloat3_GrSLType);
                        }
                        SkString strVaryingName;
                        strVaryingName.printf("TransformedCoords_%d", i);
                        varyingHandler->addVarying(strVaryingName.c_str(), &v);

                        fTransformInfos.push_back({GrShaderVar(v.vsOut(), v.type()),
                                                   transformVar.getName(),
                                                   localCoordsVar,
                                                   coordOwner});
                        transformedLocalCoord = GrShaderVar(SkString(v.fsIn()), v.type(),
                                                            GrShaderVar::TypeModifier::In);
                        if (coordOwner->numCoordTransforms() < 1 ||
                            coordOwner->coordTransform(0).isNoOp()) {
                            // As long as a legacy coord transform doesn't get in the way, we can
                            // reuse this expression for children (see comment in emitTransformCode)
                            localCoordsMap[coordOwner] = transformedLocalCoord;
                        }
                    }

                    varyingVar = transformedLocalCoord;
                } else {
                    // The FP transform hierarchy is the identity, so use the original local coord
                    varyingVar = getBaseLocalCoord();
                }
            }
        }

        if (varyingVar.getType() != kVoid_GrSLType || transformVar.getType() != kVoid_GrSLType) {
            handler->specifyCoordsForCurrCoordTransform(transformVar, varyingVar);
        } else {
            handler->omitCoordsForCurrCoordTransform();
        }
    }
}

void GrGLSLGeometryProcessor::emitTransformCode(GrGLSLVertexBuilder* vb,
                                                GrGLSLUniformHandler* uniformHandler) {
    std::unordered_map<const GrFragmentProcessor*, GrShaderVar> localCoordsMap;
    for (const auto& tr : fTransformInfos) {
        // If we recorded a transform info, its sample matrix must be const/uniform, or we have a
        // legacy coord transform that actually does something.
        SkASSERT(tr.fFP->sampleMatrix().isConstUniform() ||
                 (tr.fFP->sampleMatrix().isNoOp() && !tr.fMatrix.isEmpty()));

        SkString localCoords;
        // Build a concatenated matrix expression that we apply to the root local coord.
        // If we have an expression cached from an early FP in the hierarchy chain, we can stop
        // there instead of going all the way to the GP.
        SkString transformExpression;
        if (!tr.fMatrix.isEmpty()) {
            // We have both a const/uniform sample matrix and a legacy coord transform
            transformExpression.printf("%s", tr.fMatrix.c_str());
        }

        // If the sample matrix is kNone, then the current transform expression of just the
        // coord transform matrix is sufficient.
        if (tr.fFP->sampleMatrix().isConstUniform()) {
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
                } else if (base->sampleMatrix().isConstUniform()) {
                    // The FP knows the matrix expression it's sampled with, but its parent defined
                    // the uniform (when the expression is not a constant).
                    GrShaderVar uniform = uniformHandler->liftUniformToVertexShader(
                            *base->parent(), SkString(base->sampleMatrix().fExpression));

                    // Accumulate the base matrix expression as a preConcat
                    SkString matrix;
                    if (uniform.getType() != kVoid_GrSLType) {
                        SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
                        matrix = uniform.getName();
                    } else {
                        // No uniform found, so presumably this is a constant
                        matrix = SkString(base->sampleMatrix().fExpression);
                    }

                    if (!transformExpression.isEmpty()) {
                        transformExpression.append(" * ");
                    }
                    transformExpression.appendf("(%s)", matrix.c_str());
                } else {
                    // This intermediate FP is just a pass through and doesn't need to be built
                    // in to the expression, but must visit its parents in case they add transforms
                    SkASSERT(base->sampleMatrix().isNoOp());
                }

                base = base->parent();
            }
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

        if (tr.fMatrix.isEmpty()) {
            // Subtle work around: only cache the intermediate varying when there's no extra
            // coord transform. If the FP uses a coord transform for a legacy effect, but also
            // delegates to a child FP, we want the coordinates pre-GrCoordTransform to be sent
            // to the child FP, but have the FP use the post-coordtransform legacy values
            // (e.g. sampling a texture and relying on the GrCoordTransform for normalization
            //  and mixing with a child FP that should not be normalized).
            // FIXME: It's not really possible to apply this logic cleanly when transforms
            // have been moved to the FS; in practice this doesn't seem to occur in our tests and
            // the issue will go away once legacy coord transforms only have no-op matrices.
            localCoordsMap.insert({ tr.fFP, tr.fOutputCoords });
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
