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

SK_DEFINE_INST_COUNT(GrGLProgram)

#define GL_CALL(X) GR_GL_CALL(fContextInfo.interface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fContextInfo.interface(), R, X)

#define PRINT_SHADERS 0

typedef GrGLProgram::Desc::StageDesc StageDesc;

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"
#define COV_ATTR_NAME "aCoverage"
#define EDGE_ATTR_NAME "aEdge"

namespace {
inline void tex_attr_name(int coordIdx, SkString* s) {
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

inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }

}

GrGLProgram* GrGLProgram::Create(const GrGLContextInfo& gl,
                                 const Desc& desc,
                                 const GrCustomStage** customStages) {
    GrGLProgram* program = SkNEW_ARGS(GrGLProgram, (gl, desc, customStages));
    if (!program->succeeded()) {
        delete program;
        program = NULL;
    }
    return program;
}

GrGLProgram::GrGLProgram(const GrGLContextInfo& gl,
                         const Desc& desc,
                         const GrCustomStage** customStages)
: fContextInfo(gl)
, fUniformManager(gl) {
    fDesc = desc;
    fVShaderID = 0;
    fGShaderID = 0;
    fFShaderID = 0;
    fProgramID = 0;

    fViewMatrix = GrMatrix::InvalidMatrix();
    fViewportSize.set(-1, -1);
    fColor = GrColor_ILLEGAL;
    fColorFilterColor = GrColor_ILLEGAL;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        fProgramStage[s] = NULL;
        fTextureMatrices[s] = GrMatrix::InvalidMatrix();
        // this is arbitrary, just initialize to something
        fTextureOrientation[s] = GrGLTexture::kBottomUp_Orientation;
    }

    this->genProgram(customStages);
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
        delete fProgramStage[i];
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
static void blendTermString(SkString* str, SkXfermode::Coeff coeff,
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
static void addColorFilter(SkString* fsCode, const char * outputVar,
                           SkXfermode::Coeff uniformCoeff,
                           SkXfermode::Coeff colorCoeff,
                           const char* filterColor,
                           const char* inColor) {
    SkString colorStr, constStr;
    blendTermString(&colorStr, colorCoeff, filterColor, inColor, inColor);
    blendTermString(&constStr, uniformCoeff, filterColor, inColor, filterColor);

    fsCode->appendf("\t%s = ", outputVar);
    GrGLSLAdd4f(fsCode, colorStr.c_str(), constStr.c_str());
    fsCode->append(";\n");
}

bool GrGLProgram::genEdgeCoverage(SkString* coverageVar,
                                  GrGLShaderBuilder* segments) const {
    if (fDesc.fVertexLayout & GrDrawTarget::kEdge_VertexLayoutBit) {
        const char *vsName, *fsName;
        segments->addVarying(kVec4f_GrSLType, "Edge", &vsName, &fsName);
        segments->fVSAttrs.push_back().set(kVec4f_GrSLType,
            GrGLShaderVar::kAttribute_TypeModifier, EDGE_ATTR_NAME);
        segments->fVSCode.appendf("\t%s = " EDGE_ATTR_NAME ";\n", vsName);
        switch (fDesc.fVertexEdgeType) {
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
            if (kES2_GrGLBinding == fContextInfo.binding()) {
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
            if (kES2_GrGLBinding == fContextInfo.binding()) {
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
            fUniforms.fColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
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
    fUniforms.fCoverageUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
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
        GrAssert(fContextInfo.glslGeneration() >= k150_GrGLSLGeneration);
        segments->fGSHeader.append("layout(triangles) in;\n"
                                   "layout(triangle_strip, max_vertices = 6) out;\n");
        segments->fGSCode.append("void main() {\n"
                                 "\tfor (int i = 0; i < 3; ++i) {\n"
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
                                 "\tEndPrimitive();\n"
                                 "}\n");
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
GrGLuint compile_shader(const GrGLContextInfo& gl,
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
GrGLuint compile_shader(const GrGLContextInfo& gl, GrGLenum type, const SkString& shader) {
    const GrGLchar* str = shader.c_str();
    int length = shader.size();
    return compile_shader(gl, type, 1, &str, &length);
}

}

// compiles all the shaders from builder and stores the shader IDs
bool GrGLProgram::compileShaders(const GrGLShaderBuilder& builder) {

    SkString shader;

    builder.getShader(GrGLShaderBuilder::kVertex_ShaderType, &shader);
#if PRINT_SHADERS
    GrPrintf(shader.c_str());
    GrPrintf("\n");
#endif
    if (!(fVShaderID = compile_shader(fContextInfo, GR_GL_VERTEX_SHADER, shader))) {
        return false;
    }

    if (builder.fUsesGS) {
        builder.getShader(GrGLShaderBuilder::kGeometry_ShaderType, &shader);
#if PRINT_SHADERS
        GrPrintf(shader.c_str());
        GrPrintf("\n");
#endif
        if (!(fGShaderID = compile_shader(fContextInfo, GR_GL_GEOMETRY_SHADER, shader))) {
            return false;
        }
    } else {
        fGShaderID = 0;
    }

    builder.getShader(GrGLShaderBuilder::kFragment_ShaderType, &shader);
#if PRINT_SHADERS
    GrPrintf(shader.c_str());
    GrPrintf("\n");
#endif
    if (!(fFShaderID = compile_shader(fContextInfo, GR_GL_FRAGMENT_SHADER, shader))) {
        return false;
    }

    return true;
}

bool GrGLProgram::genProgram(const GrCustomStage** customStages) {
    GrAssert(0 == fProgramID);

    GrGLShaderBuilder builder(fContextInfo, fUniformManager);
    const uint32_t& layout = fDesc.fVertexLayout;

#if GR_GL_EXPERIMENTAL_GS
    builder.fUsesGS = fDesc.fExperimentalGS;
#endif

    SkXfermode::Coeff colorCoeff, uniformCoeff;
    bool applyColorMatrix = SkToBool(fDesc.fColorMatrixEnabled);
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

    // no need to do the color filter / matrix at all if coverage is 0. The
    // output color is scaled by the coverage. All the dual source outputs are
    // scaled by the coverage as well.
    if (Desc::kTransBlack_ColorInput == fDesc.fCoverageInput) {
        colorCoeff = SkXfermode::kZero_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
        applyColorMatrix = false;
    }

    // If we know the final color is going to be all zeros then we can
    // simplify the color filter coeffecients. needComputedColor will then
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
    needBlendInputs(uniformCoeff, colorCoeff,
                    &needColorFilterUniform, &needComputedColor);

    // the dual source output has no canonical var name, have to
    // declare an output, which is incompatible with gl_FragColor/gl_FragData.
    bool dualSourceOutputWritten = false;
    builder.fHeader.append(GrGetGLSLVersionDecl(fContextInfo.binding(),
                                                fContextInfo.glslGeneration()));

    GrGLShaderVar colorOutput;
    bool isColorDeclared = GrGLSLSetupFSColorOuput(fContextInfo.glslGeneration(),
                                                   declared_color_output_name(),
                                                   &colorOutput);
    if (isColorDeclared) {
        builder.fFSOutputs.push_back(colorOutput);
    }

    const char* viewMName;
    fUniforms.fViewMatrixUni = builder.addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                                  kMat33f_GrSLType, "ViewM", &viewMName);

    builder.fVSAttrs.push_back().set(kVec2f_GrSLType,
                                     GrGLShaderVar::kAttribute_TypeModifier,
                                     POS_ATTR_NAME);

    builder.fVSCode.appendf("void main() {\n"
                              "\tvec3 pos3 = %s * vec3("POS_ATTR_NAME", 1);\n"
                              "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n",
                            viewMName);

    // incoming color to current stage being processed.
    SkString inColor;

    if (needComputedColor) {
        this->genInputColor(&builder, &inColor);
    }

    // we output point size in the GS if present
    if (fDesc.fEmitsPointSize && !builder.fUsesGS){
        builder.fVSCode.append("\tgl_PointSize = 1.0;\n");
    }

    builder.fFSCode.append("void main() {\n");

    // add texture coordinates that are used to the list of vertex attr decls
    SkString texCoordAttrs[GrDrawState::kMaxTexCoords];
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (GrDrawTarget::VertexUsesTexCoordIdx(t, layout)) {
            tex_attr_name(t, texCoordAttrs + t);
            builder.fVSAttrs.push_back().set(kVec2f_GrSLType,
                GrGLShaderVar::kAttribute_TypeModifier,
                texCoordAttrs[t].c_str());
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the final color

    // if we have color stages string them together, feeding the output color
    // of each to the next and generating code for each stage.
    if (needComputedColor) {
        SkString outColor;
        for (int s = 0; s < fDesc.fFirstCoverageStage; ++s) {
            if (fDesc.fStages[s].isEnabled()) {
                // create var to hold stage result
                outColor = "color";
                outColor.appendS32(s);
                builder.fFSCode.appendf("\tvec4 %s;\n", outColor.c_str());

                const char* inCoords;
                // figure out what our input coords are
                int tcIdx = GrDrawTarget::VertexTexCoordsForStage(s, layout);
                if (tcIdx < 0) {
                    inCoords = POS_ATTR_NAME;
                } else {
                    // must have input tex coordinates if stage is enabled.
                    GrAssert(texCoordAttrs[tcIdx].size());
                    inCoords = texCoordAttrs[tcIdx].c_str();
                }

                builder.setCurrentStage(s);
                fProgramStage[s] = GenStageCode(customStages[s],
                                                fDesc.fStages[s],
                                                &fUniforms.fStages[s],
                                                inColor.size() ? inColor.c_str() : NULL,
                                                outColor.c_str(),
                                                inCoords,
                                                &builder);
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
            needBlendInputs(SkXfermode::kZero_Coeff, colorCoeff,
                            &needColorFilterUniform, &bogus);
        }
    }
    const char* colorFilterColorUniName = NULL;
    if (needColorFilterUniform) {
        fUniforms.fColorFilterUni = builder.addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                       kVec4f_GrSLType, "FilterColor",
                                                       &colorFilterColorUniName);
    }
    bool wroteFragColorZero = false;
    if (SkXfermode::kZero_Coeff == uniformCoeff &&
        SkXfermode::kZero_Coeff == colorCoeff &&
        !applyColorMatrix) {
        builder.fFSCode.appendf("\t%s = %s;\n",
                                colorOutput.getName().c_str(),
                                GrGLSLZerosVecf(4));
        wroteFragColorZero = true;
    } else if (SkXfermode::kDst_Mode != fDesc.fColorFilterXfermode) {
        builder.fFSCode.append("\tvec4 filteredColor;\n");
        const char* color = adjustInColor(inColor);
        addColorFilter(&builder.fFSCode, "filteredColor", uniformCoeff,
                       colorCoeff, colorFilterColorUniName, color);
        inColor = "filteredColor";
    }
    if (applyColorMatrix) {
        const char* colMatrixName;
        const char* colMatrixVecName;
        fUniforms.fColorMatrixUni = builder.addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                       kMat44f_GrSLType, "ColorMatrix",
                                                       &colMatrixName);
        fUniforms.fColorMatrixVecUni = builder.addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                          kVec4f_GrSLType, "ColorMatrixVec",
                                                          &colMatrixVecName);
        const char* color = adjustInColor(inColor);
        builder.fFSCode.appendf("\tvec4 matrixedColor = %s * vec4(%s.rgb / %s.a, %s.a) + %s;\n",
                                colMatrixName, color, color, color, colMatrixVecName);
        builder.fFSCode.append("\tmatrixedColor.rgb *= matrixedColor.a;\n");

        inColor = "matrixedColor";
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
                if (fDesc.fStages[s].isEnabled()) {
                    // create var to hold stage output
                    outCoverage = "coverage";
                    outCoverage.appendS32(s);
                    builder.fFSCode.appendf("\tvec4 %s;\n", outCoverage.c_str());

                    const char* inCoords;
                    // figure out what our input coords are
                    int tcIdx =
                        GrDrawTarget::VertexTexCoordsForStage(s, layout);
                    if (tcIdx < 0) {
                        inCoords = POS_ATTR_NAME;
                    } else {
                        // must have input tex coordinates if stage is
                        // enabled.
                        GrAssert(texCoordAttrs[tcIdx].size());
                        inCoords = texCoordAttrs[tcIdx].c_str();
                    }

                    // stages don't know how to deal with a scalar input. (Maybe they should. We
                    // could pass a GrGLShaderVar)
                    if (inCoverageIsScalar) {
                        builder.fFSCode.appendf("\tvec4 %s4 = vec4(%s);\n",
                                                inCoverage.c_str(), inCoverage.c_str());
                        inCoverage.append("4");
                    }
                    builder.setCurrentStage(s);
                    fProgramStage[s] = GenStageCode(customStages[s],
                                                    fDesc.fStages[s],
                                                    &fUniforms.fStages[s],
                                                    inCoverage.size() ? inCoverage.c_str() : NULL,
                                                    outCoverage.c_str(),
                                                    inCoords,
                                                    &builder);
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

    builder.fVSCode.append("}\n");
    builder.fFSCode.append("}\n");

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

    if (!this->bindOutputsAttribsAndLinkProgram(texCoordAttrs,
                                                isColorDeclared,
                                                dualSourceOutputWritten)) {
        return false;
    }

    builder.finished(fProgramID);
    this->initSamplerUniforms();

    return true;
}

bool GrGLProgram::bindOutputsAttribsAndLinkProgram(SkString texCoordAttrNames[],
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
    GL_CALL(BindAttribLocation(fProgramID, PositionAttributeIdx(), POS_ATTR_NAME));
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (texCoordAttrNames[t].size()) {
            GL_CALL(BindAttribLocation(fProgramID,
                                       TexCoordAttributeIdx(t),
                                       texCoordAttrNames[t].c_str()));
        }
    }

    GL_CALL(BindAttribLocation(fProgramID, ColorAttributeIdx(), COL_ATTR_NAME));
    GL_CALL(BindAttribLocation(fProgramID, CoverageAttributeIdx(), COV_ATTR_NAME));
    GL_CALL(BindAttribLocation(fProgramID, EdgeAttributeIdx(), EDGE_ATTR_NAME));

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
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        int count = fUniforms.fStages[s].fSamplerUniforms.count();
        // FIXME: We're still always reserving one texture per stage. After GrTextureParams are
        // expressed by the custom stage rather than the GrSamplerState we can move texture binding
        // into GrGLProgram and it should be easier to fix this.
        GrAssert(count <= 1);
        for (int t = 0; t < count; ++t) {
            UniformHandle uh = fUniforms.fStages[s].fSamplerUniforms[t];
            if (GrGLUniformManager::kInvalidUniformHandle != uh) {
                fUniformManager.setSampler(uh, s);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Stage code generation

// TODO: Move this function to GrGLShaderBuilder
GrGLProgramStage* GrGLProgram::GenStageCode(const GrCustomStage* stage,
                                            const StageDesc& desc,
                                            StageUniforms* uniforms,
                                            const char* fsInColor, // NULL means no incoming color
                                            const char* fsOutColor,
                                            const char* vsInCoord,
                                            GrGLShaderBuilder* builder) {

    GrGLProgramStage* glStage = stage->getFactory().createGLInstance(*stage);

    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords and whether the varying needs a
    // perspective coord.
    const char* matName = NULL;
    GrSLType texCoordVaryingType;
    if (desc.fOptFlags & StageDesc::kIdentityMatrix_OptFlagBit) {
        texCoordVaryingType = kVec2f_GrSLType;
    } else {
        uniforms->fTextureMatrixUni = builder->addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                                         kMat33f_GrSLType, "TexM", &matName);
        builder->getUniformVariable(uniforms->fTextureMatrixUni);

        if (desc.fOptFlags & StageDesc::kNoPerspective_OptFlagBit) {
            texCoordVaryingType = kVec2f_GrSLType;
        } else {
            texCoordVaryingType = kVec3f_GrSLType;
        }
    }
    const char *varyingVSName, *varyingFSName;
    builder->addVarying(texCoordVaryingType,
                        "Stage",
                        &varyingVSName,
                        &varyingFSName);
    builder->setupTextureAccess(varyingFSName, texCoordVaryingType);

    // Must setup variables after calling setupTextureAccess
    glStage->setupVariables(builder);

    int numTextures = stage->numTextures();
    SkSTArray<8, GrGLShaderBuilder::TextureSampler> textureSamplers;

    textureSamplers.push_back_n(numTextures);

    for (int i = 0; i < numTextures; ++i) {
        textureSamplers[i].init(builder, &stage->textureAccess(i));
        uniforms->fSamplerUniforms.push_back(textureSamplers[i].fSamplerUniform);
    }

    if (!matName) {
        GrAssert(kVec2f_GrSLType == texCoordVaryingType);
        builder->fVSCode.appendf("\t%s = %s;\n", varyingVSName, vsInCoord);
    } else {
        // varying = texMatrix * texCoord
        builder->fVSCode.appendf("\t%s = (%s * vec3(%s, 1))%s;\n",
                                  varyingVSName, matName, vsInCoord,
                                  vector_all_coords(GrSLTypeToVecLength(texCoordVaryingType)));
    }

    builder->fVSCode.appendf("\t{ // %s\n", glStage->name());
    glStage->emitVS(builder, varyingVSName);
    builder->fVSCode.appendf("\t}\n");

    // Enclose custom code in a block to avoid namespace conflicts
    builder->fFSCode.appendf("\t{ // %s \n", glStage->name());
    glStage->emitFS(builder, fsOutColor, fsInColor, textureSamplers);
    builder->fFSCode.appendf("\t}\n");

    return glStage;
}
