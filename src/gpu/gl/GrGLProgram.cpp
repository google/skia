/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrCustomStage.h"
#include "GrGLProgramStage.h"
#include "gl/GrGLShaderBuilder.h"
#include "GrGLShaderVar.h"
#include "GrProgramStageFactory.h"
#include "SkTrace.h"
#include "SkXfermode.h"

namespace {

enum {
    /// Used to mark a StageUniLocation field that should be bound
    /// to a uniform during getUniformLocationsAndInitCache().
    kUseUniform = 2000
};

}  // namespace

#define PRINT_SHADERS 0

typedef GrGLProgram::ProgramDesc::StageDesc StageDesc;

#define VIEW_MATRIX_NAME "uViewM"

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"
#define COV_ATTR_NAME "aCoverage"
#define EDGE_ATTR_NAME "aEdge"
#define COL_UNI_NAME "uColor"
#define COV_UNI_NAME "uCoverage"
#define COL_FILTER_UNI_NAME "uColorFilter"
#define COL_MATRIX_UNI_NAME "uColorMatrix"
#define COL_MATRIX_VEC_UNI_NAME "uColorMatrixVec"

namespace {
inline void tex_attr_name(int coordIdx, GrStringBuilder* s) {
    *s = "aTexCoord";
    s->appendS32(coordIdx);
}

inline const char* float_vector_type_str(int count) {
    return GrGLShaderVar::TypeString(GrSLFloatVectorType(count));
}

inline const char* vector_all_coords(int count) {
    static const char* ALL[] = {"ERROR", "", ".xy", ".xyz", ".xyzw"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(ALL));
    return ALL[count];
}

inline const char* all_ones_vec(int count) {
    static const char* ONESVEC[] = {"ERROR", "1.0", "vec2(1,1)",
                                    "vec3(1,1,1)", "vec4(1,1,1,1)"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(ONESVEC));
    return ONESVEC[count];
}

inline const char* all_zeros_vec(int count) {
    static const char* ZEROSVEC[] = {"ERROR", "0.0", "vec2(0,0)",
                                    "vec3(0,0,0)", "vec4(0,0,0,0)"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(ZEROSVEC));
    return ZEROSVEC[count];
}

inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }

inline void tex_matrix_name(int stage, GrStringBuilder* s) {
    *s = "uTexM";
    s->appendS32(stage);
}

inline void sampler_name(int stage, GrStringBuilder* s) {
    *s = "uSampler";
    s->appendS32(stage);
}

inline void tex_domain_name(int stage, GrStringBuilder* s) {
    *s = "uTexDom";
    s->appendS32(stage);
}
}

GrGLProgram::GrGLProgram() {
}

GrGLProgram::~GrGLProgram() {
}

void GrGLProgram::overrideBlend(GrBlendCoeff* srcCoeff,
                                GrBlendCoeff* dstCoeff) const {
    switch (fProgramDesc.fDualSrcOutput) {
        case ProgramDesc::kNone_DualSrcOutput:
            break;
        // the prog will write a coverage value to the secondary
        // output and the dst is blended by one minus that value.
        case ProgramDesc::kCoverage_DualSrcOutput:
        case ProgramDesc::kCoverageISA_DualSrcOutput:
        case ProgramDesc::kCoverageISC_DualSrcOutput:
        *dstCoeff = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
        break;
        default:
            GrCrash("Unexpected dual source blend output");
            break;
    }
}

// assigns modulation of two vars to an output var
// vars can be vec4s or floats (or one of each)
// result is always vec4
// if either var is "" then assign to the other var
// if both are "" then assign all ones
static inline void modulate_helper(const char* outputVar,
                                   const char* var0,
                                   const char* var1,
                                   GrStringBuilder* code) {
    GrAssert(NULL != outputVar);
    GrAssert(NULL != var0);
    GrAssert(NULL != var1);
    GrAssert(NULL != code);

    bool has0 = '\0' != *var0;
    bool has1 = '\0' != *var1;

    if (!has0 && !has1) {
        code->appendf("\t%s = %s;\n", outputVar, all_ones_vec(4));
    } else if (!has0) {
        code->appendf("\t%s = vec4(%s);\n", outputVar, var1);
    } else if (!has1) {
        code->appendf("\t%s = vec4(%s);\n", outputVar, var0);
    } else {
        code->appendf("\t%s = vec4(%s * %s);\n", outputVar, var0, var1);
    }
}

// assigns addition of two vars to an output var
// vars can be vec4s or floats (or one of each)
// result is always vec4
// if either var is "" then assign to the other var
// if both are "" then assign all zeros
static inline void add_helper(const char* outputVar,
                              const char* var0,
                              const char* var1,
                              GrStringBuilder* code) {
    GrAssert(NULL != outputVar);
    GrAssert(NULL != var0);
    GrAssert(NULL != var1);
    GrAssert(NULL != code);

    bool has0 = '\0' != *var0;
    bool has1 = '\0' != *var1;

    if (!has0 && !has1) {
        code->appendf("\t%s = %s;\n", outputVar, all_zeros_vec(4));
    } else if (!has0) {
        code->appendf("\t%s = vec4(%s);\n", outputVar, var1);
    } else if (!has1) {
        code->appendf("\t%s = vec4(%s);\n", outputVar, var0);
    } else {
        code->appendf("\t%s = vec4(%s + %s);\n", outputVar, var0, var1);
    }
}

// given two blend coeffecients determine whether the src
// and/or dst computation can be omitted.
static inline void needBlendInputs(SkXfermode::Coeff srcCoeff,
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
static void blendTermString(GrStringBuilder* str, SkXfermode::Coeff coeff,
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
        str->printf("((%s - %s) * %s)", all_ones_vec(4), src, value);
        break;
    case SkXfermode::kDC_Coeff:
        str->printf("(%s * %s)", dst, value);
        break;
    case SkXfermode::kIDC_Coeff:
        str->printf("((%s - %s) * %s)", all_ones_vec(4), dst, value);
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
static void addColorFilter(GrStringBuilder* fsCode, const char * outputVar,
                           SkXfermode::Coeff uniformCoeff,
                           SkXfermode::Coeff colorCoeff,
                           const char* inColor) {
    GrStringBuilder colorStr, constStr;
    blendTermString(&colorStr, colorCoeff, COL_FILTER_UNI_NAME,
                    inColor, inColor);
    blendTermString(&constStr, uniformCoeff, COL_FILTER_UNI_NAME,
                    inColor, COL_FILTER_UNI_NAME);

    add_helper(outputVar, colorStr.c_str(), constStr.c_str(), fsCode);
}
/**
 * Adds code to the fragment shader code which modifies the color by
 * the specified color matrix.
 */
static void addColorMatrix(GrStringBuilder* fsCode, const char * outputVar,
                           const char* inColor) {
    fsCode->appendf("\t%s = %s * vec4(%s.rgb / %s.a, %s.a) + %s;\n", outputVar, COL_MATRIX_UNI_NAME, inColor, inColor, inColor, COL_MATRIX_VEC_UNI_NAME);
    fsCode->appendf("\t%s.rgb *= %s.a;\n", outputVar, outputVar);
}

void GrGLProgram::genEdgeCoverage(const GrGLContextInfo& gl,
                                  GrVertexLayout layout,
                                  CachedData* programData,
                                  GrStringBuilder* coverageVar,
                                  GrGLShaderBuilder* segments) const {
    if (layout & GrDrawTarget::kEdge_VertexLayoutBit) {
        const char *vsName, *fsName;
        segments->addVarying(kVec4f_GrSLType, "Edge", &vsName, &fsName);
        segments->fVSAttrs.push_back().set(kVec4f_GrSLType,
            GrGLShaderVar::kAttribute_TypeModifier, EDGE_ATTR_NAME);
        segments->fVSCode.appendf("\t%s = " EDGE_ATTR_NAME ";\n", vsName);
        switch (fProgramDesc.fVertexEdgeType) {
        case GrDrawState::kHairLine_EdgeType:
            segments->fFSCode.appendf("\tfloat edgeAlpha = abs(dot(vec3(gl_FragCoord.xy,1), %s.xyz));\n", fsName);
            segments->fFSCode.append("\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            break;
        case GrDrawState::kQuad_EdgeType:
            segments->fFSCode.append("\tfloat edgeAlpha;\n");
            // keep the derivative instructions outside the conditional 
            segments->fFSCode.appendf("\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tif (%s.z > 0.0 && %s.w > 0.0) {\n", fsName, fsName);
            // today we know z and w are in device space. We could use derivatives
            segments->fFSCode.appendf("\t\tedgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);\n", fsName, fsName);
            segments->fFSCode.append ("\t} else {\n");
            segments->fFSCode.appendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                      "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                      fsName, fsName);
            segments->fFSCode.appendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName, fsName);
            segments->fFSCode.append("\t\tedgeAlpha = clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);\n"
                                      "\t}\n");
            if (kES2_GrGLBinding == gl.binding()) {
                segments->fHeader.printf("#extension GL_OES_standard_derivatives: enable\n");
            }
            break;
        case GrDrawState::kHairQuad_EdgeType:
            segments->fFSCode.appendf("\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                      "\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                      fsName, fsName);
            segments->fFSCode.appendf("\tfloat edgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName, fsName);
            segments->fFSCode.append("\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / dot(gF, gF));\n");
            segments->fFSCode.append("\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
            if (kES2_GrGLBinding == gl.binding()) {
                segments->fHeader.printf("#extension GL_OES_standard_derivatives: enable\n");
            }
            break;
        case GrDrawState::kCircle_EdgeType:
            segments->fFSCode.append("\tfloat edgeAlpha;\n");
            segments->fFSCode.appendf("\tfloat d = distance(gl_FragCoord.xy, %s.xy);\n", fsName);
            segments->fFSCode.appendf("\tfloat outerAlpha = smoothstep(d - 0.5, d + 0.5, %s.z);\n", fsName);
            segments->fFSCode.appendf("\tfloat innerAlpha = %s.w == 0.0 ? 1.0 : smoothstep(%s.w - 0.5, %s.w + 0.5, d);\n", fsName, fsName, fsName);
            segments->fFSCode.append("\tedgeAlpha = outerAlpha * innerAlpha;\n");
            break;
        default:
            GrCrash("Unknown Edge Type!");
            break;
        }
        *coverageVar = "edgeAlpha";
    } else {
        coverageVar->reset();
    }
}

namespace {

void genInputColor(GrGLProgram::ProgramDesc::ColorInput colorInput,
                   GrGLProgram::CachedData* programData,
                   GrGLShaderBuilder* segments,
                   GrStringBuilder* inColor) {
    switch (colorInput) {
        case GrGLProgram::ProgramDesc::kAttribute_ColorInput: {
            segments->fVSAttrs.push_back().set(kVec4f_GrSLType,
                GrGLShaderVar::kAttribute_TypeModifier,
                COL_ATTR_NAME);
            const char *vsName, *fsName;
            segments->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
            segments->fVSCode.appendf("\t%s = " COL_ATTR_NAME ";\n", vsName);
            *inColor = fsName;
            } break;
        case GrGLProgram::ProgramDesc::kUniform_ColorInput:
            segments->addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
                                 kVec4f_GrSLType, COL_UNI_NAME);
            programData->fUniLocations.fColorUni = kUseUniform;
            *inColor = COL_UNI_NAME;
            break;
        case GrGLProgram::ProgramDesc::kTransBlack_ColorInput:
            GrAssert(!"needComputedColor should be false.");
            break;
        case GrGLProgram::ProgramDesc::kSolidWhite_ColorInput:
            break;
        default:
            GrCrash("Unknown color type.");
            break;
    }
}

void genAttributeCoverage(GrGLShaderBuilder* segments,
                          GrStringBuilder* inOutCoverage) {
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
    
void genUniformCoverage(GrGLShaderBuilder* segments,
                        GrGLProgram::CachedData* programData,
                        GrStringBuilder* inOutCoverage) {
    segments->addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
                         kVec4f_GrSLType, COV_UNI_NAME);
    programData->fUniLocations.fCoverageUni = kUseUniform;
    if (inOutCoverage->size()) {
        segments->fFSCode.appendf("\tvec4 uniCoverage = %s * %s;\n",
                                  COV_UNI_NAME, inOutCoverage->c_str());
        *inOutCoverage = "uniCoverage";
    } else {
        *inOutCoverage = COV_UNI_NAME;
    }
}

}

void GrGLProgram::genGeometryShader(const GrGLContextInfo& gl,
                                    GrGLShaderBuilder* segments) const {
#if GR_GL_EXPERIMENTAL_GS
    if (fProgramDesc.fExperimentalGS) {
        GrAssert(gl.glslGeneration() >= k150_GrGLSLGeneration);
        segments->fGSHeader.append("layout(triangles) in;\n"
                                   "layout(triangle_strip, max_vertices = 6) out;\n");
        segments->fGSCode.append("void main() {\n"
                                 "\tfor (int i = 0; i < 3; ++i) {\n"
                                  "\t\tgl_Position = gl_in[i].gl_Position;\n");
        if (this->fProgramDesc.fEmitsPointSize) {
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
                                 "\tEndPrimitive();\n"
                                 "}\n");
    }
#endif
}

const char* GrGLProgram::adjustInColor(const GrStringBuilder& inColor) const {
    if (inColor.size()) {
          return inColor.c_str();
    } else {
        if (ProgramDesc::kSolidWhite_ColorInput == fProgramDesc.fColorInput) {
            return all_ones_vec(4);
        } else {
            return all_zeros_vec(4);
        }
    }
}

// If this destructor is in the header file, we must include GrGLProgramStage
// instead of just forward-declaring it.
GrGLProgram::CachedData::~CachedData() {
    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        delete fCustomStage[i];
    }
}


bool GrGLProgram::genProgram(const GrGLContextInfo& gl,
                             GrCustomStage** customStages,
                             GrGLProgram::CachedData* programData) const {
    GrGLShaderBuilder segments;
    const uint32_t& layout = fProgramDesc.fVertexLayout;

    programData->fUniLocations.reset();

#if GR_GL_EXPERIMENTAL_GS
    segments.fUsesGS = fProgramDesc.fExperimentalGS;
#endif

    SkXfermode::Coeff colorCoeff, uniformCoeff;
    bool applyColorMatrix = SkToBool(fProgramDesc.fColorMatrixEnabled);
    // The rest of transfer mode color filters have not been implemented
    if (fProgramDesc.fColorFilterXfermode < SkXfermode::kCoeffModesCnt) {
        GR_DEBUGCODE(bool success =)
            SkXfermode::ModeAsCoeff(static_cast<SkXfermode::Mode>
                                    (fProgramDesc.fColorFilterXfermode),
                                    &uniformCoeff, &colorCoeff);
        GR_DEBUGASSERT(success);
    } else {
        colorCoeff = SkXfermode::kOne_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
    }

    // no need to do the color filter / matrix at all if coverage is 0. The
    // output color is scaled by the coverage. All the dual source outputs are
    // scaled by the coverage as well.
    if (ProgramDesc::kTransBlack_ColorInput == fProgramDesc.fCoverageInput) {
        colorCoeff = SkXfermode::kZero_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
        applyColorMatrix = false;
    }

    // If we know the final color is going to be all zeros then we can
    // simplify the color filter coeffecients. needComputedColor will then
    // come out false below.
    if (ProgramDesc::kTransBlack_ColorInput == fProgramDesc.fColorInput) {
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
    needBlendInputs(uniformCoeff, colorCoeff,
                    &needColorFilterUniform, &needComputedColor);

    // the dual source output has no canonical var name, have to
    // declare an output, which is incompatible with gl_FragColor/gl_FragData.
    bool dualSourceOutputWritten = false;
    segments.fHeader.printf(GrGetGLSLVersionDecl(gl.binding(),
                                                 gl.glslGeneration()));

    GrGLShaderVar colorOutput;
    bool isColorDeclared = GrGLSLSetupFSColorOuput(gl.glslGeneration(),
                                                   declared_color_output_name(),
                                                   &colorOutput);
    if (isColorDeclared) {
        segments.fFSOutputs.push_back(colorOutput);
    }

    segments.addUniform(GrGLShaderBuilder::kVertex_VariableLifetime,
                        kMat33f_GrSLType, VIEW_MATRIX_NAME);
    programData->fUniLocations.fViewMatrixUni = kUseUniform;

    segments.fVSAttrs.push_back().set(kVec2f_GrSLType,
        GrGLShaderVar::kAttribute_TypeModifier, POS_ATTR_NAME);

    segments.fVSCode.append(
        "void main() {\n"
            "\tvec3 pos3 = " VIEW_MATRIX_NAME " * vec3("POS_ATTR_NAME", 1);\n"
            "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n");

    // incoming color to current stage being processed.
    GrStringBuilder inColor;

    if (needComputedColor) {
        genInputColor((ProgramDesc::ColorInput) fProgramDesc.fColorInput,
                      programData, &segments, &inColor);
    }

    // we output point size in the GS if present
    if (fProgramDesc.fEmitsPointSize && !segments.fUsesGS){
        segments.fVSCode.append("\tgl_PointSize = 1.0;\n");
    }

    segments.fFSCode.append("void main() {\n");

    // add texture coordinates that are used to the list of vertex attr decls
    GrStringBuilder texCoordAttrs[GrDrawState::kMaxTexCoords];
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (GrDrawTarget::VertexUsesTexCoordIdx(t, layout)) {
            tex_attr_name(t, texCoordAttrs + t);
            segments.fVSAttrs.push_back().set(kVec2f_GrSLType,
                GrGLShaderVar::kAttribute_TypeModifier,
                texCoordAttrs[t].c_str());
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // We need to convert generic effect representations to GL-specific
    // backends so they can be accesseed in genStageCode() and in subsequent,
    // uses of programData, but it's safest to do so below when we're *sure*
    // we need them.
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        programData->fCustomStage[s] = NULL;
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the final color

    // if we have color stages string them together, feeding the output color
    // of each to the next and generating code for each stage.
    if (needComputedColor) {
        GrStringBuilder outColor;
        for (int s = 0; s < fProgramDesc.fFirstCoverageStage; ++s) {
            if (fProgramDesc.fStages[s].isEnabled()) {
                // create var to hold stage result
                outColor = "color";
                outColor.appendS32(s);
                segments.fFSCode.appendf("\tvec4 %s;\n", outColor.c_str());

                const char* inCoords;
                // figure out what our input coords are
                if (GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s) &
                    layout) {
                    inCoords = POS_ATTR_NAME;
                } else {
                    int tcIdx = GrDrawTarget::VertexTexCoordsForStage(s, layout);
                     // we better have input tex coordinates if stage is enabled.
                    GrAssert(tcIdx >= 0);
                    GrAssert(texCoordAttrs[tcIdx].size());
                    inCoords = texCoordAttrs[tcIdx].c_str();
                }

                if (NULL != customStages[s]) {
                    const GrProgramStageFactory& factory =
                        customStages[s]->getFactory();
                    programData->fCustomStage[s] =
                        factory.createGLInstance(*customStages[s]);
                }
                this->genStageCode(gl,
                                   s,
                                   fProgramDesc.fStages[s],
                                   inColor.size() ? inColor.c_str() : NULL,
                                   outColor.c_str(),
                                   inCoords,
                                   &segments,
                                   &programData->fUniLocations.fStages[s],
                                   programData->fCustomStage[s]);
                inColor = outColor;
            }
        }
    }

    // if have all ones or zeros for the "dst" input to the color filter then we
    // may be able to make additional optimizations.
    if (needColorFilterUniform && needComputedColor && !inColor.size()) {
        GrAssert(ProgramDesc::kSolidWhite_ColorInput == fProgramDesc.fColorInput);
        bool uniformCoeffIsZero = SkXfermode::kIDC_Coeff == uniformCoeff ||
                                  SkXfermode::kIDA_Coeff == uniformCoeff;
        if (uniformCoeffIsZero) {
            uniformCoeff = SkXfermode::kZero_Coeff;
            bool bogus;
            needBlendInputs(SkXfermode::kZero_Coeff, colorCoeff,
                            &needColorFilterUniform, &bogus);
        }
    }
    if (needColorFilterUniform) {
        segments.addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
                            kVec4f_GrSLType, COL_FILTER_UNI_NAME);
        programData->fUniLocations.fColorFilterUni = kUseUniform;
    }
    bool wroteFragColorZero = false;
    if (SkXfermode::kZero_Coeff == uniformCoeff &&
        SkXfermode::kZero_Coeff == colorCoeff &&
        !applyColorMatrix) {
        segments.fFSCode.appendf("\t%s = %s;\n",
                                 colorOutput.getName().c_str(),
                                 all_zeros_vec(4));
        wroteFragColorZero = true;
    } else if (SkXfermode::kDst_Mode != fProgramDesc.fColorFilterXfermode) {
        segments.fFSCode.append("\tvec4 filteredColor;\n");
        const char* color = adjustInColor(inColor);
        addColorFilter(&segments.fFSCode, "filteredColor", uniformCoeff,
                       colorCoeff, color);
        inColor = "filteredColor";
    }
    if (applyColorMatrix) {
        segments.addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
                            kMat44f_GrSLType, COL_MATRIX_UNI_NAME);
        segments.addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
                            kVec4f_GrSLType, COL_MATRIX_VEC_UNI_NAME);
        programData->fUniLocations.fColorMatrixUni = kUseUniform;
        programData->fUniLocations.fColorMatrixVecUni = kUseUniform;
        segments.fFSCode.append("\tvec4 matrixedColor;\n");
        const char* color = adjustInColor(inColor);
        addColorMatrix(&segments.fFSCode, "matrixedColor", color);
        inColor = "matrixedColor";
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage (coverage stages and edge aa)

    GrStringBuilder inCoverage;
    bool coverageIsZero = ProgramDesc::kTransBlack_ColorInput ==
                          fProgramDesc.fCoverageInput;
    // we don't need to compute coverage at all if we know the final shader
    // output will be zero and we don't have a dual src blend output.
    if (!wroteFragColorZero ||
        ProgramDesc::kNone_DualSrcOutput != fProgramDesc.fDualSrcOutput) {

        if (!coverageIsZero) {
            this->genEdgeCoverage(gl,
                                  layout,
                                  programData,
                                  &inCoverage,
                                  &segments);

            switch (fProgramDesc.fCoverageInput) {
                case ProgramDesc::kSolidWhite_ColorInput:
                    // empty string implies solid white
                    break;
                case ProgramDesc::kAttribute_ColorInput:
                    genAttributeCoverage(&segments, &inCoverage);
                    break;
                case ProgramDesc::kUniform_ColorInput:
                    genUniformCoverage(&segments, programData, &inCoverage);
                    break;
                default:
                    GrCrash("Unexpected input coverage.");
            }

            GrStringBuilder outCoverage;
            const int& startStage = fProgramDesc.fFirstCoverageStage;
            for (int s = startStage; s < GrDrawState::kNumStages; ++s) {
                if (fProgramDesc.fStages[s].isEnabled()) {
                    // create var to hold stage output
                    outCoverage = "coverage";
                    outCoverage.appendS32(s);
                    segments.fFSCode.appendf("\tvec4 %s;\n",
                                             outCoverage.c_str());

                    const char* inCoords;
                    // figure out what our input coords are
                    if (GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s) &
                        layout) {
                        inCoords = POS_ATTR_NAME;
                    } else {
                        int tcIdx =
                            GrDrawTarget::VertexTexCoordsForStage(s, layout);
                        // we better have input tex coordinates if stage is
                        // enabled.
                        GrAssert(tcIdx >= 0);
                        GrAssert(texCoordAttrs[tcIdx].size());
                        inCoords = texCoordAttrs[tcIdx].c_str();
                    }

                    if (NULL != customStages[s]) {
                        const GrProgramStageFactory& factory =
                            customStages[s]->getFactory();
                        programData->fCustomStage[s] =
                            factory.createGLInstance(*customStages[s]);
                    }
                    this->genStageCode(gl, s,
                        fProgramDesc.fStages[s],
                        inCoverage.size() ? inCoverage.c_str() : NULL,
                        outCoverage.c_str(),
                        inCoords,
                        &segments,
                        &programData->fUniLocations.fStages[s],
                        programData->fCustomStage[s]);
                    inCoverage = outCoverage;
                }
            }
        }
        if (ProgramDesc::kNone_DualSrcOutput != fProgramDesc.fDualSrcOutput) {
            segments.fFSOutputs.push_back().set(kVec4f_GrSLType,
                GrGLShaderVar::kOut_TypeModifier,
                dual_source_output_name());
            bool outputIsZero = coverageIsZero;
            GrStringBuilder coeff;
            if (!outputIsZero &&
                ProgramDesc::kCoverage_DualSrcOutput !=
                fProgramDesc.fDualSrcOutput && !wroteFragColorZero) {
                if (!inColor.size()) {
                    outputIsZero = true;
                } else {
                    if (fProgramDesc.fDualSrcOutput ==
                        ProgramDesc::kCoverageISA_DualSrcOutput) {
                        coeff.printf("(1 - %s.a)", inColor.c_str());
                    } else {
                        coeff.printf("(vec4(1,1,1,1) - %s)", inColor.c_str());
                    }
                }
            }
            if (outputIsZero) {
                segments.fFSCode.appendf("\t%s = %s;\n",
                                         dual_source_output_name(),
                                         all_zeros_vec(4));
            } else {
                modulate_helper(dual_source_output_name(),
                                coeff.c_str(),
                                inCoverage.c_str(),
                                &segments.fFSCode);
            }
            dualSourceOutputWritten = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    if (!wroteFragColorZero) {
        if (coverageIsZero) {
            segments.fFSCode.appendf("\t%s = %s;\n",
                                     colorOutput.getName().c_str(),
                                     all_zeros_vec(4));
        } else {
            modulate_helper(colorOutput.getName().c_str(),
                            inColor.c_str(),
                            inCoverage.c_str(),
                            &segments.fFSCode);
        }
        if (ProgramDesc::kUnpremultiplied_RoundDown_OutputConfig ==
            fProgramDesc.fOutputConfig) {
            segments.fFSCode.appendf("\t%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(floor(%s.rgb / %s.a * 255.0)/255.0, %s.a);\n",
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str());
        } else if (ProgramDesc::kUnpremultiplied_RoundUp_OutputConfig ==
                   fProgramDesc.fOutputConfig) {
            segments.fFSCode.appendf("\t%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(ceil(%s.rgb / %s.a * 255.0)/255.0, %s.a);\n",
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str(),
                                        colorOutput.getName().c_str());
        }
    }

    segments.fVSCode.append("}\n");
    segments.fFSCode.append("}\n");

    ///////////////////////////////////////////////////////////////////////////
    // insert GS
#if GR_DEBUG
    this->genGeometryShader(gl, &segments);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // compile and setup attribs and unis

    if (!CompileShaders(gl, segments, programData)) {
        return false;
    }

    if (!this->bindOutputsAttribsAndLinkProgram(gl, texCoordAttrs,
                                                isColorDeclared,
                                                dualSourceOutputWritten,
                                                programData)) {
        return false;
    }

    this->getUniformLocationsAndInitCache(gl, programData);

    return true;
}

namespace {

inline void expand_decls(const VarArray& vars,
                         const GrGLContextInfo& gl,
                         GrStringBuilder* string) {
    const int count = vars.count();
    for (int i = 0; i < count; ++i) {
        vars[i].appendDecl(gl, string);
    }
}

inline void print_shader(int stringCnt,
                         const char** strings,
                         int* stringLengths) {
    for (int i = 0; i < stringCnt; ++i) {
        if (NULL == stringLengths || stringLengths[i] < 0) {
            GrPrintf(strings[i]);
        } else {
            GrPrintf("%.*s", stringLengths[i], strings[i]);
        }
    }
}

typedef SkTArray<const char*, true>         StrArray;
#define PREALLOC_STR_ARRAY(N) SkSTArray<(N), const char*, true>

typedef SkTArray<int, true>                 LengthArray;
#define PREALLOC_LENGTH_ARRAY(N) SkSTArray<(N), int, true>

// these shouldn't relocate
typedef GrTAllocator<GrStringBuilder>       TempArray;
#define PREALLOC_TEMP_ARRAY(N) GrSTAllocator<(N), GrStringBuilder>

inline void append_string(const GrStringBuilder& str,
                          StrArray* strings,
                          LengthArray* lengths) {
    int length = (int) str.size();
    if (length) {
        strings->push_back(str.c_str());
        lengths->push_back(length);
    }
    GrAssert(strings->count() == lengths->count());
}

inline void append_decls(const VarArray& vars,
                         const GrGLContextInfo& gl,
                         StrArray* strings,
                         LengthArray* lengths,
                         TempArray* temp) {
    expand_decls(vars, gl, &temp->push_back());
    append_string(temp->back(), strings, lengths);
}

}

bool GrGLProgram::CompileShaders(const GrGLContextInfo& gl,
                                 const GrGLShaderBuilder& segments,
                                 CachedData* programData) {
    enum { kPreAllocStringCnt = 8 };

    PREALLOC_STR_ARRAY(kPreAllocStringCnt)    strs;
    PREALLOC_LENGTH_ARRAY(kPreAllocStringCnt) lengths;
    PREALLOC_TEMP_ARRAY(kPreAllocStringCnt)   temps;

    GrStringBuilder unis;
    GrStringBuilder inputs;
    GrStringBuilder outputs;

    append_string(segments.fHeader, &strs, &lengths);
    append_decls(segments.fVSUnis, gl, &strs, &lengths, &temps);
    append_decls(segments.fVSAttrs, gl, &strs, &lengths, &temps);
    append_decls(segments.fVSOutputs, gl, &strs, &lengths, &temps);
    append_string(segments.fVSCode, &strs, &lengths);

#if PRINT_SHADERS
    print_shader(strs.count(), &strs[0], &lengths[0]);
    GrPrintf("\n");
#endif

    programData->fVShaderID =
        CompileShader(gl, GR_GL_VERTEX_SHADER, strs.count(),
                      &strs[0], &lengths[0]);

    if (!programData->fVShaderID) {
        return false;
    }
    if (segments.fUsesGS) {
        strs.reset();
        lengths.reset();
        temps.reset();
        append_string(segments.fHeader, &strs, &lengths);
        append_string(segments.fGSHeader, &strs, &lengths);
        append_decls(segments.fGSInputs, gl, &strs, &lengths, &temps);
        append_decls(segments.fGSOutputs, gl, &strs, &lengths, &temps);
        append_string(segments.fGSCode, &strs, &lengths);
#if PRINT_SHADERS
        print_shader(strs.count(), &strs[0], &lengths[0]);
        GrPrintf("\n");
#endif
        programData->fGShaderID =
            CompileShader(gl, GR_GL_GEOMETRY_SHADER, strs.count(),
                          &strs[0], &lengths[0]);
    } else {
        programData->fGShaderID = 0;
    }

    strs.reset();
    lengths.reset();
    temps.reset();

    append_string(segments.fHeader, &strs, &lengths);
    GrStringBuilder precisionStr(GrGetGLSLShaderPrecisionDecl(gl.binding()));
    append_string(precisionStr, &strs, &lengths);
    append_decls(segments.fFSUnis, gl, &strs, &lengths, &temps);
    append_decls(segments.fFSInputs, gl, &strs, &lengths, &temps);
    // We shouldn't have declared outputs on 1.10
    GrAssert(k110_GrGLSLGeneration != gl.glslGeneration() ||
             segments.fFSOutputs.empty());
    append_decls(segments.fFSOutputs, gl, &strs, &lengths, &temps);
    append_string(segments.fFSFunctions, &strs, &lengths);
    append_string(segments.fFSCode, &strs, &lengths);

#if PRINT_SHADERS
    print_shader(strs.count(), &strs[0], &lengths[0]);
    GrPrintf("\n");
#endif

    programData->fFShaderID =
        CompileShader(gl, GR_GL_FRAGMENT_SHADER, strs.count(),
                      &strs[0], &lengths[0]);

    if (!programData->fFShaderID) {
        return false;
    }

    return true;
}

#define GL_CALL(X) GR_GL_CALL(gl.interface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gl.interface(), R, X)

GrGLuint GrGLProgram::CompileShader(const GrGLContextInfo& gl,
                                    GrGLenum type,
                                    int stringCnt,
                                    const char** strings,
                                    int* stringLengths) {
    SK_TRACE_EVENT1("GrGLProgram::CompileShader",
                    "stringCount", SkStringPrintf("%i", stringCnt).c_str());

    GrGLuint shader;
    GL_CALL_RET(shader, CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    GrGLint compiled = GR_GL_INIT_ZERO;
    GL_CALL(ShaderSource(shader, stringCnt, strings, stringLengths));
    GL_CALL(CompileShader(shader));
    GL_CALL(GetShaderiv(shader, GR_GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetShaderiv(shader, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetShaderInfoLog(shader, infoLen+1, 
                                         &length, (char*)log.get()));
            print_shader(stringCnt, strings, stringLengths);
            GrPrintf("\n%s", log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GL_CALL(DeleteShader(shader));
        return 0;
    }
    return shader;
}

bool GrGLProgram::bindOutputsAttribsAndLinkProgram(
                                        const GrGLContextInfo& gl,
                                        GrStringBuilder texCoordAttrNames[],
                                        bool bindColorOut,
                                        bool bindDualSrcOut,
                                        CachedData* programData) const {
    GL_CALL_RET(programData->fProgramID, CreateProgram());
    if (!programData->fProgramID) {
        return false;
    }
    const GrGLint& progID = programData->fProgramID;

    GL_CALL(AttachShader(progID, programData->fVShaderID));
    if (programData->fGShaderID) {
        GL_CALL(AttachShader(progID, programData->fGShaderID));
    }
    GL_CALL(AttachShader(progID, programData->fFShaderID));

    if (bindColorOut) {
        GL_CALL(BindFragDataLocation(programData->fProgramID,
                                     0, declared_color_output_name()));
    }
    if (bindDualSrcOut) {
        GL_CALL(BindFragDataLocationIndexed(programData->fProgramID,
                                            0, 1, dual_source_output_name()));
    }

    // Bind the attrib locations to same values for all shaders
    GL_CALL(BindAttribLocation(progID, PositionAttributeIdx(), POS_ATTR_NAME));
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (texCoordAttrNames[t].size()) {
            GL_CALL(BindAttribLocation(progID,
                                       TexCoordAttributeIdx(t),
                                       texCoordAttrNames[t].c_str()));
        }
    }

    GL_CALL(BindAttribLocation(progID, ColorAttributeIdx(), COL_ATTR_NAME));
    GL_CALL(BindAttribLocation(progID, CoverageAttributeIdx(), COV_ATTR_NAME));
    GL_CALL(BindAttribLocation(progID, EdgeAttributeIdx(), EDGE_ATTR_NAME));

    GL_CALL(LinkProgram(progID));

    GrGLint linked = GR_GL_INIT_ZERO;
    GL_CALL(GetProgramiv(progID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(progID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramInfoLog(progID,
                                      infoLen+1,
                                      &length,
                                      (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GL_CALL(DeleteProgram(progID));
        programData->fProgramID = 0;
        return false;
    }
    return true;
}

void GrGLProgram::getUniformLocationsAndInitCache(const GrGLContextInfo& gl,
                                                  CachedData* programData) const {
    const GrGLint& progID = programData->fProgramID;

    if (kUseUniform == programData->fUniLocations.fViewMatrixUni) {
        GL_CALL_RET(programData->fUniLocations.fViewMatrixUni,
                    GetUniformLocation(progID, VIEW_MATRIX_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fViewMatrixUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorUni) {
        GL_CALL_RET(programData->fUniLocations.fColorUni,
                    GetUniformLocation(progID, COL_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorFilterUni) {
        GL_CALL_RET(programData->fUniLocations.fColorFilterUni, 
                    GetUniformLocation(progID, COL_FILTER_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorFilterUni);
    }

    if (kUseUniform == programData->fUniLocations.fColorMatrixUni) {
        GL_CALL_RET(programData->fUniLocations.fColorMatrixUni,
                    GetUniformLocation(progID, COL_MATRIX_UNI_NAME));
    }

    if (kUseUniform == programData->fUniLocations.fColorMatrixVecUni) {
        GL_CALL_RET(programData->fUniLocations.fColorMatrixVecUni,
                    GetUniformLocation(progID, COL_MATRIX_VEC_UNI_NAME));
    }
    if (kUseUniform == programData->fUniLocations.fCoverageUni) {
        GL_CALL_RET(programData->fUniLocations.fCoverageUni,
                    GetUniformLocation(progID, COV_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fCoverageUni);
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        StageUniLocations& locations = programData->fUniLocations.fStages[s];
        if (fProgramDesc.fStages[s].isEnabled()) {
            if (kUseUniform == locations.fTextureMatrixUni) {
                GrStringBuilder texMName;
                tex_matrix_name(s, &texMName);
                GL_CALL_RET(locations.fTextureMatrixUni,
                            GetUniformLocation(progID, texMName.c_str()));
                GrAssert(kUnusedUniform != locations.fTextureMatrixUni);
            }

            if (kUseUniform == locations.fSamplerUni) {
                GrStringBuilder samplerName;
                sampler_name(s, &samplerName);
                GL_CALL_RET(locations.fSamplerUni,
                            GetUniformLocation(progID,samplerName.c_str()));
                GrAssert(kUnusedUniform != locations.fSamplerUni);
            }

            if (kUseUniform == locations.fTexDomUni) {
                GrStringBuilder texDomName;
                tex_domain_name(s, &texDomName);
                GL_CALL_RET(locations.fTexDomUni,
                            GetUniformLocation(progID, texDomName.c_str()));
                GrAssert(kUnusedUniform != locations.fTexDomUni);
            }

            if (NULL != programData->fCustomStage[s]) {
                programData->fCustomStage[s]->
                    initUniforms(gl.interface(), progID);
            }
        }
    }
    GL_CALL(UseProgram(progID));

    // init sampler unis and set bogus values for state tracking
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (kUnusedUniform != programData->fUniLocations.fStages[s].fSamplerUni) {
            GL_CALL(Uniform1i(programData->fUniLocations.fStages[s].fSamplerUni, s));
        }
        programData->fTextureMatrices[s] = GrMatrix::InvalidMatrix();
        programData->fTextureDomain[s].setEmpty();
        // this is arbitrary, just initialize to something
        programData->fTextureOrientation[s] =
            GrGLTexture::kBottomUp_Orientation;
        // Must not reset fStageOverride[] here.
    }
    programData->fViewMatrix = GrMatrix::InvalidMatrix();
    programData->fViewportSize.set(-1, -1);
    programData->fColor = GrColor_ILLEGAL;
    programData->fColorFilterColor = GrColor_ILLEGAL;
}

//============================================================================
// Stage code generation
//============================================================================

void GrGLProgram::genStageCode(const GrGLContextInfo& gl,
                               int stageNum,
                               const GrGLProgram::StageDesc& desc,
                               const char* fsInColor, // NULL means no incoming color
                               const char* fsOutColor,
                               const char* vsInCoord,
                               GrGLShaderBuilder* segments,
                               StageUniLocations* locations,
                               GrGLProgramStage* customStage) const {

    GrAssert(stageNum >= 0 && stageNum <= GrDrawState::kNumStages);
    GrAssert((desc.fInConfigFlags & StageDesc::kInConfigBitMask) ==
             desc.fInConfigFlags);

    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords
    // and whether the varying needs a perspective coord.
    const char* matName = NULL;
    if (desc.fOptFlags & StageDesc::kIdentityMatrix_OptFlagBit) {
        segments->fVaryingDims = segments->fCoordDims;
    } else {
        GrStringBuilder texMatName;
        tex_matrix_name(stageNum, &texMatName);
        const GrGLShaderVar* mat = &segments->addUniform(
            GrGLShaderBuilder::kVertex_VariableLifetime, kMat33f_GrSLType,
            texMatName.c_str());
        // Can't use texMatName.c_str() because it's on the stack!
        matName = mat->getName().c_str();
        locations->fTextureMatrixUni = kUseUniform;

        if (desc.fOptFlags & StageDesc::kNoPerspective_OptFlagBit) {
            segments->fVaryingDims = segments->fCoordDims;
        } else {
            segments->fVaryingDims = segments->fCoordDims + 1;
        }
    }
    GrAssert(segments->fVaryingDims > 0);

    // Must setup variables after computing segments->fVaryingDims
    if (NULL != customStage) {
        customStage->setupVariables(segments, stageNum);
    }

    GrStringBuilder samplerName;
    sampler_name(stageNum, &samplerName);
    // const GrGLShaderVar* sampler = &
        segments->addUniform(GrGLShaderBuilder::kFragment_VariableLifetime,
        kSampler2D_GrSLType, samplerName.c_str());
    locations->fSamplerUni = kUseUniform;

    const char *varyingVSName, *varyingFSName;
    segments->addVarying(GrSLFloatVectorType(segments->fVaryingDims),
                         "Stage",
                        stageNum,
                        &varyingVSName,
                        &varyingFSName);

    if (!matName) {
        GrAssert(segments->fVaryingDims == segments->fCoordDims);
        segments->fVSCode.appendf("\t%s = %s;\n", varyingVSName, vsInCoord);
    } else {
        // varying = texMatrix * texCoord
        segments->fVSCode.appendf("\t%s = (%s * vec3(%s, 1))%s;\n",
                                  varyingVSName, matName, vsInCoord,
                                  vector_all_coords(segments->fVaryingDims));
    }

    // GrGLShaderVar* kernel = NULL;
    // const char* imageIncrementName = NULL;
    if (NULL != customStage) {
        segments->fVSCode.appendf("\t{ // stage %d %s\n",
                                  stageNum, customStage->name());
        customStage->emitVS(segments, varyingVSName);
        segments->fVSCode.appendf("\t}\n");
    }

    /// Fragment Shader Stuff

    segments->fSampleCoords = varyingFSName;

    GrGLShaderBuilder::SamplerMode sampleMode =
        GrGLShaderBuilder::kExplicitDivide_SamplerMode;
    if (desc.fOptFlags & (StageDesc::kIdentityMatrix_OptFlagBit |
                          StageDesc::kNoPerspective_OptFlagBit)) {
        sampleMode = GrGLShaderBuilder::kDefault_SamplerMode;
    } else if (NULL == customStage) {
        sampleMode = GrGLShaderBuilder::kProj_SamplerMode;
    }
    segments->setupTextureAccess(sampleMode, stageNum);

    segments->computeSwizzle(desc.fInConfigFlags);
    segments->computeModulate(fsInColor);

    static const uint32_t kMulByAlphaMask =
        (StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag |
         StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag);

    if (desc.fOptFlags & StageDesc::kCustomTextureDomain_OptFlagBit) {
        GrStringBuilder texDomainName;
        tex_domain_name(stageNum, &texDomainName);
        // const GrGLShaderVar* texDomain = &
            segments->addUniform(
                GrGLShaderBuilder::kFragment_VariableLifetime,
                kVec4f_GrSLType, texDomainName.c_str());
        GrStringBuilder coordVar("clampCoord");
        segments->fFSCode.appendf("\t%s %s = clamp(%s, %s.xy, %s.zw);\n",
                                  float_vector_type_str(segments->fCoordDims),
                                  coordVar.c_str(),
                                  segments->fSampleCoords.c_str(),
                                  texDomainName.c_str(),
                                  texDomainName.c_str());
        segments->fSampleCoords = coordVar;
        locations->fTexDomUni = kUseUniform;
    }

    // NOTE: GrGLProgramStages are now responsible for fetching
    if (NULL == customStage) {
        if (desc.fInConfigFlags & kMulByAlphaMask) {
            // only one of the mul by alpha flags should be set
            GrAssert(GrIsPow2(kMulByAlphaMask & desc.fInConfigFlags));
            GrAssert(!(desc.fInConfigFlags & 
                       StageDesc::kSmearAlpha_InConfigFlag));
            GrAssert(!(desc.fInConfigFlags & 
                       StageDesc::kSmearRed_InConfigFlag));
            segments->fFSCode.appendf("\t%s = %s(%s, %s)%s;\n",
                                      fsOutColor,
                                      segments->fTexFunc.c_str(), 
                                      samplerName.c_str(),
                                      segments->fSampleCoords.c_str(),
                                      segments->fSwizzle.c_str());
            if (desc.fInConfigFlags &
                StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag) {
                segments->fFSCode.appendf("\t%s = vec4(ceil(%s.rgb*%s.a*255.0)/255.0,%s.a)%s;\n",
                                          fsOutColor, fsOutColor, fsOutColor,
                                          fsOutColor, segments->fModulate.c_str());
            } else {
                segments->fFSCode.appendf("\t%s = vec4(floor(%s.rgb*%s.a*255.0)/255.0,%s.a)%s;\n",
                                          fsOutColor, fsOutColor, fsOutColor,
                                          fsOutColor, segments->fModulate.c_str());
            }
        } else {
            segments->emitDefaultFetch(fsOutColor, samplerName.c_str());
        }
    }

    if (NULL != customStage) {
        // Enclose custom code in a block to avoid namespace conflicts
        segments->fFSCode.appendf("\t{ // stage %d %s \n",
                                  stageNum, customStage->name());
        customStage->emitFS(segments, fsOutColor, fsInColor,
                            samplerName.c_str());
        segments->fFSCode.appendf("\t}\n");
    }
}


