/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFragmentShaderBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "GrGLProgramBuilder.h"
#include "../GrGpuGL.h"

namespace {
#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)
// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;
static const char kDstCopyColorName[] = "_dstColor";
inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }
inline void append_default_precision_qualifier(GrGLShaderVar::Precision p,
                                               GrGLStandard standard,
                                               SkString* str) {
    // Desktop GLSL has added precision qualifiers but they don't do anything.
    if (kGLES_GrGLStandard == standard) {
        switch (p) {
            case GrGLShaderVar::kHigh_Precision:
                str->append("precision highp float;\n");
                break;
            case GrGLShaderVar::kMedium_Precision:
                str->append("precision mediump float;\n");
                break;
            case GrGLShaderVar::kLow_Precision:
                str->append("precision lowp float;\n");
                break;
            case GrGLShaderVar::kDefault_Precision:
                SkFAIL("Default precision now allowed.");
            default:
                SkFAIL("Unknown precision value.");
        }
    }
}
}

GrGLFragmentShaderBuilder::DstReadKey GrGLFragmentShaderBuilder::KeyForDstRead(
        const GrTexture* dstCopy, const GrGLCaps& caps) {
    uint32_t key = kYesDstRead_DstReadKeyBit;
    if (caps.fbFetchSupport()) {
        return key;
    }
    SkASSERT(dstCopy);
    if (!caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(dstCopy->config())) {
        // The fact that the config is alpha-only must be considered when generating code.
        key |= kUseAlphaConfig_DstReadKeyBit;
    }
    if (kTopLeft_GrSurfaceOrigin == dstCopy->origin()) {
        key |= kTopLeftOrigin_DstReadKeyBit;
    }
    SkASSERT(static_cast<DstReadKey>(key) == key);
    return static_cast<DstReadKey>(key);
}

GrGLFragmentShaderBuilder::FragPosKey GrGLFragmentShaderBuilder::KeyForFragmentPosition(
        const GrRenderTarget* dst, const GrGLCaps&) {
    if (kTopLeft_GrSurfaceOrigin == dst->origin()) {
        return kTopLeftFragPosRead_FragPosKey;
    } else {
        return kBottomLeftFragPosRead_FragPosKey;
    }
}

GrGLFragmentShaderBuilder::GrGLFragmentShaderBuilder(GrGLProgramBuilder* program,
                                                     const GrGLProgramDesc& desc)
    : INHERITED(program)
    , fHasCustomColorOutput(false)
    , fHasSecondaryOutput(false)
    , fSetupFragPosition(false)
    , fTopLeftFragPosRead(kTopLeftFragPosRead_FragPosKey == desc.getHeader().fFragPosKey){
}

const char* GrGLFragmentShaderBuilder::dstColor() {
    if (fProgramBuilder->fCodeStage.inStageCode()) {
        const GrProcessor* effect = fProgramBuilder->fCodeStage.effectStage()->getProcessor();
        // TODO GPs can't read dst color, and full program builder only returns a pointer to the
        // base fragment shader builder which does not have this function.  Unfortunately,
        // the code stage class only has a GrProcessor pointer so this is required for the time
        // being
        if (!static_cast<const GrFragmentProcessor*>(effect)->willReadDstColor()) {
            SkDEBUGFAIL("GrGLProcessor asked for dst color but its generating GrProcessor "
                        "did not request access.");
            return "";
        }
    }

    GrGpuGL* gpu = fProgramBuilder->gpu();
    if (gpu->glCaps().fbFetchSupport()) {
        this->addFeature(1 << (GrGLFragmentShaderBuilder::kLastGLSLPrivateFeature + 1),
                         gpu->glCaps().fbFetchExtensionString());
        return gpu->glCaps().fbFetchColorName();
    } else if (fProgramBuilder->fUniformHandles.fDstCopySamplerUni.isValid()) {
        return kDstCopyColorName;
    } else {
        return "";
    }
}

bool GrGLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature: {
            GrGpuGL* gpu = fProgramBuilder->gpu();
            if (!gpu->glCaps().shaderDerivativeSupport()) {
                return false;
            }
            if (kGLES_GrGLStandard == gpu->glStandard()) {
                this->addFeature(1 << kStandardDerivatives_GLSLFeature,
                                 "GL_OES_standard_derivatives");
            }
            return true;
        }
        default:
            SkFAIL("Unexpected GLSLFeature requested.");
            return false;
    }
}

SkString GrGLFragmentShaderBuilder::ensureFSCoords2D(
        const GrGLProcessor::TransformedCoordsArray& coords, int index) {
    if (kVec3f_GrSLType != coords[index].getType()) {
        SkASSERT(kVec2f_GrSLType == coords[index].getType());
        return coords[index].getName();
    }

    SkString coords2D("coords2D");
    if (0 != index) {
        coords2D.appendf("_%i", index);
    }
    this->codeAppendf("\tvec2 %s = %s.xy / %s.z;",
                      coords2D.c_str(), coords[index].c_str(), coords[index].c_str());
    return coords2D;
}

const char* GrGLFragmentShaderBuilder::fragmentPosition() {
    GrGLProgramBuilder::CodeStage* cs = &fProgramBuilder->fCodeStage;
    if (cs->inStageCode()) {
        const GrProcessor* effect = cs->effectStage()->getProcessor();
        if (!effect->willReadFragmentPosition()) {
            SkDEBUGFAIL("GrGLProcessor asked for frag position but its generating GrProcessor "
                        "did not request access.");
            return "";
        }
    }

    GrGpuGL* gpu = fProgramBuilder->gpu();
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (fTopLeftFragPosRead) {
        fSetupFragPosition = true;
        return "gl_FragCoord";
    } else if (gpu->glCaps().fragCoordConventionsSupport()) {
        if (!fSetupFragPosition) {
            if (gpu->glslGeneration() < k150_GrGLSLGeneration) {
                this->addFeature(1 << kFragCoordConventions_GLSLPrivateFeature,
                                 "GL_ARB_fragment_coord_conventions");
            }
            fInputs.push_back().set(kVec4f_GrSLType,
                                    GrGLShaderVar::kIn_TypeModifier,
                                    "gl_FragCoord",
                                    GrGLShaderVar::kDefault_Precision,
                                    GrGLShaderVar::kUpperLeft_Origin);
            fSetupFragPosition = true;
        }
        return "gl_FragCoord";
    } else {
        static const char* kCoordName = "fragCoordYDown";
        if (!fSetupFragPosition) {
            // temporarily change the stage index because we're inserting non-stage code.
            GrGLProgramBuilder::CodeStage::AutoStageRestore csar(cs, NULL);

            SkASSERT(!fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
            const char* rtHeightName;

            fProgramBuilder->fUniformHandles.fRTHeightUni =
                    fProgramBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kFloat_GrSLType,
                                         "RTHeight",
                                         &rtHeightName);

            // Using glFragCoord.zw for the last two components tickles an Adreno driver bug that
            // causes programs to fail to link. Making this function return a vec2() didn't fix the
            // problem but using 1.0 for the last two components does.
            this->codePrependf("\tvec4 %s = vec4(gl_FragCoord.x, %s - gl_FragCoord.y, 1.0, "
                               "1.0);\n", kCoordName, rtHeightName);
            fSetupFragPosition = true;
        }
        SkASSERT(fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
        return kCoordName;
    }
}

void GrGLFragmentShaderBuilder::addVarying(GrSLType type,
               const char* name,
               const char** fsInName,
               GrGLShaderVar::Precision fsPrecision) {
    fInputs.push_back().set(type, GrGLShaderVar::kVaryingIn_TypeModifier, name, fsPrecision);
    if (fsInName) {
        *fsInName = name;
    }
}

void GrGLFragmentShaderBuilder::bindProgramLocations(GrGLuint programId) {
    GrGpuGL* gpu = fProgramBuilder->gpu();
    if (fHasCustomColorOutput) {
        GL_CALL(BindFragDataLocation(programId, 0, declared_color_output_name()));
    }
    if (fHasSecondaryOutput) {
        GL_CALL(BindFragDataLocationIndexed(programId, 0, 1, dual_source_output_name()));
    }
}

bool GrGLFragmentShaderBuilder::compileAndAttachShaders(GrGLuint programId,
                                                        SkTDArray<GrGLuint>* shaderIds) const {
    GrGpuGL* gpu = fProgramBuilder->gpu();
    SkString fragShaderSrc(GrGetGLSLVersionDecl(gpu->ctxInfo()));
    fragShaderSrc.append(fExtensions);
    append_default_precision_qualifier(kDefaultFragmentPrecision,
                                       gpu->glStandard(),
                                       &fragShaderSrc);
    fProgramBuilder->appendUniformDecls(GrGLProgramBuilder::kFragment_Visibility, &fragShaderSrc);
    fProgramBuilder->appendDecls(fInputs, &fragShaderSrc);
    // We shouldn't have declared outputs on 1.10
    SkASSERT(k110_GrGLSLGeneration != gpu->glslGeneration() || fOutputs.empty());
    fProgramBuilder->appendDecls(fOutputs, &fragShaderSrc);
    fragShaderSrc.append(fFunctions);
    fragShaderSrc.append("void main() {\n");
    fragShaderSrc.append(fCode);
    fragShaderSrc.append("}\n");

    GrGLuint fragShaderId = GrGLCompileAndAttachShader(gpu->glContext(), programId,
                                                       GR_GL_FRAGMENT_SHADER, fragShaderSrc,
                                                       gpu->gpuStats());
    if (!fragShaderId) {
        return false;
    }

    *shaderIds->append() = fragShaderId;

    return true;
}

void GrGLFragmentShaderBuilder::emitCodeBeforeEffects() {
    const GrGLProgramDesc::KeyHeader& header = fProgramBuilder->desc().getHeader();
    GrGpuGL* gpu = fProgramBuilder->gpu();

    ///////////////////////////////////////////////////////////////////////////
    // emit code to read the dst copy texture, if necessary
    if (kNoDstRead_DstReadKey != header.fDstReadKey && !gpu->glCaps().fbFetchSupport()) {
        bool topDown = SkToBool(kTopLeftOrigin_DstReadKeyBit & header.fDstReadKey);
        const char* dstCopyTopLeftName;
        const char* dstCopyCoordScaleName;
        const char* dstCopySamplerName;
        uint32_t configMask;
        if (SkToBool(kUseAlphaConfig_DstReadKeyBit & header.fDstReadKey)) {
            configMask = kA_GrColorComponentFlag;
        } else {
            configMask = kRGBA_GrColorComponentFlags;
        }
        fProgramBuilder->fUniformHandles.fDstCopySamplerUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                 kSampler2D_GrSLType,
                                 "DstCopySampler",
                                 &dstCopySamplerName);
        fProgramBuilder->fUniformHandles.fDstCopyTopLeftUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                 kVec2f_GrSLType,
                                 "DstCopyUpperLeft",
                                 &dstCopyTopLeftName);
        fProgramBuilder->fUniformHandles.fDstCopyScaleUni =
            fProgramBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                 kVec2f_GrSLType,
                                 "DstCopyCoordScale",
                                 &dstCopyCoordScaleName);
        const char* fragPos = fragmentPosition();

        this->codeAppend("// Read color from copy of the destination.\n");
        this->codeAppendf("vec2 _dstTexCoord = (%s.xy - %s) * %s;",
                          fragPos, dstCopyTopLeftName, dstCopyCoordScaleName);
        if (!topDown) {
            this->codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;");
        }
        this->codeAppendf("vec4 %s = ", kDstCopyColorName);
        this->appendTextureLookup(dstCopySamplerName,
                                  "_dstTexCoord",
                                  configMask,
                                  "rgba");
        this->codeAppend(";");
    }

    if (k110_GrGLSLGeneration != gpu->glslGeneration()) {
        fOutputs.push_back().set(kVec4f_GrSLType,
                                 GrGLShaderVar::kOut_TypeModifier,
                                 declared_color_output_name());
        fHasCustomColorOutput = true;
    }
}

void GrGLFragmentShaderBuilder::emitCodeAfterEffects(const GrGLSLExpr4& inputColor, const GrGLSLExpr4& inputCoverage) {
    const GrGLProgramDesc::KeyHeader& header = fProgramBuilder->desc().getHeader();

    ///////////////////////////////////////////////////////////////////////////
    // write the secondary color output if necessary
    if (GrOptDrawState::kNone_SecondaryOutputType != header.fSecondaryOutputType) {
        const char* secondaryOutputName = this->enableSecondaryOutput();
        GrGLSLExpr4 coeff(1);
        switch (header.fSecondaryOutputType) {
            case GrOptDrawState::kCoverage_SecondaryOutputType:
                break;
            case GrOptDrawState::kCoverageISA_SecondaryOutputType:
                // Get (1-A) into coeff
                coeff = GrGLSLExpr4::VectorCast(GrGLSLExpr1(1) - inputColor.a());
                break;
            case GrOptDrawState::kCoverageISC_SecondaryOutputType:
                // Get (1-RGBA) into coeff
                coeff = GrGLSLExpr4(1) - inputColor;
                break;
            default:
                SkFAIL("Unexpected Secondary Output");
        }
        // Get coeff * coverage into modulate and then write that to the dual source output.
        codeAppendf("\t%s = %s;\n", secondaryOutputName, (coeff * inputCoverage).c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    // Get "color * coverage" into fragColor
    GrGLSLExpr4 fragColor = inputColor * inputCoverage;
    switch (header.fPrimaryOutputType) {
        case GrOptDrawState::kModulate_PrimaryOutputType:
            break;
        case GrOptDrawState::kCombineWithDst_PrimaryOutputType:
            {
                // Tack on "+(1-coverage)dst onto the frag color.
                GrGLSLExpr4 dstCoeff = GrGLSLExpr4(1) - inputCoverage;
                GrGLSLExpr4 dstContribution = dstCoeff * GrGLSLExpr4(dstColor());
                fragColor = fragColor + dstContribution;
            }
            break;
        default:
            SkFAIL("Unknown Primary Output");
    }
    codeAppendf("\t%s = %s;\n", this->getColorOutputName(), fragColor.c_str());
}

const char* GrGLFragmentShaderBuilder::enableSecondaryOutput() {
    if (!fHasSecondaryOutput) {
        fOutputs.push_back().set(kVec4f_GrSLType,
                                 GrGLShaderVar::kOut_TypeModifier,
                                 dual_source_output_name());
        fHasSecondaryOutput = true;
    }
    return dual_source_output_name();
}

const char* GrGLFragmentShaderBuilder::getColorOutputName() const {
    return fHasCustomColorOutput ? declared_color_output_name() : "gl_FragColor";
}

