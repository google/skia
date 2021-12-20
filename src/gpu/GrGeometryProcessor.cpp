/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGeometryProcessor.h"

#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

#include <queue>

GrGeometryProcessor::GrGeometryProcessor(ClassID classID) : GrProcessor(classID) {}

const GrGeometryProcessor::TextureSampler& GrGeometryProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t GrGeometryProcessor::ComputeCoordTransformsKey(const GrFragmentProcessor& fp) {
    // This is highly coupled with the code in ProgramImpl::collectTransforms().
    uint32_t key = static_cast<uint32_t>(fp.sampleUsage().kind()) << 1;
    // This needs to be updated if GP starts specializing varyings on additional matrix types.
    if (fp.sampleUsage().hasPerspective()) {
        key |= 0b1;
    }
    return key;
}

void GrGeometryProcessor::getAttributeKey(skgpu::KeyBuilder* b) const {
    b->appendComment("vertex attributes");
    fVertexAttributes.addToKey(b);
    b->appendComment("instance attributes");
    fInstanceAttributes.addToKey(b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline GrSamplerState::Filter clamp_filter(GrTextureType type,
                                                  GrSamplerState::Filter requestedFilter) {
    if (GrTextureTypeHasRestrictedSampling(type)) {
        return std::min(requestedFilter, GrSamplerState::Filter::kLinear);
    }
    return requestedFilter;
}

GrGeometryProcessor::TextureSampler::TextureSampler(GrSamplerState samplerState,
                                                    const GrBackendFormat& backendFormat,
                                                    const GrSwizzle& swizzle) {
    this->reset(samplerState, backendFormat, swizzle);
}

void GrGeometryProcessor::TextureSampler::reset(GrSamplerState samplerState,
                                                const GrBackendFormat& backendFormat,
                                                const GrSwizzle& swizzle) {
    fSamplerState = samplerState;
    fSamplerState.setFilterMode(clamp_filter(backendFormat.textureType(), samplerState.filter()));
    fBackendFormat = backendFormat;
    fSwizzle = swizzle;
    fIsInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////

using ProgramImpl = GrGeometryProcessor::ProgramImpl;

std::tuple<ProgramImpl::FPCoordsMap, GrShaderVar>
ProgramImpl::emitCode(EmitArgs& args, const GrPipeline& pipeline) {
    GrGPArgs gpArgs;
    this->onEmitCode(args, &gpArgs);

    GrShaderVar positionVar = gpArgs.fPositionVar;
    // skia:12198
    if (args.fGeomProc.willUseTessellationShaders()) {
        positionVar = {};
    }
    FPCoordsMap transformMap = this->collectTransforms(args.fVertBuilder,
                                                       args.fVaryingHandler,
                                                       args.fUniformHandler,
                                                       gpArgs.fLocalCoordShader,
                                                       gpArgs.fLocalCoordVar,
                                                       positionVar,
                                                       pipeline);

    // Tessellation shaders are temporarily responsible for integrating their own code strings
    // while we work out full support.
    if (!args.fGeomProc.willUseTessellationShaders()) {
        GrGLSLVertexBuilder* vBuilder = args.fVertBuilder;
        // Emit the vertex position to the hardware in the normalized window coordinates it expects.
        SkASSERT(kFloat2_GrSLType == gpArgs.fPositionVar.getType() ||
                 kFloat3_GrSLType == gpArgs.fPositionVar.getType());
        vBuilder->emitNormalizedSkPosition(gpArgs.fPositionVar.c_str(),
                                           gpArgs.fPositionVar.getType());
        if (kFloat2_GrSLType == gpArgs.fPositionVar.getType()) {
            args.fVaryingHandler->setNoPerspective();
        }
    }
    return {transformMap, gpArgs.fLocalCoordVar};
}

ProgramImpl::FPCoordsMap ProgramImpl::collectTransforms(GrGLSLVertexBuilder* vb,
                                                        GrGLSLVaryingHandler* varyingHandler,
                                                        GrGLSLUniformHandler* uniformHandler,
                                                        GrShaderType localCoordsShader,
                                                        const GrShaderVar& localCoordsVar,
                                                        const GrShaderVar& positionVar,
                                                        const GrPipeline& pipeline) {
    SkASSERT(localCoordsVar.getType() == kFloat2_GrSLType ||
             localCoordsVar.getType() == kFloat3_GrSLType ||
             localCoordsVar.getType() == kVoid_GrSLType);
    SkASSERT(positionVar.getType() == kFloat2_GrSLType ||
             positionVar.getType() == kFloat3_GrSLType ||
             positionVar.getType() == kVoid_GrSLType);

    enum class BaseCoord { kNone, kLocal, kPosition };

    auto baseLocalCoordFSVar = [&, baseLocalCoordVarying = GrGLSLVarying()]() mutable {
        if (localCoordsShader == kFragment_GrShaderType) {
            return localCoordsVar;
        }
        SkASSERT(localCoordsShader == kVertex_GrShaderType);
        SkASSERT(GrSLTypeIsFloatType(localCoordsVar.getType()));
        if (baseLocalCoordVarying.type() == kVoid_GrSLType) {
            // Initialize to the GP provided coordinate
            baseLocalCoordVarying = GrGLSLVarying(localCoordsVar.getType());
            varyingHandler->addVarying("LocalCoord", &baseLocalCoordVarying);
            vb->codeAppendf("%s = %s;\n",
                            baseLocalCoordVarying.vsOut(),
                            localCoordsVar.getName().c_str());
        }
        return baseLocalCoordVarying.fsInVar();
    };

    bool canUsePosition = positionVar.getType() != kVoid_GrSLType;

    FPCoordsMap result;
    // Performs a pre-order traversal of FP hierarchy rooted at fp and identifies FPs that are
    // sampled with a series of matrices applied to local coords. For each such FP a varying is
    // added to the varying handler and added to 'result'.
    auto liftTransforms = [&, traversalIndex = 0](
                                  auto& self,
                                  const GrFragmentProcessor& fp,
                                  bool hasPerspective,
                                  const GrFragmentProcessor* lastMatrixFP = nullptr,
                                  int lastMatrixTraversalIndex = -1,
                                  BaseCoord baseCoord = BaseCoord::kLocal) mutable -> void {
        ++traversalIndex;
        if (localCoordsShader == kVertex_GrShaderType) {
            switch (fp.sampleUsage().kind()) {
                case SkSL::SampleUsage::Kind::kNone:
                    // This should only happen at the root. Otherwise how did this FP get added?
                    SkASSERT(!fp.parent());
                    break;
                case SkSL::SampleUsage::Kind::kPassThrough:
                    break;
                case SkSL::SampleUsage::Kind::kUniformMatrix:
                    // Update tracking of last matrix and matrix props.
                    hasPerspective |= fp.sampleUsage().hasPerspective();
                    lastMatrixFP = &fp;
                    lastMatrixTraversalIndex = traversalIndex;
                    break;
                case SkSL::SampleUsage::Kind::kFragCoord:
                    hasPerspective = positionVar.getType() == kFloat3_GrSLType;
                    lastMatrixFP = nullptr;
                    lastMatrixTraversalIndex = -1;
                    baseCoord = BaseCoord::kPosition;
                    break;
                case SkSL::SampleUsage::Kind::kExplicit:
                    baseCoord = BaseCoord::kNone;
                    break;
            }
        } else {
            // If the GP doesn't provide an interpolatable local coord then there is no hope to
            // lift.
            baseCoord = BaseCoord::kNone;
        }

        auto& [varyingFSVar, hasCoordsParam] = result[&fp];
        hasCoordsParam = fp.usesSampleCoordsDirectly();

        // We add a varying if we're in a chain of matrices multiplied by local or device coords.
        // If the coord is the untransformed local coord we add a varying. We don't if it is
        // untransformed device coords since it doesn't save us anything over "sk_FragCoord.xy". Of
        // course, if the FP doesn't directly use its coords then we don't add a varying.
        if (fp.usesSampleCoordsDirectly() &&
            (baseCoord == BaseCoord::kLocal ||
             (baseCoord == BaseCoord::kPosition && lastMatrixFP && canUsePosition))) {
            // Associate the varying with the highest possible node in the FP tree that shares the
            // same coordinates so that multiple FPs in a subtree can share. If there are no matrix
            // sample nodes on the way up the tree then directly use the local coord.
            if (!lastMatrixFP) {
                varyingFSVar = baseLocalCoordFSVar();
            } else {
                // If there is an already a varying that incorporates all matrices from the root to
                // lastMatrixFP just use it. Otherwise, we add it.
                auto& [varying, inputCoords, varyingIdx] = fTransformVaryingsMap[lastMatrixFP];
                if (varying.type() == kVoid_GrSLType) {
                    varying = GrGLSLVarying(hasPerspective ? kFloat3_GrSLType : kFloat2_GrSLType);
                    SkString strVaryingName = SkStringPrintf("TransformedCoords_%d",
                                                             lastMatrixTraversalIndex);
                    varyingHandler->addVarying(strVaryingName.c_str(), &varying);
                    inputCoords = baseCoord == BaseCoord::kLocal ? localCoordsVar : positionVar;
                    varyingIdx = lastMatrixTraversalIndex;
                }
                SkASSERT(varyingIdx == lastMatrixTraversalIndex);
                // The FP will use the varying in the fragment shader as its coords.
                varyingFSVar = varying.fsInVar();
            }
            hasCoordsParam = false;
        }

        for (int c = 0; c < fp.numChildProcessors(); ++c) {
            if (auto* child = fp.childProcessor(c)) {
                self(self,
                     *child,
                     hasPerspective,
                     lastMatrixFP,
                     lastMatrixTraversalIndex,
                     baseCoord);
                // If we have a varying then we never need a param. Otherwise, if one of our
                // children takes a non-explicit coord then we'll need our coord.
                hasCoordsParam |= varyingFSVar.getType() == kVoid_GrSLType &&
                                  !child->sampleUsage().isExplicit()       &&
                                  !child->sampleUsage().isFragCoord()      &&
                                  result[child].hasCoordsParam;
            }
        }
    };

    bool hasPerspective = GrSLTypeVecLength(localCoordsVar.getType()) == 3;
    for (int i = 0; i < pipeline.numFragmentProcessors(); ++i) {
        liftTransforms(liftTransforms, pipeline.getFragmentProcessor(i), hasPerspective);
    }
    return result;
}

void ProgramImpl::emitTransformCode(GrGLSLVertexBuilder* vb, GrGLSLUniformHandler* uniformHandler) {
    // Because descendant varyings may be computed using the varyings of ancestor FPs we make
    // sure to visit the varyings according to FP pre-order traversal by dumping them into a
    // priority queue.
    using FPAndInfo = std::tuple<const GrFragmentProcessor*, TransformInfo>;
    auto compare = [](const FPAndInfo& a, const FPAndInfo& b) {
        return std::get<1>(a).traversalOrder > std::get<1>(b).traversalOrder;
    };
    std::priority_queue<FPAndInfo, std::vector<FPAndInfo>, decltype(compare)> pq(compare);
    std::for_each(fTransformVaryingsMap.begin(), fTransformVaryingsMap.end(), [&pq](auto entry) {
        pq.push(entry);
    });
    for (; !pq.empty(); pq.pop()) {
        const auto& [fp, info] = pq.top();
        // If we recorded a transform info, its sample matrix must be uniform
        SkASSERT(fp->sampleUsage().isUniformMatrix());
        GrShaderVar uniform = uniformHandler->liftUniformToVertexShader(
                *fp->parent(), SkString(SkSL::SampleUsage::MatrixUniformName()));
        // Start with this matrix and accumulate additional matrices as we walk up the FP tree
        // to either the base coords or an ancestor FP that has an associated varying.
        SkString transformExpression = uniform.getName();

        // If we hit an ancestor with a varying on our walk up then save off the varying as the
        // input to our accumulated transformExpression. Start off assuming we'll reach the root.
        GrShaderVar inputCoords = info.inputCoords;

        for (const auto* base = fp->parent(); base; base = base->parent()) {
            if (auto iter = fTransformVaryingsMap.find(base); iter != fTransformVaryingsMap.end()) {
                // Can stop here, as this varying already holds all transforms from higher FPs
                // We'll apply the residual transformExpression we've accumulated up from our
                // starting FP to this varying.
                inputCoords = iter->second.varying.vsOutVar();
                break;
            } else if (base->sampleUsage().isUniformMatrix()) {
                // Accumulate any matrices along the path to either the original local/device coords
                // or a parent varying. Getting here means this FP was sampled with a uniform matrix
                // but all uses of coords below here in the FP hierarchy are beneath additional
                // matrix samples and thus this node wasn't assigned a varying.
                GrShaderVar parentUniform = uniformHandler->liftUniformToVertexShader(
                        *base->parent(), SkString(SkSL::SampleUsage::MatrixUniformName()));
                transformExpression.appendf(" * %s", parentUniform.getName().c_str());
            } else if (base->sampleUsage().isFragCoord()) {
                // Our chain of matrices starts here and is based on the device space position.
                break;
            } else {
                // This intermediate FP is just a pass through and doesn't need to be built
                // in to the expression, but we must visit its parents in case they add transforms.
                SkASSERT(base->sampleUsage().isPassThrough() || !base->sampleUsage().isSampled());
            }
        }

        SkString inputStr;
        if (inputCoords.getType() == kFloat2_GrSLType) {
            inputStr = SkStringPrintf("%s.xy1", inputCoords.getName().c_str());
        } else {
            SkASSERT(inputCoords.getType() == kFloat3_GrSLType);
            inputStr = inputCoords.getName();
        }

        vb->codeAppend("{\n");
        if (info.varying.type() == kFloat2_GrSLType) {
            if (vb->getProgramBuilder()->shaderCaps()->nonsquareMatrixSupport()) {
                vb->codeAppendf("%s = float3x2(%s) * %s",
                                info.varying.vsOut(),
                                transformExpression.c_str(),
                                inputStr.c_str());
            } else {
                vb->codeAppendf("%s = (%s * %s).xy",
                                info.varying.vsOut(),
                                transformExpression.c_str(),
                                inputStr.c_str());
            }
        } else {
            SkASSERT(info.varying.type() == kFloat3_GrSLType);
            vb->codeAppendf("%s = %s * %s",
                            info.varying.vsOut(),
                            transformExpression.c_str(),
                            inputStr.c_str());
        }
        vb->codeAppend(";\n");
        vb->codeAppend("}\n");
    }
    // We don't need this map anymore.
    fTransformVaryingsMap.clear();
}

void ProgramImpl::setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                    GrGLSLUniformHandler* uniformHandler,
                                    const char* outputName,
                                    UniformHandle* colorUniform) {
    SkASSERT(colorUniform);
    const char* stagedLocalVarName;
    *colorUniform = uniformHandler->addUniform(nullptr,
                                               kFragment_GrShaderFlag,
                                               kHalf4_GrSLType,
                                               "Color",
                                               &stagedLocalVarName);
    fragBuilder->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
    if (fragBuilder->getProgramBuilder()->shaderCaps()->mustObfuscateUniformColor()) {
        fragBuilder->codeAppendf("%s = max(%s, half4(0));", outputName, outputName);
    }
}

void ProgramImpl::SetTransform(const GrGLSLProgramDataManager& pdman,
                               const GrShaderCaps& shaderCaps,
                               const UniformHandle& uniform,
                               const SkMatrix& matrix,
                               SkMatrix* state) {
    if (!uniform.isValid() || (state && SkMatrixPriv::CheapEqual(*state, matrix))) {
        // No update needed
        return;
    }
    if (state) {
        *state = matrix;
    }
    if (matrix.isScaleTranslate() && !shaderCaps.reducedShaderMode()) {
        // ComputeMatrixKey and writeX() assume the uniform is a float4 (can't assert since nothing
        // is exposed on a handle, but should be caught lower down).
        float values[4] = {matrix.getScaleX(), matrix.getTranslateX(),
                           matrix.getScaleY(), matrix.getTranslateY()};
        pdman.set4fv(uniform, 1, values);
    } else {
        pdman.setSkMatrix(uniform, matrix);
    }
}

static void write_passthrough_vertex_position(GrGLSLVertexBuilder* vertBuilder,
                                              const GrShaderVar& inPos,
                                              GrShaderVar* outPos) {
    SkASSERT(inPos.getType() == kFloat3_GrSLType || inPos.getType() == kFloat2_GrSLType);
    SkString outName = vertBuilder->newTmpVarName(inPos.getName().c_str());
    outPos->set(inPos.getType(), outName.c_str());
    vertBuilder->codeAppendf("float%d %s = %s;",
                             GrSLTypeVecLength(inPos.getType()),
                             outName.c_str(),
                             inPos.getName().c_str());
}

static void write_vertex_position(GrGLSLVertexBuilder* vertBuilder,
                                  GrGLSLUniformHandler* uniformHandler,
                                  const GrShaderCaps& shaderCaps,
                                  const GrShaderVar& inPos,
                                  const SkMatrix& matrix,
                                  const char* matrixName,
                                  GrShaderVar* outPos,
                                  ProgramImpl::UniformHandle* matrixUniform) {
    SkASSERT(inPos.getType() == kFloat3_GrSLType || inPos.getType() == kFloat2_GrSLType);
    SkString outName = vertBuilder->newTmpVarName(inPos.getName().c_str());

    if (matrix.isIdentity() && !shaderCaps.reducedShaderMode()) {
        write_passthrough_vertex_position(vertBuilder, inPos, outPos);
        return;
    }
    SkASSERT(matrixUniform);

    bool useCompactTransform = matrix.isScaleTranslate() && !shaderCaps.reducedShaderMode();
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
                                     outName.c_str(),
                                     mangledMatrixName,
                                     inPos.getName().c_str(),
                                     mangledMatrixName);
        } else {
            vertBuilder->codeAppendf("float3 %s = %s * %s;\n",
                                     outName.c_str(),
                                     mangledMatrixName,
                                     inPos.getName().c_str());
        }
        outPos->set(kFloat3_GrSLType, outName.c_str());
        return;
    }
    if (matrix.hasPerspective()) {
        // A float2 is promoted to a float3 if we add perspective via the matrix
        SkASSERT(!useCompactTransform);
        vertBuilder->codeAppendf("float3 %s = (%s * %s.xy1);",
                                 outName.c_str(),
                                 mangledMatrixName,
                                 inPos.getName().c_str());
        outPos->set(kFloat3_GrSLType, outName.c_str());
        return;
    }
    if (useCompactTransform) {
        vertBuilder->codeAppendf("float2 %s = %s.xz * %s + %s.yw;\n",
                                 outName.c_str(),
                                 mangledMatrixName,
                                 inPos.getName().c_str(),
                                 mangledMatrixName);
    } else if (shaderCaps.nonsquareMatrixSupport()) {
        vertBuilder->codeAppendf("float2 %s = float3x2(%s) * %s.xy1;\n",
                                 outName.c_str(),
                                 mangledMatrixName,
                                 inPos.getName().c_str());
    } else {
        vertBuilder->codeAppendf("float2 %s = (%s * %s.xy1).xy;\n",
                                 outName.c_str(),
                                 mangledMatrixName,
                                 inPos.getName().c_str());
    }
    outPos->set(kFloat2_GrSLType, outName.c_str());
}

void ProgramImpl::WriteOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                      GrGPArgs* gpArgs,
                                      const char* posName) {
    // writeOutputPosition assumes the incoming pos name points to a float2 variable
    GrShaderVar inPos(posName, kFloat2_GrSLType);
    write_passthrough_vertex_position(vertBuilder, inPos, &gpArgs->fPositionVar);
}

void ProgramImpl::WriteOutputPosition(GrGLSLVertexBuilder* vertBuilder,
                                      GrGLSLUniformHandler* uniformHandler,
                                      const GrShaderCaps& shaderCaps,
                                      GrGPArgs* gpArgs,
                                      const char* posName,
                                      const SkMatrix& mat,
                                      UniformHandle* viewMatrixUniform) {
    GrShaderVar inPos(posName, kFloat2_GrSLType);
    write_vertex_position(vertBuilder,
                          uniformHandler,
                          shaderCaps,
                          inPos,
                          mat,
                          "viewMatrix",
                          &gpArgs->fPositionVar,
                          viewMatrixUniform);
}

void ProgramImpl::WriteLocalCoord(GrGLSLVertexBuilder* vertBuilder,
                                  GrGLSLUniformHandler* uniformHandler,
                                  const GrShaderCaps& shaderCaps,
                                  GrGPArgs* gpArgs,
                                  GrShaderVar localVar,
                                  const SkMatrix& localMatrix,
                                  UniformHandle* localMatrixUniform) {
    write_vertex_position(vertBuilder,
                          uniformHandler,
                          shaderCaps,
                          localVar,
                          localMatrix,
                          "localMatrix",
                          &gpArgs->fLocalCoordVar,
                          localMatrixUniform);
}

//////////////////////////////////////////////////////////////////////////////

using Attribute    = GrGeometryProcessor::Attribute;
using AttributeSet = GrGeometryProcessor::AttributeSet;

GrGeometryProcessor::Attribute AttributeSet::Iter::operator*() const {
    if (fCurr->offset().has_value()) {
        return *fCurr;
    }
    return Attribute(fCurr->name(), fCurr->cpuType(), fCurr->gpuType(), fImplicitOffset);
}

void AttributeSet::Iter::operator++() {
    if (fRemaining) {
        fRemaining--;
        fImplicitOffset += Attribute::AlignOffset(fCurr->size());
        fCurr++;
        this->skipUninitialized();
    }
}

void AttributeSet::Iter::skipUninitialized() {
    if (!fRemaining) {
        fCurr = nullptr;
    } else {
        while (!fCurr->isInitialized()) {
            ++fCurr;
        }
    }
}

void AttributeSet::initImplicit(const Attribute* attrs, int count) {
    fAttributes = attrs;
    fRawCount   = count;
    fCount      = 0;
    fStride     = 0;
    for (int i = 0; i < count; ++i) {
        if (attrs[i].isInitialized()) {
            fCount++;
            fStride += Attribute::AlignOffset(attrs[i].size());
        }
    }
}

void AttributeSet::initExplicit(const Attribute* attrs, int count, size_t stride) {
    fAttributes = attrs;
    fRawCount   = count;
    fCount      = count;
    fStride     = stride;
    SkASSERT(Attribute::AlignOffset(fStride) == fStride);
    for (int i = 0; i < count; ++i) {
        SkASSERT(attrs[i].isInitialized());
        SkASSERT(attrs[i].offset().has_value());
        SkASSERT(Attribute::AlignOffset(*attrs[i].offset()) == *attrs[i].offset());
        SkASSERT(*attrs[i].offset() + attrs[i].size() <= fStride);
    }
}

void AttributeSet::addToKey(skgpu::KeyBuilder* b) const {
    int rawCount = SkAbs32(fRawCount);
    b->addBits(16, SkToU16(this->stride()), "stride");
    b->addBits(16, rawCount, "attribute count");
    size_t implicitOffset = 0;
    for (int i = 0; i < rawCount; ++i) {
        const Attribute& attr = fAttributes[i];
        b->appendComment(attr.isInitialized() ? attr.name() : "unusedAttr");
        static_assert(kGrVertexAttribTypeCount < (1 << 8), "");
        static_assert(kGrSLTypeCount           < (1 << 8), "");
        b->addBits(8,  attr.isInitialized() ? attr.cpuType() : 0xff, "attrType");
        b->addBits(8 , attr.isInitialized() ? attr.gpuType() : 0xff, "attrGpuType");
        int16_t offset = -1;
        if (attr.isInitialized()) {
            if (attr.offset().has_value()) {
                offset = *attr.offset();
            } else {
                offset = implicitOffset;
                implicitOffset += Attribute::AlignOffset(attr.size());
            }
        }
        b->addBits(16, static_cast<uint16_t>(offset), "attrOffset");
    }
}

AttributeSet::Iter AttributeSet::begin() const { return Iter(fAttributes, fCount); }
AttributeSet::Iter AttributeSet::end() const { return Iter(); }
