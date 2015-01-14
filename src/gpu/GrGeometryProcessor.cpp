/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGeometryProcessor.h"

#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "gl/GrGLGeometryProcessor.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The key for an individual coord transform is made up of a matrix type, a precision, and a bit
 * that indicates the source of the input coords.
 */
enum {
    kMatrixTypeKeyBits   = 1,
    kMatrixTypeKeyMask   = (1 << kMatrixTypeKeyBits) - 1,

    kPrecisionBits       = 2,
    kPrecisionShift      = kMatrixTypeKeyBits,

    kPositionCoords_Flag = (1 << (kPrecisionShift + kPrecisionBits)),
    kDeviceCoords_Flag   = kPositionCoords_Flag + kPositionCoords_Flag,

    kTransformKeyBits    = kMatrixTypeKeyBits + kPrecisionBits + 2,
};

GR_STATIC_ASSERT(kHigh_GrSLPrecision < (1 << kPrecisionBits));

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

uint32_t
GrPrimitiveProcessor::getTransformKey(const SkTArray<const GrCoordTransform*, true>& coords) const {
    uint32_t totalKey = 0;
    for (int t = 0; t < coords.count(); ++t) {
        uint32_t key = 0;
        const GrCoordTransform* coordTransform = coords[t];
        if (coordTransform->getMatrix().hasPerspective()) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }

        if (kLocal_GrCoordSet == coordTransform->sourceCoords() &&
            !this->hasExplicitLocalCoords()) {
            key |= kPositionCoords_Flag;
        } else if (kDevice_GrCoordSet == coordTransform->sourceCoords()) {
            key |= kDeviceCoords_Flag;
        }

        GR_STATIC_ASSERT(kGrSLPrecisionCount <= (1 << kPrecisionBits));
        key |= (coordTransform->precision() << kPrecisionShift);

        key <<= kTransformKeyBits * t;

        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void GrGeometryProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    if (fHasVertexColor) {
        if (fOpaqueVertexColors) {
            out->setUnknownOpaqueFourComponents();
        } else {
            out->setUnknownFourComponents();
        }
    } else {
        out->setKnownFourComponents(fColor);
    }
    this->onGetInvariantOutputColor(out);
}

void GrGeometryProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    this->onGetInvariantOutputCoverage(out);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gl/builders/GrGLProgramBuilder.h"

SkMatrix GrGLPrimitiveProcessor::GetTransformMatrix(const SkMatrix& localMatrix,
                                                    const GrCoordTransform& coordTransform) {
    SkMatrix combined;
    // We only apply the localmatrix to localcoords
    if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
        combined.setConcat(coordTransform.getMatrix(), localMatrix);
    } else {
        combined = coordTransform.getMatrix();
    }
    if (coordTransform.reverseY()) {
        // combined.postScale(1,-1);
        // combined.postTranslate(0,1);
        combined.set(SkMatrix::kMSkewY,
            combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
        combined.set(SkMatrix::kMScaleY,
            combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
        combined.set(SkMatrix::kMTransY,
            combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
    }
    return combined;
}

void
GrGLPrimitiveProcessor::setupColorPassThrough(GrGLGPBuilder* pb,
                                              GrGPInput inputType,
                                              const char* outputName,
                                              const GrGeometryProcessor::Attribute* colorAttr,
                                              UniformHandle* colorUniform) {
    GrGLGPFragmentBuilder* fs = pb->getFragmentShaderBuilder();
    if (kUniform_GrGPInput == inputType) {
        SkASSERT(colorUniform);
        const char* stagedLocalVarName;
        *colorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "Color",
                                       &stagedLocalVarName);
        fs->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
    } else if (kAttribute_GrGPInput == inputType) {
        SkASSERT(colorAttr);
        pb->addPassThroughAttribute(colorAttr, outputName);
    } else if (kAllOnes_GrGPInput == inputType) {
        fs->codeAppendf("%s = vec4(1);", outputName);
    }
}

void GrGLPrimitiveProcessor::addUniformViewMatrix(GrGLGPBuilder* pb) {
    fViewMatrixUniform = pb->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                        kMat33f_GrSLType, kDefault_GrSLPrecision,
                                        "uViewM",
                                        &fViewMatrixName);
}

void GrGLPrimitiveProcessor::setUniformViewMatrix(const GrGLProgramDataManager& pdman,
                                                  const SkMatrix& viewMatrix) {
    if (!fViewMatrix.cheapEqualTo(viewMatrix)) {
        SkASSERT(fViewMatrixUniform.isValid());
        fViewMatrix = viewMatrix;

        GrGLfloat viewMatrix[3 * 3];
        GrGLGetMatrix<3>(viewMatrix, fViewMatrix);
        pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


void GrGLGeometryProcessor::emitCode(EmitArgs& args) {
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
    vsBuilder->codeAppendf("vec3 %s;", this->position());
    this->onEmitCode(args);
    vsBuilder->transformToNormalizedDeviceSpace(this->position());
}

void GrGLGeometryProcessor::emitTransforms(GrGLGPBuilder* pb,
                                           const char* position,
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
                    vb->codeAppendf("%s = (%s * %s).xy;", v.vsOut(), uniName, position);
                } else {
                    vb->codeAppendf("%s = %s * %s;", v.vsOut(), uniName, position);
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


void
GrGLGeometryProcessor::setTransformData(const GrPrimitiveProcessor* primProc,
                                        const GrGLProgramDataManager& pdman,
                                        int index,
                                        const SkTArray<const GrCoordTransform*, true>& transforms) {
    SkSTArray<2, Transform, true>& procTransforms = fInstalledTransforms[index];
    int numTransforms = transforms.count();
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(procTransforms[t].fHandle.isValid());
        const SkMatrix& transform = GetTransformMatrix(primProc->localMatrix(), *transforms[t]);
        if (!procTransforms[t].fCurrentValue.cheapEqualTo(transform)) {
            pdman.setSkMatrix(procTransforms[t].fHandle.convertToUniformHandle(), transform);
            procTransforms[t].fCurrentValue = transform;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gl/GrGLGpu.h"
#include "gl/GrGLPathRendering.h"

struct PathBatchTracker {
    GrGPInput fInputColorType;
    GrGPInput fInputCoverageType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

GrGLPathProcessor::GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&)
    : fColor(GrColor_ILLEGAL) {}

void GrGLPathProcessor::emitCode(EmitArgs& args) {
    GrGLGPBuilder* pb = args.fPB;
    GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
    const PathBatchTracker& local = args.fBT.cast<PathBatchTracker>();

    // emit transforms
    this->emitTransforms(args.fPB, args.fTransformsIn, args.fTransformsOut);

    // Setup uniform color
    if (kUniform_GrGPInput == local.fInputColorType) {
        const char* stagedLocalVarName;
        fColorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "Color",
                                       &stagedLocalVarName);
        fs->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);
    }

    // setup constant solid coverage
    if (kAllOnes_GrGPInput == local.fInputCoverageType) {
        fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }
}

void GrGLPathProcessor::GenKey(const GrPathProcessor&,
                               const GrBatchTracker& bt,
                               const GrGLCaps&,
                               GrProcessorKeyBuilder* b) {
    const PathBatchTracker& local = bt.cast<PathBatchTracker>();
    b->add32(local.fInputColorType | local.fInputCoverageType << 16);
}

void GrGLPathProcessor::setData(const GrGLProgramDataManager& pdman,
                                const GrPrimitiveProcessor& primProc,
                                const GrBatchTracker& bt) {
    const PathBatchTracker& local = bt.cast<PathBatchTracker>();
    if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(local.fColor, c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = local.fColor;
    }
}

class GrGLLegacyPathProcessor : public GrGLPathProcessor {
public:
    GrGLLegacyPathProcessor(const GrPathProcessor& pathProc, const GrBatchTracker& bt,
                            int maxTexCoords)
        : INHERITED(pathProc, bt)
        , fTexCoordSetCnt(0) {
            SkDEBUGCODE(fMaxTexCoords = maxTexCoords;)
    }

    int addTexCoordSets(int count) {
        int firstFreeCoordSet = fTexCoordSetCnt;
        fTexCoordSetCnt += count;
        SkASSERT(fMaxTexCoords >= fTexCoordSetCnt);
        return firstFreeCoordSet;
    }

    void emitTransforms(GrGLGPBuilder*, const TransformsIn& tin, TransformsOut* tout) SK_OVERRIDE {
        tout->push_back_n(tin.count());
        fInstalledTransforms.push_back_n(tin.count());
        for (int i = 0; i < tin.count(); i++) {
            const ProcCoords& coordTransforms = tin[i];
            int texCoordIndex = this->addTexCoordSets(coordTransforms.count());

            // Use the first uniform location as the texcoord index.
            fInstalledTransforms[i].push_back_n(1);
            fInstalledTransforms[i][0].fHandle = ShaderVarHandle(texCoordIndex);

            SkString name;
            for (int t = 0; t < coordTransforms.count(); ++t) {
                GrSLType type = coordTransforms[t]->getMatrix().hasPerspective() ? kVec3f_GrSLType :
                                                                                   kVec2f_GrSLType;

                name.printf("%s(gl_TexCoord[%i])", GrGLSLTypeString(type), texCoordIndex++);
                SkNEW_APPEND_TO_TARRAY(&(*tout)[i], GrGLProcessor::TransformedCoords, (name, type));
            }
        }
    }

    void setTransformData(const GrPrimitiveProcessor* primProc,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms,
                          GrGLPathRendering* glpr,
                          GrGLuint) SK_OVERRIDE {
        // We've hidden the texcoord index in the first entry of the transforms array for each
        // effect
        int texCoordIndex = fInstalledTransforms[index][0].fHandle.handle();
        for (int t = 0; t < transforms.count(); ++t) {
            const SkMatrix& transform = GetTransformMatrix(primProc->localMatrix(), *transforms[t]);
            GrGLPathRendering::PathTexGenComponents components =
                    GrGLPathRendering::kST_PathTexGenComponents;
            if (transform.hasPerspective()) {
                components = GrGLPathRendering::kSTR_PathTexGenComponents;
            }
            glpr->enablePathTexGen(texCoordIndex++, components, transform);
        }
    }

    void didSetData(GrGLPathRendering* glpr) SK_OVERRIDE {
        glpr->flushPathTexGenSettings(fTexCoordSetCnt);
    }

private:
    SkDEBUGCODE(int fMaxTexCoords;)
    int fTexCoordSetCnt;

    typedef GrGLPathProcessor INHERITED;
};

class GrGLNormalPathProcessor : public GrGLPathProcessor {
public:
    GrGLNormalPathProcessor(const GrPathProcessor& pathProc, const GrBatchTracker& bt)
        : INHERITED(pathProc, bt) {}

    void emitTransforms(GrGLGPBuilder* pb, const TransformsIn& tin,
                        TransformsOut* tout) SK_OVERRIDE {
        tout->push_back_n(tin.count());
        fInstalledTransforms.push_back_n(tin.count());
        for (int i = 0; i < tin.count(); i++) {
            const ProcCoords& coordTransforms = tin[i];
            fInstalledTransforms[i].push_back_n(coordTransforms.count());
            for (int t = 0; t < coordTransforms.count(); t++) {
                GrSLType varyingType =
                        coordTransforms[t]->getMatrix().hasPerspective() ? kVec3f_GrSLType :
                                                                           kVec2f_GrSLType;


                SkString strVaryingName("MatrixCoord");
                strVaryingName.appendf("_%i_%i", i, t);
                GrGLVertToFrag v(varyingType);
                pb->addVarying(strVaryingName.c_str(), &v);
                SeparableVaryingInfo& varyingInfo = fSeparableVaryingInfos.push_back();
                varyingInfo.fVariable = pb->getFragmentShaderBuilder()->fInputs.back();
                varyingInfo.fLocation = fSeparableVaryingInfos.count() - 1;
                varyingInfo.fType = varyingType;
                fInstalledTransforms[i][t].fHandle = ShaderVarHandle(varyingInfo.fLocation);
                fInstalledTransforms[i][t].fType = varyingType;

                SkNEW_APPEND_TO_TARRAY(&(*tout)[i], GrGLProcessor::TransformedCoords,
                                       (SkString(v.fsIn()), varyingType));
            }
        }
    }

    void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId) {
        int count = fSeparableVaryingInfos.count();
        for (int i = 0; i < count; ++i) {
            GrGLint location;
            GR_GL_CALL_RET(gpu->glInterface(),
                           location,
                           GetProgramResourceLocation(programId,
                                                      GR_GL_FRAGMENT_INPUT,
                                                      fSeparableVaryingInfos[i].fVariable.c_str()));
            fSeparableVaryingInfos[i].fLocation = location;
        }
    }

    void setTransformData(const GrPrimitiveProcessor* primProc,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& coordTransforms,
                          GrGLPathRendering* glpr,
                          GrGLuint programID) SK_OVERRIDE {
        SkSTArray<2, Transform, true>& transforms = fInstalledTransforms[index];
        int numTransforms = transforms.count();
        for (int t = 0; t < numTransforms; ++t) {
            SkASSERT(transforms[t].fHandle.isValid());
            const SkMatrix& transform = GetTransformMatrix(primProc->localMatrix(),
                                                           *coordTransforms[t]);
            if (transforms[t].fCurrentValue.cheapEqualTo(transform)) {
                continue;
            }
            transforms[t].fCurrentValue = transform;
            const SeparableVaryingInfo& fragmentInput =
                    fSeparableVaryingInfos[transforms[t].fHandle.handle()];
            SkASSERT(transforms[t].fType == kVec2f_GrSLType ||
                     transforms[t].fType == kVec3f_GrSLType);
            unsigned components = transforms[t].fType == kVec2f_GrSLType ? 2 : 3;
            glpr->setProgramPathFragmentInputTransform(programID,
                                                       fragmentInput.fLocation,
                                                       GR_GL_OBJECT_LINEAR,
                                                       components,
                                                       transform);
        }
    }

private:
    struct SeparableVaryingInfo {
        GrSLType      fType;
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };


    typedef SkSTArray<8, SeparableVaryingInfo, true> SeparableVaryingInfoArray;

    SeparableVaryingInfoArray fSeparableVaryingInfos;

    typedef GrGLPathProcessor INHERITED;
};

GrPathProcessor::GrPathProcessor(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
    : INHERITED(viewMatrix, localMatrix)
    , fColor(color) {
    this->initClassID<GrPathProcessor>();
}

void GrPathProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    out->setKnownFourComponents(fColor);
}

void GrPathProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setKnownSingleComponent(0xff);
}

void GrPathProcessor::initBatchTracker(GrBatchTracker* bt, const InitBT& init) const {
    PathBatchTracker* local = bt->cast<PathBatchTracker>();
    if (init.fColorIgnored) {
        local->fInputColorType = kIgnored_GrGPInput;
        local->fColor = GrColor_ILLEGAL;
    } else {
        local->fInputColorType = kUniform_GrGPInput;
        local->fColor = GrColor_ILLEGAL == init.fOverrideColor ? this->color() :
                                                                 init.fOverrideColor;
    }

    local->fInputCoverageType = init.fCoverageIgnored ? kIgnored_GrGPInput : kAllOnes_GrGPInput;
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrPathProcessor::canMakeEqual(const GrBatchTracker& m,
                                   const GrPrimitiveProcessor& that,
                                   const GrBatchTracker& t) const {
    if (this->classID() != that.classID() || !this->hasSameTextureAccesses(that)) {
        return false;
    }

    if (!this->viewMatrix().cheapEqualTo(that.viewMatrix())) {
        return false;
    }

    const PathBatchTracker& mine = m.cast<PathBatchTracker>();
    const PathBatchTracker& theirs = t.cast<PathBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor) &&
           CanCombineOutput(mine.fInputCoverageType, 0xff,
                            theirs.fInputCoverageType, 0xff);
}

void GrPathProcessor::getGLProcessorKey(const GrBatchTracker& bt,
                                        const GrGLCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLPathProcessor::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* GrPathProcessor::createGLInstance(const GrBatchTracker& bt,
                                                          const GrGLCaps& caps) const {
    SkASSERT(caps.nvprSupport() != GrGLCaps::kNone_NvprSupport);
    if (caps.nvprSupport() == GrGLCaps::kLegacy_NvprSupport) {
        return SkNEW_ARGS(GrGLLegacyPathProcessor, (*this, bt,
                                                    caps.maxFixedFunctionTextureCoords()));
    } else {
        return SkNEW_ARGS(GrGLNormalPathProcessor, (*this, bt));
    }
}

