/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrEffect.h"
#include "GrGLEffect.h"
#include "GrGpuGL.h"
#include "GrGLShaderVar.h"
#include "GrBackendEffectFactory.h"
#include "SkTrace.h"
#include "SkXfermode.h"

#include "SkRTConf.h"

SK_DEFINE_INST_COUNT(GrGLProgram)

#define GL_CALL(X) GR_GL_CALL(fContext.interface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fContext.interface(), R, X)

SK_CONF_DECLARE(bool, c_PrintShaders, "gpu.printShaders", false, "Print the source code for all shaders generated.");

#define TEX_ATTR_NAME "aTexCoord"
#define COL_ATTR_NAME "aColor"
#define COV_ATTR_NAME "aCoverage"
#define EDGE_ATTR_NAME "aEdge"

namespace {
inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }
}

const GrGLProgram::AttribLayout GrGLProgram::kAttribLayouts[kGrVertexAttribTypeCount] = {
    {1, GR_GL_FLOAT, false},         // kFloat_GrVertexAttribType
    {2, GR_GL_FLOAT, false},         // kVec2f_GrVertexAttribType
    {3, GR_GL_FLOAT, false},         // kVec3f_GrVertexAttribType
    {4, GR_GL_FLOAT, false},         // kVec4f_GrVertexAttribType
    {4, GR_GL_UNSIGNED_BYTE, true},  // kVec4ub_GrVertexAttribType
};

void GrGLProgram::BuildDesc(const GrDrawState& drawState,
                            bool isPoints,
                            GrDrawState::BlendOptFlags blendOpts,
                            GrBlendCoeff srcCoeff,
                            GrBlendCoeff dstCoeff,
                            const GrGpuGL* gpu,
                            Desc* desc) {

    // This should already have been caught
    GrAssert(!(GrDrawState::kSkipDraw_BlendOptFlag & blendOpts));

    bool skipCoverage = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);

    bool skipColor = SkToBool(blendOpts & (GrDrawState::kEmitTransBlack_BlendOptFlag |
                                           GrDrawState::kEmitCoverage_BlendOptFlag));

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    // Must initialize all fields or cache will have false negatives!
    desc->fAttribBindings = drawState.getAttribBindings();

    desc->fEmitsPointSize = isPoints;

    bool requiresAttributeColors = !skipColor &&
                                   SkToBool(desc->fAttribBindings & GrDrawState::kColor_AttribBindingsBit);
    bool requiresAttributeCoverage = !skipCoverage &&
                                     SkToBool(desc->fAttribBindings & GrDrawState::kCoverage_AttribBindingsBit);

    // fColorInput/fCoverageInput records how colors are specified for the program So we strip the
    // bits from the bindings to avoid false negatives when searching for an existing program in the
    // cache.
    desc->fAttribBindings &= ~(GrDrawState::kColor_AttribBindingsBit | GrDrawState::kCoverage_AttribBindingsBit);

    desc->fColorFilterXfermode = skipColor ?
                                SkXfermode::kDst_Mode :
                                drawState.getColorFilterMode();

    // no reason to do edge aa or look at per-vertex coverage if coverage is ignored
    if (skipCoverage) {
        desc->fAttribBindings &= ~(GrDrawState::kEdge_AttribBindingsBit |
                                   GrDrawState::kCoverage_AttribBindingsBit);
    }

    bool colorIsTransBlack = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);
    bool colorIsSolidWhite = (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) ||
                             (!requiresAttributeColors && 0xffffffff == drawState.getColor());
    if (colorIsTransBlack) {
        desc->fColorInput = Desc::kTransBlack_ColorInput;
    } else if (colorIsSolidWhite) {
        desc->fColorInput = Desc::kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeColors) {
        desc->fColorInput = Desc::kUniform_ColorInput;
    } else {
        desc->fColorInput = Desc::kAttribute_ColorInput;
    }

    bool covIsSolidWhite = !requiresAttributeCoverage && 0xffffffff == drawState.getCoverage();

    if (skipCoverage) {
        desc->fCoverageInput = Desc::kTransBlack_ColorInput;
    } else if (covIsSolidWhite) {
        desc->fCoverageInput = Desc::kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeCoverage) {
        desc->fCoverageInput = Desc::kUniform_ColorInput;
    } else {
        desc->fCoverageInput = Desc::kAttribute_ColorInput;
    }

    int lastEnabledStage = -1;

    if (!skipCoverage && (desc->fAttribBindings & GrDrawState::kEdge_AttribBindingsBit)) {
        desc->fVertexEdgeType = drawState.getVertexEdgeType();
        desc->fDiscardIfOutsideEdge = drawState.getStencil().doesWrite();
    } else {
        // Use canonical values when edge-aa is not enabled to avoid program cache misses.
        desc->fVertexEdgeType = GrDrawState::kHairLine_EdgeType;
        desc->fDiscardIfOutsideEdge = false;
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {

        bool skip = s < drawState.getFirstCoverageStage() ? skipColor : skipCoverage;
        if (!skip && drawState.isStageEnabled(s)) {
            lastEnabledStage = s;
            const GrEffectRef& effect = *drawState.getStage(s).getEffect();
            const GrBackendEffectFactory& factory = effect->getFactory();
            desc->fEffectKeys[s] = factory.glEffectKey(drawState.getStage(s), gpu->glCaps());
        } else {
            desc->fEffectKeys[s] = 0;
        }
    }

    desc->fDualSrcOutput = Desc::kNone_DualSrcOutput;

    // Currently the experimental GS will only work with triangle prims (and it doesn't do anything
    // other than pass through values from the VS to the FS anyway).
#if GR_GL_EXPERIMENTAL_GS
#if 0
    desc->fExperimentalGS = gpu->getCaps().geometryShaderSupport();
#else
    desc->fExperimentalGS = false;
#endif
#endif

    // We want to avoid generating programs with different "first cov stage" values when they would
    // compute the same result. We set field in the desc to kNumStages when either there are no
    // coverage stages or the distinction between coverage and color is immaterial.
    int firstCoverageStage = GrDrawState::kNumStages;
    desc->fFirstCoverageStage = GrDrawState::kNumStages;
    bool hasCoverage = drawState.getFirstCoverageStage() <= lastEnabledStage;
    if (hasCoverage) {
        firstCoverageStage = drawState.getFirstCoverageStage();
    }

    // other coverage inputs
    if (!hasCoverage) {
        hasCoverage = requiresAttributeCoverage ||
                      (desc->fAttribBindings & GrDrawState::kEdge_AttribBindingsBit);
    }

    if (hasCoverage) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != desc->fColorFilterXfermode) {
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        if (gpu->getCaps().dualSourceBlendingSupport() &&
            !(blendOpts & (GrDrawState::kEmitCoverage_BlendOptFlag |
                           GrDrawState::kCoverageAsAlpha_BlendOptFlag))) {
            if (kZero_GrBlendCoeff == dstCoeff) {
                // write the coverage value to second color
                desc->fDualSrcOutput =  Desc::kCoverage_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSA_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = Desc::kCoverageISA_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSC_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = Desc::kCoverageISC_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            }
        }
    }

    desc->fPositionAttributeIndex = drawState.getAttribIndex(GrDrawState::kPosition_AttribIndex);
    if (requiresAttributeColors) {
        desc->fColorAttributeIndex = drawState.getAttribIndex(GrDrawState::kColor_AttribIndex);
    } else {
        desc->fColorAttributeIndex = GrDrawState::kColorOverrideAttribIndexValue;
    }
    if (requiresAttributeCoverage) {
        desc->fCoverageAttributeIndex = drawState.getAttribIndex(GrDrawState::kCoverage_AttribIndex);
    } else {
        desc->fCoverageAttributeIndex = GrDrawState::kCoverageOverrideAttribIndexValue;
    }
    if (desc->fAttribBindings & GrDrawState::kEdge_AttribBindingsBit) {
        desc->fEdgeAttributeIndex = drawState.getAttribIndex(GrDrawState::kEdge_AttribIndex);
    }
    if (GrDrawState::AttributesBindExplicitTexCoords(desc->fAttribBindings)) {
        desc->fTexCoordAttributeIndex = drawState.getAttribIndex(GrDrawState::kTexCoord_AttribIndex);
    }

#if GR_DEBUG
    // verify valid vertex attribute state
    const GrVertexAttrib* vertexAttribs = drawState.getVertexAttribs();
    GrAssert(desc->fPositionAttributeIndex < GrDrawState::kVertexAttribCnt);
    GrAssert(kAttribLayouts[vertexAttribs[desc->fPositionAttributeIndex].fType].fCount == 2);
    if (requiresAttributeColors) {
        GrAssert(desc->fColorAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(kAttribLayouts[vertexAttribs[desc->fColorAttributeIndex].fType].fCount == 4);
    }
    if (requiresAttributeCoverage) {
        GrAssert(desc->fCoverageAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(kAttribLayouts[vertexAttribs[desc->fCoverageAttributeIndex].fType].fCount == 4);
    }
    if (desc->fAttribBindings & GrDrawState::kEdge_AttribBindingsBit) {
        GrAssert(desc->fEdgeAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(kAttribLayouts[vertexAttribs[desc->fEdgeAttributeIndex].fType].fCount == 4);
     }
    if (GrDrawState::AttributesBindExplicitTexCoords(desc->fAttribBindings)) {
        GrAssert(desc->fTexCoordAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(kAttribLayouts[vertexAttribs[desc->fTexCoordAttributeIndex].fType].fCount == 2);
    }
#endif
}

GrGLProgram* GrGLProgram::Create(const GrGLContext& gl,
                                 const Desc& desc,
                                 const GrEffectStage* stages[]) {
    GrGLProgram* program = SkNEW_ARGS(GrGLProgram, (gl, desc, stages));
    if (!program->succeeded()) {
        delete program;
        program = NULL;
    }
    return program;
}

GrGLProgram::GrGLProgram(const GrGLContext& gl,
                         const Desc& desc,
                         const GrEffectStage* stages[])
: fContext(gl)
, fUniformManager(gl) {
    fDesc = desc;
    fVShaderID = 0;
    fGShaderID = 0;
    fFShaderID = 0;
    fProgramID = 0;

    fColor = GrColor_ILLEGAL;
    fColorFilterColor = GrColor_ILLEGAL;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        fEffects[s] = NULL;
    }

    this->genProgram(stages);
}

GrGLProgram::~GrGLProgram() {
    if (fVShaderID) {
        GL_CALL(DeleteShader(fVShaderID));
    }
    if (fGShaderID) {
        GL_CALL(DeleteShader(fGShaderID));
    }
    if (fFShaderID) {
        GL_CALL(DeleteShader(fFShaderID));
    }
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }

    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        delete fEffects[i];
    }
}

void GrGLProgram::abandon() {
    fVShaderID = 0;
    fGShaderID = 0;
    fFShaderID = 0;
    fProgramID = 0;
}

void GrGLProgram::overrideBlend(GrBlendCoeff* srcCoeff,
                                GrBlendCoeff* dstCoeff) const {
    switch (fDesc.fDualSrcOutput) {
        case Desc::kNone_DualSrcOutput:
            break;
        // the prog will write a coverage value to the secondary
        // output and the dst is blended by one minus that value.
        case Desc::kCoverage_DualSrcOutput:
        case Desc::kCoverageISA_DualSrcOutput:
        case Desc::kCoverageISC_DualSrcOutput:
        *dstCoeff = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
        break;
        default:
            GrCrash("Unexpected dual source blend output");
            break;
    }
}

namespace {

// given two blend coeffecients determine whether the src
// and/or dst computation can be omitted.
inline void need_blend_inputs(SkXfermode::Coeff srcCoeff,
                              SkXfermode::Coeff dstCoeff,
                              bool* needSrcValue,
                              bool* needDstValue) {
    if (SkXfermode::kZero_Coeff == srcCoeff) {
        switch (dstCoeff) {
            // these all read the src
            case SkXfermode::kSC_Coeff:
            case SkXfermode::kISC_Coeff:
            case SkXfermode::kSA_Coeff:
            case SkXfermode::kISA_Coeff:
                *needSrcValue = true;
                break;
            default:
                *needSrcValue = false;
                break;
        }
    } else {
        *needSrcValue = true;
    }
    if (SkXfermode::kZero_Coeff == dstCoeff) {
        switch (srcCoeff) {
            // these all read the dst
            case SkXfermode::kDC_Coeff:
            case SkXfermode::kIDC_Coeff:
            case SkXfermode::kDA_Coeff:
            case SkXfermode::kIDA_Coeff:
                *needDstValue = true;
                break;
            default:
                *needDstValue = false;
                break;
        }
    } else {
        *needDstValue = true;
    }
}

/**
 * Create a blend_coeff * value string to be used in shader code. Sets empty
 * string if result is trivially zero.
 */
inline void blend_term_string(SkString* str, SkXfermode::Coeff coeff,
                       const char* src, const char* dst,
                       const char* value) {
    switch (coeff) {
    case SkXfermode::kZero_Coeff:    /** 0 */
        *str = "";
        break;
    case SkXfermode::kOne_Coeff:     /** 1 */
        *str = value;
        break;
    case SkXfermode::kSC_Coeff:
        str->printf("(%s * %s)", src, value);
        break;
    case SkXfermode::kISC_Coeff:
        str->printf("((%s - %s) * %s)", GrGLSLOnesVecf(4), src, value);
        break;
    case SkXfermode::kDC_Coeff:
        str->printf("(%s * %s)", dst, value);
        break;
    case SkXfermode::kIDC_Coeff:
        str->printf("((%s - %s) * %s)", GrGLSLOnesVecf(4), dst, value);
        break;
    case SkXfermode::kSA_Coeff:      /** src alpha */
        str->printf("(%s.a * %s)", src, value);
        break;
    case SkXfermode::kISA_Coeff:     /** inverse src alpha (i.e. 1 - sa) */
        str->printf("((1.0 - %s.a) * %s)", src, value);
        break;
    case SkXfermode::kDA_Coeff:      /** dst alpha */
        str->printf("(%s.a * %s)", dst, value);
        break;
    case SkXfermode::kIDA_Coeff:     /** inverse dst alpha (i.e. 1 - da) */
        str->printf("((1.0 - %s.a) * %s)", dst, value);
        break;
    default:
        GrCrash("Unexpected xfer coeff.");
        break;
    }
}
/**
 * Adds a line to the fragment shader code which modifies the color by
 * the specified color filter.
 */
void add_color_filter(SkString* fsCode, const char * outputVar,
                      SkXfermode::Coeff uniformCoeff,
                      SkXfermode::Coeff colorCoeff,
                      const char* filterColor,
                      const char* inColor) {
    SkString colorStr, constStr;
    blend_term_string(&colorStr, colorCoeff, filterColor, inColor, inColor);
    blend_term_string(&constStr, uniformCoeff, filterColor, inColor, filterColor);

    fsCode->appendf("\t%s = ", outputVar);
    GrGLSLAdd4f(fsCode, colorStr.c_str(), constStr.c_str());
    fsCode->append(";\n");
}
}

bool GrGLProgram::genEdgeCoverage(SkString* coverageVar,
                                  GrGLShaderBuilder* builder) const {
    if (fDesc.fAttribBindings & GrDrawState::kEdge_AttribBindingsBit) {
        const char *vsName, *fsName;
        builder->addVarying(kVec4f_GrSLType, "Edge", &vsName, &fsName);
        builder->fVSAttrs.push_back().set(kVec4f_GrSLType,
                                          GrGLShaderVar::kAttribute_TypeModifier,
                                          EDGE_ATTR_NAME);
        builder->fVSCode.appendf("\t%s = " EDGE_ATTR_NAME ";\n", vsName);
        switch (fDesc.fVertexEdgeType) {
        case GrDrawState::kHairLine_EdgeType:
            builder->fFSCode.appendf("\tfloat edgeAlpha = abs(dot(vec3(%s.xy,1), %s.xyz));\n", builder->fragmentPosition(), fsName);
            builder->fFSCode.append("\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            break;
        case GrDrawState::kQuad_EdgeType:
            builder->fFSCode.append("\tfloat edgeAlpha;\n");
            // keep the derivative instructions outside the conditional
            builder->fFSCode.appendf("\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fFSCode.appendf("\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fFSCode.appendf("\tif (%s.z > 0.0 && %s.w > 0.0) {\n", fsName, fsName);
            // today we know z and w are in device space. We could use derivatives
            builder->fFSCode.appendf("\t\tedgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);\n", fsName, fsName);
            builder->fFSCode.append ("\t} else {\n");
            builder->fFSCode.appendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                     "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                     fsName, fsName);
            builder->fFSCode.appendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName, fsName);
            builder->fFSCode.append("\t\tedgeAlpha = clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);\n"
                                    "\t}\n");
            if (kES2_GrGLBinding == fContext.info().binding()) {
                builder->fHeader.printf("#extension GL_OES_standard_derivatives: enable\n");
            }
            break;
        case GrDrawState::kHairQuad_EdgeType:
            builder->fFSCode.appendf("\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fFSCode.appendf("\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fFSCode.appendf("\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                     "\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                     fsName, fsName);
            builder->fFSCode.appendf("\tfloat edgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName, fsName);
            builder->fFSCode.append("\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / dot(gF, gF));\n");
            builder->fFSCode.append("\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            if (kES2_GrGLBinding == fContext.info().binding()) {
                builder->fHeader.printf("#extension GL_OES_standard_derivatives: enable\n");
            }
            break;
        case GrDrawState::kCircle_EdgeType:
            builder->fFSCode.append("\tfloat edgeAlpha;\n");
            builder->fFSCode.appendf("\tfloat d = distance(%s.xy, %s.xy);\n", builder->fragmentPosition(), fsName);
            builder->fFSCode.appendf("\tfloat outerAlpha = smoothstep(d - 0.5, d + 0.5, %s.z);\n", fsName);
            builder->fFSCode.appendf("\tfloat innerAlpha = %s.w == 0.0 ? 1.0 : smoothstep(%s.w - 0.5, %s.w + 0.5, d);\n", fsName, fsName, fsName);
            builder->fFSCode.append("\tedgeAlpha = outerAlpha * innerAlpha;\n");
            break;
        case GrDrawState::kEllipse_EdgeType:
            builder->fFSCode.append("\tfloat edgeAlpha;\n");
            builder->fFSCode.appendf("\tvec2 offset = (%s.xy - %s.xy);\n", builder->fragmentPosition(), fsName);
            builder->fFSCode.appendf("\toffset.y *= %s.w;\n", fsName);
            builder->fFSCode.append("\tfloat d = length(offset);\n");
            builder->fFSCode.appendf("\tedgeAlpha = smoothstep(d - 0.5, d + 0.5, %s.z);\n", fsName);
            break;
        default:
            GrCrash("Unknown Edge Type!");
            break;
        }
        if (fDesc.fDiscardIfOutsideEdge) {
            builder->fFSCode.appendf("\tif (edgeAlpha <= 0.0) {\n\t\tdiscard;\n\t}\n");
        }
        *coverageVar = "edgeAlpha";
        return true;
    } else {
        coverageVar->reset();
        return false;
    }
}

void GrGLProgram::genInputColor(GrGLShaderBuilder* builder, SkString* inColor) {
    switch (fDesc.fColorInput) {
        case GrGLProgram::Desc::kAttribute_ColorInput: {
            builder->fVSAttrs.push_back().set(kVec4f_GrSLType,
                GrGLShaderVar::kAttribute_TypeModifier,
                COL_ATTR_NAME);
            const char *vsName, *fsName;
            builder->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
            builder->fVSCode.appendf("\t%s = " COL_ATTR_NAME ";\n", vsName);
            *inColor = fsName;
            } break;
        case GrGLProgram::Desc::kUniform_ColorInput: {
            const char* name;
            fUniformHandles.fColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                            kVec4f_GrSLType, "Color", &name);
            *inColor = name;
            break;
        }
        case GrGLProgram::Desc::kTransBlack_ColorInput:
            GrAssert(!"needComputedColor should be false.");
            break;
        case GrGLProgram::Desc::kSolidWhite_ColorInput:
            break;
        default:
            GrCrash("Unknown color type.");
            break;
    }
}

void GrGLProgram::genUniformCoverage(GrGLShaderBuilder* builder, SkString* inOutCoverage) {
    const char* covUniName;
    fUniformHandles.fCoverageUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                       kVec4f_GrSLType, "Coverage", &covUniName);
    if (inOutCoverage->size()) {
        builder->fFSCode.appendf("\tvec4 uniCoverage = %s * %s;\n",
                                  covUniName, inOutCoverage->c_str());
        *inOutCoverage = "uniCoverage";
    } else {
        *inOutCoverage = covUniName;
    }
}

namespace {
void gen_attribute_coverage(GrGLShaderBuilder* segments,
                            SkString* inOutCoverage) {
    segments->fVSAttrs.push_back().set(kVec4f_GrSLType,
                                       GrGLShaderVar::kAttribute_TypeModifier,
                                       COV_ATTR_NAME);
    const char *vsName, *fsName;
    segments->addVarying(kVec4f_GrSLType, "Coverage", &vsName, &fsName);
    segments->fVSCode.appendf("\t%s = " COV_ATTR_NAME ";\n", vsName);
    if (inOutCoverage->size()) {
        segments->fFSCode.appendf("\tvec4 attrCoverage = %s * %s;\n",
                                  fsName, inOutCoverage->c_str());
        *inOutCoverage = "attrCoverage";
    } else {
        *inOutCoverage = fsName;
    }
}
}

void GrGLProgram::genGeometryShader(GrGLShaderBuilder* segments) const {
#if GR_GL_EXPERIMENTAL_GS
    if (fDesc.fExperimentalGS) {
        GrAssert(fContext.info().glslGeneration() >= k150_GrGLSLGeneration);
        segments->fGSHeader.append("layout(triangles) in;\n"
                                   "layout(triangle_strip, max_vertices = 6) out;\n");
        segments->fGSCode.append("\tfor (int i = 0; i < 3; ++i) {\n"
                                 "\t\tgl_Position = gl_in[i].gl_Position;\n");
        if (fDesc.fEmitsPointSize) {
            segments->fGSCode.append("\t\tgl_PointSize = 1.0;\n");
        }
        GrAssert(segments->fGSInputs.count() == segments->fGSOutputs.count());
        int count = segments->fGSInputs.count();
        for (int i = 0; i < count; ++i) {
            segments->fGSCode.appendf("\t\t%s = %s[i];\n",
                                      segments->fGSOutputs[i].getName().c_str(),
                                      segments->fGSInputs[i].getName().c_str());
        }
        segments->fGSCode.append("\t\tEmitVertex();\n"
                                 "\t}\n"
                                 "\tEndPrimitive();\n");
    }
#endif
}

const char* GrGLProgram::adjustInColor(const SkString& inColor) const {
    if (inColor.size()) {
          return inColor.c_str();
    } else {
        if (Desc::kSolidWhite_ColorInput == fDesc.fColorInput) {
            return GrGLSLOnesVecf(4);
        } else {
            return GrGLSLZerosVecf(4);
        }
    }
}

namespace {
// prints a shader using params similar to glShaderSource
void print_shader(GrGLint stringCnt,
                  const GrGLchar** strings,
                  GrGLint* stringLengths) {
    for (int i = 0; i < stringCnt; ++i) {
        if (NULL == stringLengths || stringLengths[i] < 0) {
            GrPrintf(strings[i]);
        } else {
            GrPrintf("%.*s", stringLengths[i], strings[i]);
        }
    }
}

// Compiles a GL shader, returns shader ID or 0 if failed params have same meaning as glShaderSource
GrGLuint compile_shader(const GrGLContext& gl,
                        GrGLenum type,
                        int stringCnt,
                        const char** strings,
                        int* stringLengths) {
    SK_TRACE_EVENT1("GrGLProgram::CompileShader",
                    "stringCount", SkStringPrintf("%i", stringCnt).c_str());

    GrGLuint shader;
    GR_GL_CALL_RET(gl.interface(), shader, CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    const GrGLInterface* gli = gl.interface();
    GrGLint compiled = GR_GL_INIT_ZERO;
    GR_GL_CALL(gli, ShaderSource(shader, stringCnt, strings, stringLengths));
    GR_GL_CALL(gli, CompileShader(shader));
    GR_GL_CALL(gli, GetShaderiv(shader, GR_GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GR_GL_CALL(gli, GetShaderiv(shader, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround bug in chrome cmd buffer
            // param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GR_GL_CALL(gli, GetShaderInfoLog(shader, infoLen+1,
                                             &length, (char*)log.get()));
            print_shader(stringCnt, strings, stringLengths);
            GrPrintf("\n%s", log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GR_GL_CALL(gli, DeleteShader(shader));
        return 0;
    }
    return shader;
}

// helper version of above for when shader is already flattened into a single SkString
GrGLuint compile_shader(const GrGLContext& gl, GrGLenum type, const SkString& shader) {
    const GrGLchar* str = shader.c_str();
    int length = shader.size();
    return compile_shader(gl, type, 1, &str, &length);
}

}

// compiles all the shaders from builder and stores the shader IDs
bool GrGLProgram::compileShaders(const GrGLShaderBuilder& builder) {

    SkString shader;

    builder.getShader(GrGLShaderBuilder::kVertex_ShaderType, &shader);
    if (c_PrintShaders) {
        GrPrintf(shader.c_str());
        GrPrintf("\n");
    }

    if (!(fVShaderID = compile_shader(fContext, GR_GL_VERTEX_SHADER, shader))) {
        return false;
    }

    if (builder.fUsesGS) {
        builder.getShader(GrGLShaderBuilder::kGeometry_ShaderType, &shader);
        if (c_PrintShaders) {
            GrPrintf(shader.c_str());
            GrPrintf("\n");
        }
        if (!(fGShaderID = compile_shader(fContext, GR_GL_GEOMETRY_SHADER, shader))) {
            return false;
        }
    } else {
        fGShaderID = 0;
    }

    builder.getShader(GrGLShaderBuilder::kFragment_ShaderType, &shader);
    if (c_PrintShaders) {
        GrPrintf(shader.c_str());
        GrPrintf("\n");
    }
    if (!(fFShaderID = compile_shader(fContext, GR_GL_FRAGMENT_SHADER, shader))) {
        return false;
    }

    return true;
}

bool GrGLProgram::genProgram(const GrEffectStage* stages[]) {
    GrAssert(0 == fProgramID);

    GrGLShaderBuilder builder(fContext.info(), fUniformManager);
    const GrAttribBindings& attribBindings = fDesc.fAttribBindings;

#if GR_GL_EXPERIMENTAL_GS
    builder.fUsesGS = fDesc.fExperimentalGS;
#endif

    SkXfermode::Coeff colorCoeff, uniformCoeff;
    // The rest of transfer mode color filters have not been implemented
    if (fDesc.fColorFilterXfermode < SkXfermode::kCoeffModesCnt) {
        GR_DEBUGCODE(bool success =)
            SkXfermode::ModeAsCoeff(static_cast<SkXfermode::Mode>
                                    (fDesc.fColorFilterXfermode),
                                    &uniformCoeff, &colorCoeff);
        GR_DEBUGASSERT(success);
    } else {
        colorCoeff = SkXfermode::kOne_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
    }

    // no need to do the color filter if coverage is 0. The output color is scaled by the coverage.
    // All the dual source outputs are scaled by the coverage as well.
    if (Desc::kTransBlack_ColorInput == fDesc.fCoverageInput) {
        colorCoeff = SkXfermode::kZero_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
    }

    // If we know the final color is going to be all zeros then we can
    // simplify the color filter coefficients. needComputedColor will then
    // come out false below.
    if (Desc::kTransBlack_ColorInput == fDesc.fColorInput) {
        colorCoeff = SkXfermode::kZero_Coeff;
        if (SkXfermode::kDC_Coeff == uniformCoeff ||
            SkXfermode::kDA_Coeff == uniformCoeff) {
            uniformCoeff = SkXfermode::kZero_Coeff;
        } else if (SkXfermode::kIDC_Coeff == uniformCoeff ||
                   SkXfermode::kIDA_Coeff == uniformCoeff) {
            uniformCoeff = SkXfermode::kOne_Coeff;
        }
    }

    bool needColorFilterUniform;
    bool needComputedColor;
    need_blend_inputs(uniformCoeff, colorCoeff,
                      &needColorFilterUniform, &needComputedColor);

    // the dual source output has no canonical var name, have to
    // declare an output, which is incompatible with gl_FragColor/gl_FragData.
    bool dualSourceOutputWritten = false;
    builder.fHeader.append(GrGetGLSLVersionDecl(fContext.info().binding(),
                                                fContext.info().glslGeneration()));

    GrGLShaderVar colorOutput;
    bool isColorDeclared = GrGLSLSetupFSColorOuput(fContext.info().glslGeneration(),
                                                   declared_color_output_name(),
                                                   &colorOutput);
    if (isColorDeclared) {
        builder.fFSOutputs.push_back(colorOutput);
    }

    const char* viewMName;
    fUniformHandles.fViewMatrixUni = builder.addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                                        kMat33f_GrSLType, "ViewM", &viewMName);


    builder.fVSCode.appendf("\tvec3 pos3 = %s * vec3(%s, 1);\n"
                            "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n",
                            viewMName, builder.positionAttribute().getName().c_str());

    // incoming color to current stage being processed.
    SkString inColor;

    if (needComputedColor) {
        this->genInputColor(&builder, &inColor);
    }

    // we output point size in the GS if present
    if (fDesc.fEmitsPointSize && !builder.fUsesGS){
        builder.fVSCode.append("\tgl_PointSize = 1.0;\n");
    }

    // add texture coordinates that are used to the list of vertex attr decls
    if (GrDrawState::AttributesBindExplicitTexCoords(attribBindings)) {
        builder.fVSAttrs.push_back().set(kVec2f_GrSLType,
            GrGLShaderVar::kAttribute_TypeModifier,
            TEX_ATTR_NAME);
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the final color

    // if we have color stages string them together, feeding the output color
    // of each to the next and generating code for each stage.
    if (needComputedColor) {
        SkString outColor;
        for (int s = 0; s < fDesc.fFirstCoverageStage; ++s) {
            if (GrGLEffect::kNoEffectKey != fDesc.fEffectKeys[s]) {
                // create var to hold stage result
                outColor = "color";
                outColor.appendS32(s);
                builder.fFSCode.appendf("\tvec4 %s;\n", outColor.c_str());

                const char* inCoords;
                // figure out what our input coords are
                if (!GrDrawState::StageBindsExplicitTexCoords(attribBindings, s)) {
                    inCoords = builder.positionAttribute().c_str();
                } else {
                    // must have input tex coordinates if stage is enabled.
                    inCoords = TEX_ATTR_NAME;
                }

                builder.setCurrentStage(s);
                fEffects[s] = builder.createAndEmitGLEffect(*stages[s],
                                                            fDesc.fEffectKeys[s],
                                                            inColor.size() ? inColor.c_str() : NULL,
                                                            outColor.c_str(),
                                                            inCoords,
                                                            &fUniformHandles.fSamplerUnis[s]);
                builder.setNonStage();
                inColor = outColor;
            }
        }
    }

    // if have all ones or zeros for the "dst" input to the color filter then we
    // may be able to make additional optimizations.
    if (needColorFilterUniform && needComputedColor && !inColor.size()) {
        GrAssert(Desc::kSolidWhite_ColorInput == fDesc.fColorInput);
        bool uniformCoeffIsZero = SkXfermode::kIDC_Coeff == uniformCoeff ||
                                  SkXfermode::kIDA_Coeff == uniformCoeff;
        if (uniformCoeffIsZero) {
            uniformCoeff = SkXfermode::kZero_Coeff;
            bool bogus;
            need_blend_inputs(SkXfermode::kZero_Coeff, colorCoeff,
                              &needColorFilterUniform, &bogus);
        }
    }
    const char* colorFilterColorUniName = NULL;
    if (needColorFilterUniform) {
        fUniformHandles.fColorFilterUni = builder.addUniform(
                                                        GrGLShaderBuilder::kFragment_ShaderType,
                                                        kVec4f_GrSLType, "FilterColor",
                                                        &colorFilterColorUniName);
    }
    bool wroteFragColorZero = false;
    if (SkXfermode::kZero_Coeff == uniformCoeff &&
        SkXfermode::kZero_Coeff == colorCoeff) {
        builder.fFSCode.appendf("\t%s = %s;\n",
                                colorOutput.getName().c_str(),
                                GrGLSLZerosVecf(4));
        wroteFragColorZero = true;
    } else if (SkXfermode::kDst_Mode != fDesc.fColorFilterXfermode) {
        builder.fFSCode.append("\tvec4 filteredColor;\n");
        const char* color = adjustInColor(inColor);
        add_color_filter(&builder.fFSCode, "filteredColor", uniformCoeff,
                       colorCoeff, colorFilterColorUniName, color);
        inColor = "filteredColor";
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage (coverage stages and edge aa)

    SkString inCoverage;
    bool coverageIsZero = Desc::kTransBlack_ColorInput == fDesc.fCoverageInput;
    // we don't need to compute coverage at all if we know the final shader
    // output will be zero and we don't have a dual src blend output.
    if (!wroteFragColorZero || Desc::kNone_DualSrcOutput != fDesc.fDualSrcOutput) {

        if (!coverageIsZero) {
            bool inCoverageIsScalar  = this->genEdgeCoverage(&inCoverage, &builder);

            switch (fDesc.fCoverageInput) {
                case Desc::kSolidWhite_ColorInput:
                    // empty string implies solid white
                    break;
                case Desc::kAttribute_ColorInput:
                    gen_attribute_coverage(&builder, &inCoverage);
                    inCoverageIsScalar = false;
                    break;
                case Desc::kUniform_ColorInput:
                    this->genUniformCoverage(&builder, &inCoverage);
                    inCoverageIsScalar = false;
                    break;
                default:
                    GrCrash("Unexpected input coverage.");
            }

            SkString outCoverage;
            const int& startStage = fDesc.fFirstCoverageStage;
            for (int s = startStage; s < GrDrawState::kNumStages; ++s) {
                if (fDesc.fEffectKeys[s]) {
                    // create var to hold stage output
                    outCoverage = "coverage";
                    outCoverage.appendS32(s);
                    builder.fFSCode.appendf("\tvec4 %s;\n", outCoverage.c_str());

                    const char* inCoords;
                    // figure out what our input coords are
                    if (!GrDrawState::StageBindsExplicitTexCoords(attribBindings, s)) {
                        inCoords = builder.positionAttribute().c_str();
                    } else {
                        // must have input tex coordinates if stage is
                        // enabled.
                        inCoords = TEX_ATTR_NAME;
                    }

                    // stages don't know how to deal with a scalar input. (Maybe they should. We
                    // could pass a GrGLShaderVar)
                    if (inCoverageIsScalar) {
                        builder.fFSCode.appendf("\tvec4 %s4 = vec4(%s);\n",
                                                inCoverage.c_str(), inCoverage.c_str());
                        inCoverage.append("4");
                    }
                    builder.setCurrentStage(s);
                    fEffects[s] = builder.createAndEmitGLEffect(
                                                    *stages[s],
                                                    fDesc.fEffectKeys[s],
                                                    inCoverage.size() ? inCoverage.c_str() : NULL,
                                                    outCoverage.c_str(),
                                                    inCoords,
                                                    &fUniformHandles.fSamplerUnis[s]);
                    builder.setNonStage();
                    inCoverage = outCoverage;
                }
            }
        }

        if (Desc::kNone_DualSrcOutput != fDesc.fDualSrcOutput) {
            builder.fFSOutputs.push_back().set(kVec4f_GrSLType,
                                               GrGLShaderVar::kOut_TypeModifier,
                                               dual_source_output_name());
            bool outputIsZero = coverageIsZero;
            SkString coeff;
            if (!outputIsZero &&
                Desc::kCoverage_DualSrcOutput != fDesc.fDualSrcOutput && !wroteFragColorZero) {
                if (!inColor.size()) {
                    outputIsZero = true;
                } else {
                    if (Desc::kCoverageISA_DualSrcOutput == fDesc.fDualSrcOutput) {
                        coeff.printf("(1 - %s.a)", inColor.c_str());
                    } else {
                        coeff.printf("(vec4(1,1,1,1) - %s)", inColor.c_str());
                    }
                }
            }
            if (outputIsZero) {
                builder.fFSCode.appendf("\t%s = %s;\n",
                                        dual_source_output_name(),
                                        GrGLSLZerosVecf(4));
            } else {
                builder.fFSCode.appendf("\t%s =", dual_source_output_name());
                GrGLSLModulate4f(&builder.fFSCode, coeff.c_str(), inCoverage.c_str());
                builder.fFSCode.append(";\n");
            }
            dualSourceOutputWritten = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    if (!wroteFragColorZero) {
        if (coverageIsZero) {
            builder.fFSCode.appendf("\t%s = %s;\n",
                                    colorOutput.getName().c_str(),
                                    GrGLSLZerosVecf(4));
        } else {
            builder.fFSCode.appendf("\t%s = ", colorOutput.getName().c_str());
            GrGLSLModulate4f(&builder.fFSCode, inColor.c_str(), inCoverage.c_str());
            builder.fFSCode.append(";\n");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // insert GS
#if GR_DEBUG
    this->genGeometryShader(&builder);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // compile and setup attribs and unis

    if (!this->compileShaders(builder)) {
        return false;
    }

    if (!this->bindOutputsAttribsAndLinkProgram(builder,
                                                isColorDeclared,
                                                dualSourceOutputWritten)) {
        return false;
    }

    builder.finished(fProgramID);
    this->initSamplerUniforms();
    fUniformHandles.fRTHeightUni = builder.getRTHeightUniform();

    return true;
}

bool GrGLProgram::bindOutputsAttribsAndLinkProgram(const GrGLShaderBuilder& builder,
                                                   bool bindColorOut,
                                                   bool bindDualSrcOut) {
    GL_CALL_RET(fProgramID, CreateProgram());
    if (!fProgramID) {
        return false;
    }

    GL_CALL(AttachShader(fProgramID, fVShaderID));
    if (fGShaderID) {
        GL_CALL(AttachShader(fProgramID, fGShaderID));
    }
    GL_CALL(AttachShader(fProgramID, fFShaderID));

    if (bindColorOut) {
        GL_CALL(BindFragDataLocation(fProgramID, 0, declared_color_output_name()));
    }
    if (bindDualSrcOut) {
        GL_CALL(BindFragDataLocationIndexed(fProgramID, 0, 1, dual_source_output_name()));
    }

    // Bind the attrib locations to same values for all shaders
    GL_CALL(BindAttribLocation(fProgramID,
                               fDesc.fPositionAttributeIndex,
                               builder.positionAttribute().c_str()));
    GL_CALL(BindAttribLocation(fProgramID, fDesc.fColorAttributeIndex, COL_ATTR_NAME));
    GL_CALL(BindAttribLocation(fProgramID, fDesc.fCoverageAttributeIndex, COV_ATTR_NAME));

    if (fDesc.fAttribBindings & GrDrawState::kEdge_AttribBindingsBit) {
        GL_CALL(BindAttribLocation(fProgramID, fDesc.fEdgeAttributeIndex, EDGE_ATTR_NAME));
    }
    if (GrDrawState::AttributesBindExplicitTexCoords(fDesc.fAttribBindings)) {
        GL_CALL(BindAttribLocation(fProgramID, fDesc.fTexCoordAttributeIndex, TEX_ATTR_NAME));
    }

    GL_CALL(LinkProgram(fProgramID));

    GrGLint linked = GR_GL_INIT_ZERO;
    GL_CALL(GetProgramiv(fProgramID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(fProgramID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramInfoLog(fProgramID,
                                      infoLen+1,
                                      &length,
                                      (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GL_CALL(DeleteProgram(fProgramID));
        fProgramID = 0;
        return false;
    }
    return true;
}

void GrGLProgram::initSamplerUniforms() {
    GL_CALL(UseProgram(fProgramID));
    // We simply bind the uniforms to successive texture units beginning at 0. setData() assumes this
    // behavior.
    GrGLint texUnitIdx = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        int numSamplers = fUniformHandles.fSamplerUnis[s].count();
        for (int u = 0; u < numSamplers; ++u) {
            UniformHandle handle = fUniformHandles.fSamplerUnis[s][u];
            if (GrGLUniformManager::kInvalidUniformHandle != handle) {
                fUniformManager.setSampler(handle, texUnitIdx);
                ++texUnitIdx;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(GrGpuGL* gpu,
                          GrColor color,
                          GrColor coverage,
                          SharedGLState* sharedState) {
    const GrDrawState& drawState = gpu->getDrawState();

    this->setColor(drawState, color, sharedState);
    this->setCoverage(drawState, coverage, sharedState);
    this->setMatrixAndRenderTargetHeight(drawState);

    // Setup the SkXfermode::Mode-based colorfilter uniform if necessary
    if (GrGLUniformManager::kInvalidUniformHandle != fUniformHandles.fColorFilterUni &&
        fColorFilterColor != drawState.getColorFilterColor()) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(drawState.getColorFilterColor(), c);
        fUniformManager.set4fv(fUniformHandles.fColorFilterUni, 0, 1, c);
        fColorFilterColor = drawState.getColorFilterColor();
    }

    GrGLint texUnitIdx = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (NULL != fEffects[s]) {
            const GrEffectStage& stage = drawState.getStage(s);
            GrAssert(NULL != stage.getEffect());
            fEffects[s]->setData(fUniformManager, stage);
            int numSamplers = fUniformHandles.fSamplerUnis[s].count();
            for (int u = 0; u < numSamplers; ++u) {
                UniformHandle handle = fUniformHandles.fSamplerUnis[s][u];
                if (GrGLUniformManager::kInvalidUniformHandle != handle) {
                    const GrTextureAccess& access = (*stage.getEffect())->textureAccess(u);
                    GrGLTexture* texture = static_cast<GrGLTexture*>(access.getTexture());
                    gpu->bindTexture(texUnitIdx, access.getParams(), texture);
                    ++texUnitIdx;
                }
            }
        }
    }
}

void GrGLProgram::setColor(const GrDrawState& drawState,
                           GrColor color,
                           SharedGLState* sharedState) {
    if (!(drawState.getAttribBindings() & GrDrawState::kColor_AttribBindingsBit)) {
        switch (fDesc.fColorInput) {
            case GrGLProgram::Desc::kAttribute_ColorInput:
                if (sharedState->fConstAttribColor != color) {
                    // OpenGL ES only supports the float varieties of glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    GL_CALL(VertexAttrib4fv(fDesc.fColorAttributeIndex, c));
                    sharedState->fConstAttribColor = color;
                }
                break;
            case GrGLProgram::Desc::kUniform_ColorInput:
                if (fColor != color) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    GrAssert(GrGLUniformManager::kInvalidUniformHandle !=
                             fUniformHandles.fColorUni);
                    fUniformManager.set4fv(fUniformHandles.fColorUni, 0, 1, c);
                    fColor = color;
                }
                break;
            case GrGLProgram::Desc::kSolidWhite_ColorInput:
            case GrGLProgram::Desc::kTransBlack_ColorInput:
                break;
            default:
                GrCrash("Unknown color type.");
        }
    }
}

void GrGLProgram::setCoverage(const GrDrawState& drawState,
                              GrColor coverage,
                              SharedGLState* sharedState) {
    if (!(drawState.getAttribBindings() & GrDrawState::kCoverage_AttribBindingsBit)) {
        switch (fDesc.fCoverageInput) {
            case Desc::kAttribute_ColorInput:
                if (sharedState->fConstAttribCoverage != coverage) {
                    // OpenGL ES only supports the float varieties of  glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    GL_CALL(VertexAttrib4fv(fDesc.fCoverageAttributeIndex, c));
                    sharedState->fConstAttribCoverage = coverage;
                }
                break;
            case Desc::kUniform_ColorInput:
                if (fCoverage != coverage) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    GrAssert(GrGLUniformManager::kInvalidUniformHandle !=
                             fUniformHandles.fCoverageUni);
                    fUniformManager.set4fv(fUniformHandles.fCoverageUni, 0, 1, c);
                    fCoverage = coverage;
                }
                break;
            case Desc::kSolidWhite_ColorInput:
            case Desc::kTransBlack_ColorInput:
                break;
            default:
                GrCrash("Unknown coverage type.");
        }
    }
}

void GrGLProgram::setMatrixAndRenderTargetHeight(const GrDrawState& drawState) {
    const GrRenderTarget* rt = drawState.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (GrGLUniformManager::kInvalidUniformHandle != fUniformHandles.fRTHeightUni &&
        fMatrixState.fRenderTargetSize.fHeight != size.fHeight) {
        fUniformManager.set1f(fUniformHandles.fRTHeightUni, SkIntToScalar(size.fHeight));
    }

    if (fMatrixState.fRenderTargetOrigin != rt->origin() ||
        !fMatrixState.fViewMatrix.cheapEqualTo(drawState.getViewMatrix()) ||
        fMatrixState.fRenderTargetSize != size) {
        SkMatrix m;
        if (kBottomLeft_GrSurfaceOrigin == rt->origin()) {
            m.setAll(
                SkIntToScalar(2) / size.fWidth, 0, -SK_Scalar1,
                0,-SkIntToScalar(2) / size.fHeight, SK_Scalar1,
            0, 0, SkMatrix::I()[8]);
        } else {
            m.setAll(
                SkIntToScalar(2) / size.fWidth, 0, -SK_Scalar1,
                0, SkIntToScalar(2) / size.fHeight,-SK_Scalar1,
            0, 0, SkMatrix::I()[8]);
        }
        m.setConcat(m, drawState.getViewMatrix());

        // ES doesn't allow you to pass true to the transpose param so we do our own transpose.
        GrGLfloat mt[]  = {
            SkScalarToFloat(m[SkMatrix::kMScaleX]),
            SkScalarToFloat(m[SkMatrix::kMSkewY]),
            SkScalarToFloat(m[SkMatrix::kMPersp0]),
            SkScalarToFloat(m[SkMatrix::kMSkewX]),
            SkScalarToFloat(m[SkMatrix::kMScaleY]),
            SkScalarToFloat(m[SkMatrix::kMPersp1]),
            SkScalarToFloat(m[SkMatrix::kMTransX]),
            SkScalarToFloat(m[SkMatrix::kMTransY]),
            SkScalarToFloat(m[SkMatrix::kMPersp2])
        };
        fUniformManager.setMatrix3f(fUniformHandles.fViewMatrixUni, mt);
        fMatrixState.fViewMatrix = drawState.getViewMatrix();
        fMatrixState.fRenderTargetSize = size;
        fMatrixState.fRenderTargetOrigin = rt->origin();
    }
}
