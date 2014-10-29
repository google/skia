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

#define GL_CALL(X) GR_GL_CALL(fProgramBuilder->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fProgramBuilder->gpu()->glInterface(), R, X)

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;
const char* GrGLFragmentShaderBuilder::kDstCopyColorName = "_dstColor";
static const char* declared_color_output_name() { return "fsColorOut"; }
static const char* dual_source_output_name() { return "dualSourceOut"; }
static void append_default_precision_qualifier(GrGLShaderVar::Precision p,
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

GrGLFragmentShaderBuilder::DstReadKey
GrGLFragmentShaderBuilder::KeyForDstRead(const GrTexture* dstCopy, const GrGLCaps& caps) {
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

GrGLFragmentShaderBuilder::FragPosKey
GrGLFragmentShaderBuilder::KeyForFragmentPosition(const GrRenderTarget* dst, const GrGLCaps&) {
    if (kTopLeft_GrSurfaceOrigin == dst->origin()) {
        return kTopLeftFragPosRead_FragPosKey;
    } else {
        return kBottomLeftFragPosRead_FragPosKey;
    }
}

GrGLFragmentShaderBuilder::GrGLFragmentShaderBuilder(GrGLProgramBuilder* program,
                                                     uint8_t fragPosKey)
    : INHERITED(program)
    , fHasCustomColorOutput(false)
    , fHasSecondaryOutput(false)
    , fSetupFragPosition(false)
    , fTopLeftFragPosRead(kTopLeftFragPosRead_FragPosKey == fragPosKey)
    , fCustomColorOutputIndex(-1)
    , fHasReadDstColor(false)
    , fHasReadFragmentPosition(false) {
}

bool GrGLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature: {
            GrGpuGL* gpu = fProgramBuilder->gpu();
            if (!gpu->glCaps().shaderDerivativeSupport()) {
                return false;
            }
            if (kGLES_GrGLStandard == gpu->glStandard() &&
                k110_GrGLSLGeneration == gpu->glslGeneration()) {
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
    fHasReadFragmentPosition = true;

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
            GrGLProgramBuilder::AutoStageRestore asr(fProgramBuilder);
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

const char* GrGLFragmentShaderBuilder::dstColor() {
    fHasReadDstColor = true;

    GrGpuGL* gpu = fProgramBuilder->gpu();
    if (gpu->glCaps().fbFetchSupport()) {
        this->addFeature(1 << (GrGLFragmentShaderBuilder::kLastGLSLPrivateFeature + 1),
                         gpu->glCaps().fbFetchExtensionString());

        // On ES 3.0 we have to declare this, and use the custom color output name
        const char* fbFetchColorName = gpu->glCaps().fbFetchColorName();
        if (gpu->glslGeneration() >= k330_GrGLSLGeneration) {
            this->enableCustomOutput();
            fOutputs[fCustomColorOutputIndex].setTypeModifier(GrShaderVar::kInOut_TypeModifier);
            fbFetchColorName = declared_color_output_name();
        }
        return fbFetchColorName;
    } else if (fProgramBuilder->fUniformHandles.fDstCopySamplerUni.isValid()) {
        return kDstCopyColorName;
    } else {
        return "";
    }
}

void GrGLFragmentShaderBuilder::emitCodeToReadDstTexture() {
    bool topDown = SkToBool(kTopLeftOrigin_DstReadKeyBit & fProgramBuilder->header().fDstReadKey);
    const char* dstCopyTopLeftName;
    const char* dstCopyCoordScaleName;
    const char* dstCopySamplerName;
    uint32_t configMask;
    if (SkToBool(kUseAlphaConfig_DstReadKeyBit & fProgramBuilder->header().fDstReadKey)) {
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
    const char* fragPos = this->fragmentPosition();

    this->codeAppend("// Read color from copy of the destination.\n");
    this->codeAppendf("vec2 _dstTexCoord = (%s.xy - %s) * %s;",
                      fragPos, dstCopyTopLeftName, dstCopyCoordScaleName);
    if (!topDown) {
        this->codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;");
    }
    this->codeAppendf("vec4 %s = ", GrGLFragmentShaderBuilder::kDstCopyColorName);
    this->appendTextureLookup(dstCopySamplerName,
                              "_dstTexCoord",
                              configMask,
                              "rgba");
    this->codeAppend(";");
}

void GrGLFragmentShaderBuilder::enableCustomOutput() {
    if (!fHasCustomColorOutput) {
        fHasCustomColorOutput = true;
        fCustomColorOutputIndex = fOutputs.count();
        fOutputs.push_back().set(kVec4f_GrSLType,
                                 GrGLShaderVar::kOut_TypeModifier,
                                 declared_color_output_name());
    }
}

void GrGLFragmentShaderBuilder::enableSecondaryOutput() {
    SkASSERT(!fHasSecondaryOutput);
    fHasSecondaryOutput = true;
    fOutputs.push_back().set(kVec4f_GrSLType, GrGLShaderVar::kOut_TypeModifier,
                             dual_source_output_name());
}

const char* GrGLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? declared_color_output_name() : "gl_FragColor";
}

const char* GrGLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    return dual_source_output_name();
}

void GrGLFragmentShaderBuilder::enableSecondaryOutput(const GrGLSLExpr4& inputColor,
                                                      const GrGLSLExpr4& inputCoverage) {
    this->enableSecondaryOutput();
    const char* secondaryOutputName = this->getSecondaryColorOutputName();
    GrGLSLExpr4 coeff(1);
    switch (fProgramBuilder->header().fSecondaryOutputType) {
        case GrProgramDesc::kCoverage_SecondaryOutputType:
            break;
        case GrProgramDesc::kCoverageISA_SecondaryOutputType:
            // Get (1-A) into coeff
            coeff = GrGLSLExpr4::VectorCast(GrGLSLExpr1(1) - inputColor.a());
            break;
        case GrProgramDesc::kCoverageISC_SecondaryOutputType:
            // Get (1-RGBA) into coeff
            coeff = GrGLSLExpr4(1) - inputColor;
            break;
        default:
            SkFAIL("Unexpected Secondary Output");
    }
    // Get coeff * coverage into modulate and then write that to the dual source output.
    this->codeAppendf("\t%s = %s;\n", secondaryOutputName, (coeff * inputCoverage).c_str());
}

void GrGLFragmentShaderBuilder::combineColorAndCoverage(const GrGLSLExpr4& inputColor,
                                                        const GrGLSLExpr4& inputCoverage) {
    GrGLSLExpr4 fragColor = inputColor * inputCoverage;
    switch (fProgramBuilder->header().fPrimaryOutputType) {
        case GrProgramDesc::kModulate_PrimaryOutputType:
            break;
        case GrProgramDesc::kCombineWithDst_PrimaryOutputType:
            {
                // Tack on "+(1-coverage)dst onto the frag color.
                GrGLSLExpr4 dstCoeff = GrGLSLExpr4(1) - inputCoverage;
                GrGLSLExpr4 dstContribution = dstCoeff * GrGLSLExpr4(this->dstColor());
                fragColor = fragColor + dstContribution;
            }
            break;
        default:
            SkFAIL("Unknown Primary Output");
    }

    // On any post 1.10 GLSL supporting GPU, we declare custom output
    if (k110_GrGLSLGeneration != fProgramBuilder->gpu()->glslGeneration()) {
        this->enableCustomOutput();
    }

    this->codeAppendf("\t%s = %s;\n", this->getPrimaryColorOutputName(), fragColor.c_str());
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
    this->appendDecls(fInputs, &fragShaderSrc);
    // We shouldn't have declared outputs on 1.10
    SkASSERT(k110_GrGLSLGeneration != gpu->glslGeneration() || fOutputs.empty());
    this->appendDecls(fOutputs, &fragShaderSrc);
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

void GrGLFragmentShaderBuilder::bindFragmentShaderLocations(GrGLuint programID) {
    // ES 3.00 requires custom color output but doesn't support bindFragDataLocation
    if (fHasCustomColorOutput &&
        kGLES_GrGLStandard != fProgramBuilder->gpu()->ctxInfo().standard()) {
        GL_CALL(BindFragDataLocation(programID, 0, declared_color_output_name()));
    }
    if (fHasSecondaryOutput) {
        GL_CALL(BindFragDataLocationIndexed(programID, 0, 1, dual_source_output_name()));
    }
}

void GrGLFragmentShaderBuilder::addVarying(GrGLVarying* v, GrGLShaderVar::Precision fsPrec) {
    v->fFsIn = v->fVsOut;
    if (v->fGsOut) {
        v->fFsIn = v->fGsOut;
    }
    fInputs.push_back().set(v->fType, GrGLShaderVar::kVaryingIn_TypeModifier, v->fFsIn, fsPrec);
}
