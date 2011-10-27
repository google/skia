
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrGLShaderVar.h"
#include "SkTrace.h"
#include "SkXfermode.h"

namespace {

enum {
    /// Used to mark a StageUniLocation field that should be bound
    /// to a uniform during getUniformLocationsAndInitCache().
    kUseUniform = 2000
};


const char* GrPrecision(const GrGLInterface* gl) {
    if (gl->supportsES()) {
        return "mediump";
    } else {
        return " ";
    }
}

const char* GrShaderPrecision(const GrGLInterface* gl) {
    if (gl->supportsES()) {
        return "precision mediump float;\n";
    } else {
        return "";
    }
}

}  // namespace

#define PRINT_SHADERS 0

typedef GrTAllocator<GrGLShaderVar> VarArray;

// number of each input/output type in a single allocation block
static const int gVarsPerBlock = 8;
// except FS outputs where we expect 2 at most.
static const int gMaxFSOutputs = 2;

struct ShaderCodeSegments {
    ShaderCodeSegments() 
    : fVSUnis(gVarsPerBlock)
    , fVSAttrs(gVarsPerBlock)
    , fVSOutputs(gVarsPerBlock)
    , fGSInputs(gVarsPerBlock)
    , fGSOutputs(gVarsPerBlock)
    , fFSInputs(gVarsPerBlock)
    , fFSUnis(gVarsPerBlock)
    , fFSOutputs(gMaxFSOutputs)
    , fUsesGS(false) {}
    GrStringBuilder fHeader; // VS+FS, GLSL version, etc
    VarArray        fVSUnis;
    VarArray        fVSAttrs;
    VarArray        fVSOutputs;
    VarArray        fGSInputs;
    VarArray        fGSOutputs;
    VarArray        fFSInputs;
    GrStringBuilder fGSHeader; // layout qualifiers specific to GS
    VarArray        fFSUnis;
    VarArray        fFSOutputs;
    GrStringBuilder fFSFunctions;
    GrStringBuilder fVSCode;
    GrStringBuilder fGSCode;
    GrStringBuilder fFSCode;

    bool            fUsesGS;
};


#if GR_GL_ATTRIBUTE_MATRICES
    #define VIEW_MATRIX_NAME "aViewM"
#else
    #define VIEW_MATRIX_NAME "uViewM"
#endif

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"
#define COV_ATTR_NAME "aCoverage"
#define EDGE_ATTR_NAME "aEdge"
#define COL_UNI_NAME "uColor"
#define EDGES_UNI_NAME "uEdges"
#define COL_FILTER_UNI_NAME "uColorFilter"

namespace {
inline void tex_attr_name(int coordIdx, GrStringBuilder* s) {
    *s = "aTexCoord";
    s->appendS32(coordIdx);
}

inline GrGLShaderVar::Type float_vector_type(int count) {
    GR_STATIC_ASSERT(GrGLShaderVar::kFloat_Type == 0);
    GR_STATIC_ASSERT(GrGLShaderVar::kVec2f_Type == 1);
    GR_STATIC_ASSERT(GrGLShaderVar::kVec3f_Type == 2);
    GR_STATIC_ASSERT(GrGLShaderVar::kVec4f_Type == 3);
    GrAssert(count > 0 && count <= 4);
    return (GrGLShaderVar::Type)(count - 1);
}

inline const char* float_vector_type_str(int count) {
    return GrGLShaderVar::TypeString(float_vector_type(count));
}

inline const char* vector_homog_coord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

inline const char* vector_nonhomog_coords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
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
#if GR_GL_ATTRIBUTE_MATRICES
    *s = "aTexM";
#else
    *s = "uTexM";
#endif
    s->appendS32(stage);
}

inline void normalized_texel_size_name(int stage, GrStringBuilder* s) {
    *s = "uTexelSize";
    s->appendS32(stage);
}

inline void sampler_name(int stage, GrStringBuilder* s) {
    *s = "uSampler";
    s->appendS32(stage);
}

inline void radial2_param_name(int stage, GrStringBuilder* s) {
    *s = "uRadial2Params";
    s->appendS32(stage);
}

inline void convolve_param_names(int stage, GrStringBuilder* k, GrStringBuilder* i) {
    *k = "uKernel";
    k->appendS32(stage);
    *i = "uImageIncrement";
    i->appendS32(stage);
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
        *dstCoeff = (GrBlendCoeff)GrGpu::kIS2C_BlendCoeff;
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

namespace {

const char* glsl_version_string(const GrGLInterface* gl,
                                GrGLProgram::GLSLVersion v) {
    switch (v) {
        case GrGLProgram::k110_GLSLVersion:
            if (gl->supportsES()) {
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            } else {
                return "#version 110\n";
            }
        case GrGLProgram::k130_GLSLVersion:
            GrAssert(!gl->supportsES());
            return "#version 130\n";
        case GrGLProgram::k150_GLSLVersion:
            GrAssert(!gl->supportsES());
            return "#version 150\n";
        default:
            GrCrash("Unknown GL version.");
            return ""; // suppress warning
    }
}

// Adds a var that is computed in the VS and read in FS.
// If there is a GS it will just pass it through.
void append_varying(GrGLShaderVar::Type type,
                    const char* name,
                    ShaderCodeSegments* segments,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL) {
    segments->fVSOutputs.push_back();
    segments->fVSOutputs.back().setType(type);
    segments->fVSOutputs.back().accessName()->printf("v%s", name);
    if (vsOutName) {
        *vsOutName = segments->fVSOutputs.back().getName().c_str();
    }
    // input to FS comes either from VS or GS
    const GrStringBuilder* fsName;
    if (segments->fUsesGS) {
        // if we have a GS take each varying in as an array
        // and output as non-array.
        segments->fGSInputs.push_back();
        segments->fGSInputs.back().setType(type);
        segments->fGSInputs.back().setUnsizedArray();
        *segments->fGSInputs.back().accessName() =
            segments->fVSOutputs.back().getName();
        segments->fGSOutputs.push_back();
        segments->fGSOutputs.back().setType(type);
        segments->fGSOutputs.back().accessName()->printf("g%s", name);
        fsName = segments->fGSOutputs.back().accessName();
    } else {
        fsName = segments->fVSOutputs.back().accessName();
    }
    segments->fFSInputs.push_back();
    segments->fFSInputs.back().setType(type);
    segments->fFSInputs.back().setName(*fsName);
    if (fsInName) {
        *fsInName = fsName->c_str();
    }
}

// version of above that adds a stage number to the
// the var name (for uniqueness)
void append_varying(GrGLShaderVar::Type type,
                    const char* name,
                    int stageNum,
                    ShaderCodeSegments* segments,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL) {
    GrStringBuilder nameWithStage(name);
    nameWithStage.appendS32(stageNum);
    append_varying(type, nameWithStage.c_str(), segments, vsOutName, fsInName);
}
}

void GrGLProgram::genEdgeCoverage(const GrGLInterface* gl,
                                  GrVertexLayout layout,
                                  CachedData* programData,
                                  GrStringBuilder* coverageVar,
                                  ShaderCodeSegments* segments) const {
    if (fProgramDesc.fEdgeAANumEdges > 0) {
        segments->fFSUnis.push_back().set(GrGLShaderVar::kVec3f_Type,
                                          EDGES_UNI_NAME,
                                          fProgramDesc.fEdgeAANumEdges);
        programData->fUniLocations.fEdgesUni = kUseUniform;
        int count = fProgramDesc.fEdgeAANumEdges;
        segments->fFSCode.append(
            "\tvec3 pos = vec3(gl_FragCoord.xy, 1);\n");
        for (int i = 0; i < count; i++) {
            segments->fFSCode.append("\tfloat a");
            segments->fFSCode.appendS32(i);
            segments->fFSCode.append(" = clamp(dot(" EDGES_UNI_NAME "[");
            segments->fFSCode.appendS32(i);
            segments->fFSCode.append("], pos), 0.0, 1.0);\n");
        }
        if (fProgramDesc.fEdgeAAConcave && (count & 0x01) == 0) {
            // For concave polys, we consider the edges in pairs.
            segments->fFSFunctions.append("float cross2(vec2 a, vec2 b) {\n");
            segments->fFSFunctions.append("\treturn dot(a, vec2(b.y, -b.x));\n");
            segments->fFSFunctions.append("}\n");
            for (int i = 0; i < count; i += 2) {
                segments->fFSCode.appendf("\tfloat eb%d;\n", i / 2);
                segments->fFSCode.appendf("\tif (cross2(" EDGES_UNI_NAME "[%d].xy, " EDGES_UNI_NAME "[%d].xy) < 0.0) {\n", i, i + 1);
                segments->fFSCode.appendf("\t\teb%d = a%d * a%d;\n", i / 2, i, i + 1);
                segments->fFSCode.append("\t} else {\n");
                segments->fFSCode.appendf("\t\teb%d = a%d + a%d - a%d * a%d;\n", i / 2, i, i + 1, i, i + 1);
                segments->fFSCode.append("\t}\n");
            }
            segments->fFSCode.append("\tfloat edgeAlpha = ");
            for (int i = 0; i < count / 2 - 1; i++) {
                segments->fFSCode.appendf("min(eb%d, ", i);
            }
            segments->fFSCode.appendf("eb%d", count / 2 - 1);
            for (int i = 0; i < count / 2 - 1; i++) {
                segments->fFSCode.append(")");
            }
            segments->fFSCode.append(";\n");
        } else {
            segments->fFSCode.append("\tfloat edgeAlpha = ");
            for (int i = 0; i < count - 1; i++) {
                segments->fFSCode.appendf("min(a%d * a%d, ", i, i + 1);
            }
            segments->fFSCode.appendf("a%d * a0", count - 1);
            for (int i = 0; i < count - 1; i++) {
                segments->fFSCode.append(")");
            }
            segments->fFSCode.append(";\n");
        }
        *coverageVar = "edgeAlpha";
    } else  if (layout & GrDrawTarget::kEdge_VertexLayoutBit) {
        const char *vsName, *fsName;
        append_varying(GrGLShaderVar::kVec4f_Type, "Edge", segments, &vsName, &fsName);
        segments->fVSAttrs.push_back().set(GrGLShaderVar::kVec4f_Type, EDGE_ATTR_NAME);
        segments->fVSCode.appendf("\t%s = " EDGE_ATTR_NAME ";\n", vsName);
        if (GrDrawState::kHairLine_EdgeType == fProgramDesc.fVertexEdgeType) {
            segments->fFSCode.appendf("\tfloat edgeAlpha = abs(dot(vec3(gl_FragCoord.xy,1), %s.xyz));\n", fsName);
        } else {
            GrAssert(GrDrawState::kHairQuad_EdgeType == fProgramDesc.fVertexEdgeType);
            // for now we know we're not in perspective, so we could compute this
            // per-quadratic rather than per pixel
            segments->fFSCode.appendf("\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            segments->fFSCode.appendf("\tfloat dfdx = 2.0*%s.x*duvdx.x - duvdx.y;\n", fsName);
            segments->fFSCode.appendf("\tfloat dfdy = 2.0*%s.x*duvdy.x - duvdy.y;\n", fsName);
            segments->fFSCode.appendf("\tfloat edgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName, fsName);
            segments->fFSCode.append("\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / (dfdx*dfdx + dfdy*dfdy));\n");
            if (gl->supportsES()) {
                segments->fHeader.printf("#extension GL_OES_standard_derivatives: enable\n");
            }
        }
        segments->fFSCode.append("\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");
        *coverageVar = "edgeAlpha";
    } else {
        coverageVar->reset();
    }
}

namespace {

// returns true if the color output was explicitly declared or not.
bool decl_and_get_fs_color_output(GrGLProgram::GLSLVersion v,
                                  VarArray* fsOutputs,
                                  const char** name) {
    switch (v) {
        case GrGLProgram::k110_GLSLVersion:
            *name = "gl_FragColor";
            return false;
            break;
        case GrGLProgram::k130_GLSLVersion: // fallthru
        case GrGLProgram::k150_GLSLVersion:
            *name = declared_color_output_name();
            fsOutputs->push_back().set(GrGLShaderVar::kVec4f_Type,
                                       declared_color_output_name());
            return true;
            break;
        default:
            GrCrash("Unknown GLSL version.");
            return false; // suppress warning
    }
}

void genInputColor(GrGLProgram::ProgramDesc::ColorType colorType,
                   GrGLProgram::CachedData* programData,
                   ShaderCodeSegments* segments,
                   GrStringBuilder* inColor) {
    switch (colorType) {
        case GrGLProgram::ProgramDesc::kAttribute_ColorType: {
            segments->fVSAttrs.push_back().set(GrGLShaderVar::kVec4f_Type,
                                               COL_ATTR_NAME);
            const char *vsName, *fsName;
            append_varying(GrGLShaderVar::kVec4f_Type, "Color", segments, &vsName, &fsName);
            segments->fVSCode.appendf("\t%s = " COL_ATTR_NAME ";\n", vsName);
            *inColor = fsName;
            } break;
        case GrGLProgram::ProgramDesc::kUniform_ColorType:
            segments->fFSUnis.push_back().set(GrGLShaderVar::kVec4f_Type,
                                              COL_UNI_NAME);
            programData->fUniLocations.fColorUni = kUseUniform;
            *inColor = COL_UNI_NAME;
            break;
        case GrGLProgram::ProgramDesc::kTransBlack_ColorType:
            GrAssert(!"needComputedColor should be false.");
            break;
        case GrGLProgram::ProgramDesc::kSolidWhite_ColorType:
            break;
        default:
            GrCrash("Unknown color type.");
            break;
    }
}

void genPerVertexCoverage(ShaderCodeSegments* segments,
                          GrStringBuilder* inCoverage) {
    segments->fVSAttrs.push_back().set(GrGLShaderVar::kFloat_Type,
                                       COV_ATTR_NAME);
    const char *vsName, *fsName;
    append_varying(GrGLShaderVar::kFloat_Type, "Coverage", 
                   segments, &vsName, &fsName);
    segments->fVSCode.appendf("\t%s = " COV_ATTR_NAME ";\n", vsName);
    if (inCoverage->size()) {
        segments->fFSCode.appendf("\tfloat edgeAndAttrCov = %s * %s;\n",
                                  fsName, inCoverage->c_str());
        *inCoverage = "edgeAndAttrCov";
    } else {
        *inCoverage = fsName;
    }
}

}

void GrGLProgram::genGeometryShader(const GrGLInterface* gl,
                                    GLSLVersion glslVersion,
                                    ShaderCodeSegments* segments) const {
#if GR_GL_EXPERIMENTAL_GS
    if (fProgramDesc.fExperimentalGS) {
        GrAssert(glslVersion >= k150_GLSLVersion);
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

bool GrGLProgram::genProgram(const GrGLInterface* gl,
                             GLSLVersion glslVersion,
                             GrGLProgram::CachedData* programData) const {

    ShaderCodeSegments segments;
    const uint32_t& layout = fProgramDesc.fVertexLayout;

    programData->fUniLocations.reset();

#if GR_GL_EXPERIMENTAL_GS
    segments.fUsesGS = fProgramDesc.fExperimentalGS;
#endif

    SkXfermode::Coeff colorCoeff, uniformCoeff;
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

    // If we know the final color is going to be all zeros then we can
    // simplify the color filter coeffecients. needComputedColor will then
    // come out false below.
    if (ProgramDesc::kTransBlack_ColorType == fProgramDesc.fColorType) {
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
    const char* fsColorOutput = NULL;
    bool dualSourceOutputWritten = false;
    segments.fHeader.printf(glsl_version_string(gl, glslVersion));
    bool isColorDeclared = decl_and_get_fs_color_output(glslVersion,
                                                        &segments.fFSOutputs,
                                                        &fsColorOutput);

#if GR_GL_ATTRIBUTE_MATRICES
    segments.fVSAttrs.push_back().set(GrGLShaderVar::kMat33f_Type, VIEW_MATRIX_NAME);
    programData->fUniLocations.fViewMatrixUni = kSetAsAttribute;
#else
    segments.fVSUnis.push_back().set(GrGLShaderVar::kMat33f_Type, VIEW_MATRIX_NAME);
    programData->fUniLocations.fViewMatrixUni = kUseUniform;
#endif
    segments.fVSAttrs.push_back().set(GrGLShaderVar::kVec2f_Type, POS_ATTR_NAME);

    segments.fVSCode.append(
        "void main() {\n"
            "\tvec3 pos3 = " VIEW_MATRIX_NAME " * vec3("POS_ATTR_NAME", 1);\n"
            "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n");

    // incoming color to current stage being processed.
    GrStringBuilder inColor;

    if (needComputedColor) {
        genInputColor((ProgramDesc::ColorType) fProgramDesc.fColorType,
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
            segments.fVSAttrs.push_back().set(GrGLShaderVar::kVec2f_Type,
                                              texCoordAttrs[t].c_str());
        }
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

                this->genStageCode(gl,
                                   s,
                                   fProgramDesc.fStages[s],
                                   inColor.size() ? inColor.c_str() : NULL,
                                   outColor.c_str(),
                                   inCoords,
                                   &segments,
                                   &programData->fUniLocations.fStages[s]);
                inColor = outColor;
            }
        }
    }

    // if have all ones or zeros for the "dst" input to the color filter then we
    // may be able to make additional optimizations.
    if (needColorFilterUniform && needComputedColor && !inColor.size()) {
        GrAssert(ProgramDesc::kSolidWhite_ColorType == fProgramDesc.fColorType);
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
        segments.fFSUnis.push_back().set(GrGLShaderVar::kVec4f_Type,
                                         COL_FILTER_UNI_NAME);
        programData->fUniLocations.fColorFilterUni = kUseUniform;
    }

    bool wroteFragColorZero = false;
    if (SkXfermode::kZero_Coeff == uniformCoeff &&
        SkXfermode::kZero_Coeff == colorCoeff) {
        segments.fFSCode.appendf("\t%s = %s;\n",
                                 fsColorOutput,
                                 all_zeros_vec(4));
        wroteFragColorZero = true;
    } else if (SkXfermode::kDst_Mode != fProgramDesc.fColorFilterXfermode) {
        segments.fFSCode.appendf("\tvec4 filteredColor;\n");
        const char* color;
        if (inColor.size()) {
            color = inColor.c_str();
        } else {
            if (ProgramDesc::kSolidWhite_ColorType == fProgramDesc.fColorType) {
                color = all_ones_vec(4);
            } else {
                color = all_zeros_vec(4);
            }
        }
        addColorFilter(&segments.fFSCode, "filteredColor", uniformCoeff,
                       colorCoeff, color);
        inColor = "filteredColor";
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage (coverage stages and edge aa)

    GrStringBuilder inCoverage;

    // we don't need to compute coverage at all if we know the final shader
    // output will be zero and we don't have a dual src blend output.
    if (!wroteFragColorZero ||
        ProgramDesc::kNone_DualSrcOutput != fProgramDesc.fDualSrcOutput) {

        // get edge AA coverage and use it as inCoverage to first coverage stage
        this->genEdgeCoverage(gl, layout, programData, &inCoverage, &segments);

        // include explicit per-vertex coverage if we have it
        if (GrDrawTarget::kCoverage_VertexLayoutBit & layout) {
            genPerVertexCoverage(&segments, &inCoverage);
        }

        GrStringBuilder outCoverage;
        const int& startStage = fProgramDesc.fFirstCoverageStage;
        for (int s = startStage; s < GrDrawState::kNumStages; ++s) {
            if (fProgramDesc.fStages[s].isEnabled()) {
                // create var to hold stage output
                outCoverage = "coverage";
                outCoverage.appendS32(s);
                segments.fFSCode.appendf("\tvec4 %s;\n", outCoverage.c_str());

                const char* inCoords;
                // figure out what our input coords are
                if (GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s) & layout) {
                    inCoords = POS_ATTR_NAME;
                } else {
                    int tcIdx = GrDrawTarget::VertexTexCoordsForStage(s, layout);
                        // we better have input tex coordinates if stage is enabled.
                    GrAssert(tcIdx >= 0);
                    GrAssert(texCoordAttrs[tcIdx].size());
                    inCoords = texCoordAttrs[tcIdx].c_str();
                }

                genStageCode(gl, s,
                             fProgramDesc.fStages[s],
                             inCoverage.size() ? inCoverage.c_str() : NULL,
                             outCoverage.c_str(),
                             inCoords,
                             &segments,
                             &programData->fUniLocations.fStages[s]);
                inCoverage = outCoverage;
            }
        }
        if (ProgramDesc::kNone_DualSrcOutput != fProgramDesc.fDualSrcOutput) {
            segments.fFSOutputs.push_back().set(GrGLShaderVar::kVec4f_Type,
                                                dual_source_output_name());
            bool outputIsZero = false;
            GrStringBuilder coeff;
            if (ProgramDesc::kCoverage_DualSrcOutput !=
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
        modulate_helper(fsColorOutput,
                        inColor.c_str(),
                        inCoverage.c_str(),
                        &segments.fFSCode);
    }

    segments.fVSCode.append("}\n");
    segments.fFSCode.append("}\n");

    ///////////////////////////////////////////////////////////////////////////
    // insert GS
#if GR_DEBUG
    this->genGeometryShader(gl, glslVersion, &segments);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // compile and setup attribs and unis

    if (!CompileShaders(gl, glslVersion, segments, programData)) {
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
                         const GrGLInterface* gl,
                         const char* prefix,
                         GrStringBuilder* string) {
    const int count = vars.count();
    for (int i = 0; i < count; ++i) {
        string->append(prefix);
        string->append(" ");
        vars[i].appendDecl(gl, string);
        string->append(";\n");
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
                         const GrGLInterface* gl,
                         const char* prefix,
                         StrArray* strings,
                         LengthArray* lengths,
                         TempArray* temp) {
    expand_decls(vars, gl, prefix, &temp->push_back());
    append_string(temp->back(), strings, lengths);
}

}

bool GrGLProgram::CompileShaders(const GrGLInterface* gl,
                                 GLSLVersion glslVersion,
                                 const ShaderCodeSegments& segments,
                                 CachedData* programData) {
    enum { kPreAllocStringCnt = 8 };

    PREALLOC_STR_ARRAY(kPreAllocStringCnt)    strs;
    PREALLOC_LENGTH_ARRAY(kPreAllocStringCnt) lengths;
    PREALLOC_TEMP_ARRAY(kPreAllocStringCnt)   temps;

    GrStringBuilder unis;
    GrStringBuilder inputs;
    GrStringBuilder outputs;

    static const char* gVaryingPrefixes[2][2] = {{"varying", "varying"},
                                                 {"out", "in"}};
    const char** varyingPrefixes = k110_GLSLVersion == glslVersion ?
                                                    gVaryingPrefixes[0] :
                                                    gVaryingPrefixes[1];
    const char* attributePrefix = k110_GLSLVersion == glslVersion ?
                                                    "attribute" :
                                                    "in";

    append_string(segments.fHeader, &strs, &lengths);
    append_decls(segments.fVSUnis, gl, "uniform", &strs, &lengths, &temps);
    append_decls(segments.fVSAttrs, gl, attributePrefix, &strs, &lengths, &temps);
    append_decls(segments.fVSOutputs, gl, varyingPrefixes[0], &strs, &lengths, &temps);
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
        append_decls(segments.fGSInputs, gl, "in", &strs, &lengths, &temps);
        append_decls(segments.fGSOutputs, gl, "out", &strs, &lengths, &temps);
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
    GrStringBuilder precisionStr(GrShaderPrecision(gl));
    append_string(precisionStr, &strs, &lengths);
    append_decls(segments.fFSUnis, gl, "uniform", &strs, &lengths, &temps);
    append_decls(segments.fFSInputs, gl, varyingPrefixes[1], &strs, &lengths, &temps);
    // We shouldn't have declared outputs on 1.10
    GrAssert(k110_GLSLVersion != glslVersion || segments.fFSOutputs.empty());
    append_decls(segments.fFSOutputs, gl, "out", &strs, &lengths, &temps);
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

GrGLuint GrGLProgram::CompileShader(const GrGLInterface* gl,
                                    GrGLenum type,
                                    int stringCnt,
                                    const char** strings,
                                    int* stringLengths) {
    SK_TRACE_EVENT1("GrGLProgram::CompileShader",
                    "stringCount", SkStringPrintf("%i", stringCnt).c_str());

    GrGLuint shader;
    GR_GL_CALL_RET(gl, shader, CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    GrGLint compiled = GR_GL_INIT_ZERO;
    GR_GL_CALL(gl, ShaderSource(shader, stringCnt, strings, stringLengths));
    GR_GL_CALL(gl, CompileShader(shader));
    GR_GL_CALL(gl, GetShaderiv(shader, GR_GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GR_GL_CALL(gl, GetShaderiv(shader, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GR_GL_CALL(gl, GetShaderInfoLog(shader, infoLen+1, 
                                            &length, (char*)log.get()));
            print_shader(stringCnt, strings, stringLengths);
            GrPrintf("\n%s", log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GR_GL_CALL(gl, DeleteShader(shader));
        return 0;
    }
    return shader;
}

bool GrGLProgram::bindOutputsAttribsAndLinkProgram(
                                        const GrGLInterface* gl,
                                        GrStringBuilder texCoordAttrNames[],
                                        bool bindColorOut,
                                        bool bindDualSrcOut,
                                        CachedData* programData) const {
    GR_GL_CALL_RET(gl, programData->fProgramID, CreateProgram());
    if (!programData->fProgramID) {
        return false;
    }
    const GrGLint& progID = programData->fProgramID;

    GR_GL_CALL(gl, AttachShader(progID, programData->fVShaderID));
    if (programData->fGShaderID) {
        GR_GL_CALL(gl, AttachShader(progID, programData->fGShaderID));
    }
    GR_GL_CALL(gl, AttachShader(progID, programData->fFShaderID));

    if (bindColorOut) {
        GR_GL_CALL(gl, BindFragDataLocation(programData->fProgramID,
                                          0, declared_color_output_name()));
    }
    if (bindDualSrcOut) {
        GR_GL_CALL(gl, BindFragDataLocationIndexed(programData->fProgramID,
                                          0, 1, dual_source_output_name()));
    }

    // Bind the attrib locations to same values for all shaders
    GR_GL_CALL(gl, BindAttribLocation(progID, PositionAttributeIdx(),
                                      POS_ATTR_NAME));
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (texCoordAttrNames[t].size()) {
            GR_GL_CALL(gl, BindAttribLocation(progID,
                                              TexCoordAttributeIdx(t),
                                              texCoordAttrNames[t].c_str()));
        }
    }

    if (kSetAsAttribute == programData->fUniLocations.fViewMatrixUni) {
        GR_GL_CALL(gl, BindAttribLocation(progID,
                                          ViewMatrixAttributeIdx(),
                                          VIEW_MATRIX_NAME));
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        const StageUniLocations& unis = programData->fUniLocations.fStages[s];
        if (kSetAsAttribute == unis.fTextureMatrixUni) {
            GrStringBuilder matName;
            tex_matrix_name(s, &matName);
            GR_GL_CALL(gl, BindAttribLocation(progID,
                                              TextureMatrixAttributeIdx(s),
                                              matName.c_str()));
        }
    }

    GR_GL_CALL(gl, BindAttribLocation(progID, ColorAttributeIdx(),
                                      COL_ATTR_NAME));
    GR_GL_CALL(gl, BindAttribLocation(progID, CoverageAttributeIdx(),
                                      COV_ATTR_NAME));
    GR_GL_CALL(gl, BindAttribLocation(progID, EdgeAttributeIdx(),
                                      EDGE_ATTR_NAME));

    GR_GL_CALL(gl, LinkProgram(progID));

    GrGLint linked = GR_GL_INIT_ZERO;
    GR_GL_CALL(gl, GetProgramiv(progID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GR_GL_CALL(gl, GetProgramiv(progID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GR_GL_CALL(gl, GetProgramInfoLog(progID, infoLen+1,
                                             &length, (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GR_GL_CALL(gl, DeleteProgram(progID));
        programData->fProgramID = 0;
        return false;
    }
    return true;
}

void GrGLProgram::getUniformLocationsAndInitCache(const GrGLInterface* gl, 
                                                  CachedData* programData) const {
    const GrGLint& progID = programData->fProgramID;

    if (kUseUniform == programData->fUniLocations.fViewMatrixUni) {
        GR_GL_CALL_RET(gl, programData->fUniLocations.fViewMatrixUni,
                       GetUniformLocation(progID, VIEW_MATRIX_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fViewMatrixUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorUni) {
        GR_GL_CALL_RET(gl, programData->fUniLocations.fColorUni,
                       GetUniformLocation(progID, COL_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorFilterUni) {
        GR_GL_CALL_RET(gl, programData->fUniLocations.fColorFilterUni, 
                       GetUniformLocation(progID, COL_FILTER_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorFilterUni);
    }

    if (kUseUniform == programData->fUniLocations.fEdgesUni) {
        GR_GL_CALL_RET(gl, programData->fUniLocations.fEdgesUni,
                       GetUniformLocation(progID, EDGES_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fEdgesUni);
    } else {
        programData->fUniLocations.fEdgesUni = kUnusedUniform;
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        StageUniLocations& locations = programData->fUniLocations.fStages[s];
        if (fProgramDesc.fStages[s].isEnabled()) {
            if (kUseUniform == locations.fTextureMatrixUni) {
                GrStringBuilder texMName;
                tex_matrix_name(s, &texMName);
                GR_GL_CALL_RET(gl, locations.fTextureMatrixUni,
                               GetUniformLocation(progID, texMName.c_str()));
                GrAssert(kUnusedUniform != locations.fTextureMatrixUni);
            }

            if (kUseUniform == locations.fSamplerUni) {
                GrStringBuilder samplerName;
                sampler_name(s, &samplerName);
                GR_GL_CALL_RET(gl, locations.fSamplerUni,
                               GetUniformLocation(progID,samplerName.c_str()));
                GrAssert(kUnusedUniform != locations.fSamplerUni);
            }

            if (kUseUniform == locations.fNormalizedTexelSizeUni) {
                GrStringBuilder texelSizeName;
                normalized_texel_size_name(s, &texelSizeName);
                GR_GL_CALL_RET(gl, locations.fNormalizedTexelSizeUni,
                               GetUniformLocation(progID, texelSizeName.c_str()));
                GrAssert(kUnusedUniform != locations.fNormalizedTexelSizeUni);
            }

            if (kUseUniform == locations.fRadial2Uni) {
                GrStringBuilder radial2ParamName;
                radial2_param_name(s, &radial2ParamName);
                GR_GL_CALL_RET(gl, locations.fRadial2Uni,
                               GetUniformLocation(progID, radial2ParamName.c_str()));
                GrAssert(kUnusedUniform != locations.fRadial2Uni);
            }

            if (kUseUniform == locations.fTexDomUni) {
                GrStringBuilder texDomName;
                tex_domain_name(s, &texDomName);
                GR_GL_CALL_RET(gl, locations.fTexDomUni,
                               GetUniformLocation(progID, texDomName.c_str()));
                GrAssert(kUnusedUniform != locations.fTexDomUni);
            }

            GrStringBuilder kernelName, imageIncrementName;
            convolve_param_names(s, &kernelName, &imageIncrementName);
            if (kUseUniform == locations.fKernelUni) {
                GR_GL_CALL_RET(gl, locations.fKernelUni, 
                               GetUniformLocation(progID, kernelName.c_str()));
                GrAssert(kUnusedUniform != locations.fKernelUni);
            }

            if (kUseUniform == locations.fImageIncrementUni) {
                GR_GL_CALL_RET(gl, locations.fImageIncrementUni, 
                               GetUniformLocation(progID, 
                                                  imageIncrementName.c_str()));
                GrAssert(kUnusedUniform != locations.fImageIncrementUni);
            }
        }
    }
    GR_GL_CALL(gl, UseProgram(progID));

    // init sampler unis and set bogus values for state tracking
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (kUnusedUniform != programData->fUniLocations.fStages[s].fSamplerUni) {
            GR_GL_CALL(gl, Uniform1i(programData->fUniLocations.fStages[s].fSamplerUni, s));
        }
        programData->fTextureMatrices[s] = GrMatrix::InvalidMatrix();
        programData->fRadial2CenterX1[s] = GR_ScalarMax;
        programData->fRadial2Radius0[s] = -GR_ScalarMax;
        programData->fTextureWidth[s] = -1;
        programData->fTextureHeight[s] = -1;
    }
    programData->fViewMatrix = GrMatrix::InvalidMatrix();
    programData->fColor = GrColor_ILLEGAL;
    programData->fColorFilterColor = GrColor_ILLEGAL;
}

//============================================================================
// Stage code generation
//============================================================================

namespace {

bool isRadialMapping(GrGLProgram::StageDesc::CoordMapping mapping) {
    return
       (GrGLProgram::StageDesc::kRadial2Gradient_CoordMapping == mapping ||
        GrGLProgram::StageDesc::kRadial2GradientDegenerate_CoordMapping == mapping);
}

const char* genRadialVS(int stageNum,
                        ShaderCodeSegments* segments,
                        GrGLProgram::StageUniLocations* locations,
                        const char** radial2VaryingVSName,
                        const char** radial2VaryingFSName,
                        const char* varyingVSName,
                        int varyingDims, int coordDims) {

    GrGLShaderVar* radial2FSParams = &segments->fFSUnis.push_back();
    radial2FSParams->setType(GrGLShaderVar::kFloat_Type);
    radial2FSParams->setArrayCount(6);
    radial2_param_name(stageNum, radial2FSParams->accessName());
    segments->fVSUnis.push_back(*radial2FSParams).setEmitPrecision(true);

    locations->fRadial2Uni = kUseUniform;

    // for radial grads without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (varyingDims == coordDims) {
        GrAssert(2 == coordDims);
        append_varying(GrGLShaderVar::kFloat_Type,
                       "Radial2BCoeff",
                       stageNum,
                       segments,
                       radial2VaryingVSName,
                       radial2VaryingFSName);

        // r2Var = 2 * (r2Parm[2] * varCoord.x - r2Param[3])
        const char* r2ParamName = radial2FSParams->getName().c_str();
        segments->fVSCode.appendf("\t%s = 2.0 *(%s[2] * %s.x - %s[3]);\n",
                                  *radial2VaryingVSName, r2ParamName,
                                  varyingVSName, r2ParamName);
    }

    return radial2FSParams->getName().c_str();
}

bool genRadial2GradientCoordMapping(int stageNum,
                                    ShaderCodeSegments* segments,
                                    const char* radial2VaryingFSName,
                                    const char* radial2ParamsName,
                                    GrStringBuilder& sampleCoords,
                                    GrStringBuilder& fsCoordName,
                                    int varyingDims,
                                    int coordDims) {
    GrStringBuilder cName("c");
    GrStringBuilder ac4Name("ac4");
    GrStringBuilder rootName("root");

    cName.appendS32(stageNum);
    ac4Name.appendS32(stageNum);
    rootName.appendS32(stageNum);

    // if we were able to interpolate the linear component bVar is the varying
    // otherwise compute it
    GrStringBuilder bVar;
    if (coordDims == varyingDims) {
        bVar = radial2VaryingFSName;
        GrAssert(2 == varyingDims);
    } else {
        GrAssert(3 == varyingDims);
        bVar = "b";
        bVar.appendS32(stageNum);
        segments->fFSCode.appendf("\tfloat %s = 2.0 * (%s[2] * %s.x - %s[3]);\n",
                                    bVar.c_str(), radial2ParamsName,
                                    fsCoordName.c_str(), radial2ParamsName);
    }

    // c = (x^2)+(y^2) - params[4]
    segments->fFSCode.appendf("\tfloat %s = dot(%s, %s) - %s[4];\n",
                              cName.c_str(), fsCoordName.c_str(),
                              fsCoordName.c_str(),
                              radial2ParamsName);
    // ac4 = 4.0 * params[0] * c
    segments->fFSCode.appendf("\tfloat %s = %s[0] * 4.0 * %s;\n",
                              ac4Name.c_str(), radial2ParamsName,
                              cName.c_str());

    // root = sqrt(b^2-4ac)
    // (abs to avoid exception due to fp precision)
    segments->fFSCode.appendf("\tfloat %s = sqrt(abs(%s*%s - %s));\n",
                              rootName.c_str(), bVar.c_str(), bVar.c_str(),
                              ac4Name.c_str());

    // x coord is: (-b + params[5] * sqrt(b^2-4ac)) * params[1]
    // y coord is 0.5 (texture is effectively 1D)
    sampleCoords.printf("vec2((-%s + %s[5] * %s) * %s[1], 0.5)",
                        bVar.c_str(), radial2ParamsName,
                        rootName.c_str(), radial2ParamsName);
    return true;
}

bool genRadial2GradientDegenerateCoordMapping(int stageNum,
                                              ShaderCodeSegments* segments,
                                              const char* radial2VaryingFSName,
                                              const char* radial2ParamsName,
                                              GrStringBuilder& sampleCoords,
                                              GrStringBuilder& fsCoordName,
                                              int varyingDims,
                                              int coordDims) {
    GrStringBuilder cName("c");

    cName.appendS32(stageNum);

    // if we were able to interpolate the linear component bVar is the varying
    // otherwise compute it
    GrStringBuilder bVar;
    if (coordDims == varyingDims) {
        bVar = radial2VaryingFSName;
        GrAssert(2 == varyingDims);
    } else {
        GrAssert(3 == varyingDims);
        bVar = "b";
        bVar.appendS32(stageNum);
        segments->fFSCode.appendf("\tfloat %s = 2.0 * (%s[2] * %s.x - %s[3]);\n",
                                    bVar.c_str(), radial2ParamsName,
                                    fsCoordName.c_str(), radial2ParamsName);
    }

    // c = (x^2)+(y^2) - params[4]
    segments->fFSCode.appendf("\tfloat %s = dot(%s, %s) - %s[4];\n",
                              cName.c_str(), fsCoordName.c_str(),
                              fsCoordName.c_str(),
                              radial2ParamsName);

    // x coord is: -c/b
    // y coord is 0.5 (texture is effectively 1D)
    sampleCoords.printf("vec2((-%s / %s), 0.5)", cName.c_str(), bVar.c_str());
    return true;
}

void gen2x2FS(int stageNum,
              ShaderCodeSegments* segments,
              GrGLProgram::StageUniLocations* locations,
              GrStringBuilder* sampleCoords,
              const char* samplerName,
              const char* texelSizeName,
              const char* smear,
              const char* fsOutColor,
              GrStringBuilder& texFunc,
              GrStringBuilder& modulate,
              bool complexCoord,
              int coordDims) {
    locations->fNormalizedTexelSizeUni = kUseUniform;
    if (complexCoord) {
        // assign the coord to a var rather than compute 4x.
        GrStringBuilder coordVar("tCoord");
        coordVar.appendS32(stageNum);
        segments->fFSCode.appendf("\t%s %s = %s;\n",
                            float_vector_type_str(coordDims),
                            coordVar.c_str(), sampleCoords->c_str());
        *sampleCoords = coordVar;
    }
    GrAssert(2 == coordDims);
    GrStringBuilder accumVar("accum");
    accumVar.appendS32(stageNum);
    segments->fFSCode.appendf("\tvec4 %s  = %s(%s, %s + vec2(-%s.x,-%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName, sampleCoords->c_str(), texelSizeName, texelSizeName, smear);
    segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(+%s.x,-%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName, sampleCoords->c_str(), texelSizeName, texelSizeName, smear);
    segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(-%s.x,+%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName, sampleCoords->c_str(), texelSizeName, texelSizeName, smear);
    segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(+%s.x,+%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName, sampleCoords->c_str(), texelSizeName, texelSizeName, smear);
    segments->fFSCode.appendf("\t%s = .25 * %s%s;\n", fsOutColor, accumVar.c_str(), modulate.c_str());

}

void genConvolutionVS(int stageNum,
                      const GrGLProgram::ProgramDesc::StageDesc& desc,
                      ShaderCodeSegments* segments,
                      GrGLProgram::StageUniLocations* locations,
                      const char** kernelName,
                      const char** imageIncrementName,
                      const char* varyingVSName) {
    GrGLShaderVar* kernel = &segments->fFSUnis.push_back();
    kernel->setType(GrGLShaderVar::kFloat_Type);
    kernel->setArrayCount(desc.fKernelWidth);
    GrGLShaderVar* imgInc = &segments->fFSUnis.push_back();
    imgInc->setType(GrGLShaderVar::kVec2f_Type);

    convolve_param_names(stageNum,
                         kernel->accessName(),
                         imgInc->accessName());
    *kernelName = kernel->getName().c_str();
    *imageIncrementName = imgInc->getName().c_str();

    // need image increment in both VS and FS
    segments->fVSUnis.push_back(*imgInc).setEmitPrecision(true);

    locations->fKernelUni = kUseUniform;
    locations->fImageIncrementUni = kUseUniform;
    float scale = (desc.fKernelWidth - 1) * 0.5f;
    segments->fVSCode.appendf("\t%s -= vec2(%g, %g) * %s;\n",
                                  varyingVSName, scale, scale,
                                  *imageIncrementName);
}

void genConvolutionFS(int stageNum,
                      const GrGLProgram::ProgramDesc::StageDesc& desc,
                      ShaderCodeSegments* segments,
                      const char* samplerName,
                      const char* kernelName,
                      const char* smear,
                      const char* imageIncrementName,
                      const char* fsOutColor,
                      GrStringBuilder& sampleCoords,
                      GrStringBuilder& texFunc,
                      GrStringBuilder& modulate) {
    GrStringBuilder sumVar("sum");
    sumVar.appendS32(stageNum);
    GrStringBuilder coordVar("coord");
    coordVar.appendS32(stageNum);

    segments->fFSCode.appendf("\tvec4 %s = vec4(0, 0, 0, 0);\n",
                              sumVar.c_str());
    segments->fFSCode.appendf("\tvec2 %s = %s;\n", 
                              coordVar.c_str(),
                              sampleCoords.c_str());
    segments->fFSCode.appendf("\tfor (int i = 0; i < %d; i++) {\n",
                              desc.fKernelWidth);
    segments->fFSCode.appendf("\t\t%s += %s(%s, %s)%s * %s[i];\n",
                              sumVar.c_str(), texFunc.c_str(),
                              samplerName, coordVar.c_str(), smear,
                              kernelName);
    segments->fFSCode.appendf("\t\t%s += %s;\n",
                              coordVar.c_str(),
                              imageIncrementName);
    segments->fFSCode.appendf("\t}\n");
    segments->fFSCode.appendf("\t%s = %s%s;\n", fsOutColor,
                              sumVar.c_str(), modulate.c_str());
}

}

void GrGLProgram::genStageCode(const GrGLInterface* gl,
                               int stageNum,
                               const GrGLProgram::StageDesc& desc,
                               const char* fsInColor, // NULL means no incoming color
                               const char* fsOutColor,
                               const char* vsInCoord,
                               ShaderCodeSegments* segments,
                               StageUniLocations* locations) const {

    GrAssert(stageNum >= 0 && stageNum <= 9);

    // First decide how many coords are needed to access the texture
    // Right now it's always 2 but we could start using 1D textures for
    // gradients.
    static const int coordDims = 2;
    int varyingDims;
    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords
    // and whether the varying needs a perspective coord.
    const char* matName = NULL;
    if (desc.fOptFlags & StageDesc::kIdentityMatrix_OptFlagBit) {
        varyingDims = coordDims;
    } else {
        GrGLShaderVar* mat;
    #if GR_GL_ATTRIBUTE_MATRICES
        mat = &segments->fVSAttrs.push_back();
        locations->fTextureMatrixUni = kSetAsAttribute;
    #else
        mat = &segments->fVSUnis.push_back();
        locations->fTextureMatrixUni = kUseUniform;
    #endif
        tex_matrix_name(stageNum, mat->accessName());
        mat->setType(GrGLShaderVar::kMat33f_Type);
        matName = mat->getName().c_str();

        if (desc.fOptFlags & StageDesc::kNoPerspective_OptFlagBit) {
            varyingDims = coordDims;
        } else {
            varyingDims = coordDims + 1;
        }
    }

    segments->fFSUnis.push_back().setType(GrGLShaderVar::kSampler2D_Type);
    sampler_name(stageNum, segments->fFSUnis.back().accessName());
    locations->fSamplerUni = kUseUniform;
    const char* samplerName = segments->fFSUnis.back().getName().c_str();

    const char* texelSizeName = NULL;
    if (StageDesc::k2x2_FetchMode == desc.fFetchMode) {
        segments->fFSUnis.push_back().setType(GrGLShaderVar::kVec2f_Type);
        normalized_texel_size_name(stageNum, segments->fFSUnis.back().accessName());
        texelSizeName = segments->fFSUnis.back().getName().c_str();
    }

    const char *varyingVSName, *varyingFSName;
    append_varying(float_vector_type(varyingDims),
                    "Stage",
                   stageNum,
                   segments,
                   &varyingVSName,
                   &varyingFSName);

    if (!matName) {
        GrAssert(varyingDims == coordDims);
        segments->fVSCode.appendf("\t%s = %s;\n", varyingVSName, vsInCoord);
    } else {
        // varying = texMatrix * texCoord
        segments->fVSCode.appendf("\t%s = (%s * vec3(%s, 1))%s;\n",
                                  varyingVSName, matName, vsInCoord,
                                  vector_all_coords(varyingDims));
    }

    const char* radial2ParamsName = NULL;
    const char *radial2VaryingVSName = NULL;
    const char *radial2VaryingFSName = NULL;

    if (isRadialMapping((StageDesc::CoordMapping) desc.fCoordMapping)) {
        radial2ParamsName = genRadialVS(stageNum, segments,
                                        locations,
                                        &radial2VaryingVSName,
                                        &radial2VaryingFSName,
                                        varyingVSName,
                                        varyingDims, coordDims);
    }

    const char* kernelName = NULL;
    const char* imageIncrementName = NULL;
    if (ProgramDesc::StageDesc::kConvolution_FetchMode == desc.fFetchMode) {
        genConvolutionVS(stageNum, desc, segments, locations,
                         &kernelName, &imageIncrementName, varyingVSName);
    }

    /// Fragment Shader Stuff
    GrStringBuilder fsCoordName;
    // function used to access the shader, may be made projective
    GrStringBuilder texFunc("texture2D");
    if (desc.fOptFlags & (StageDesc::kIdentityMatrix_OptFlagBit |
                          StageDesc::kNoPerspective_OptFlagBit)) {
        GrAssert(varyingDims == coordDims);
        fsCoordName = varyingFSName;
    } else {
        // if we have to do some special op on the varyings to get
        // our final tex coords then when in perspective we have to
        // do an explicit divide. Otherwise, we can use a Proj func.
        if  (StageDesc::kIdentity_CoordMapping == desc.fCoordMapping &&
             StageDesc::kSingle_FetchMode == desc.fFetchMode) {
            texFunc.append("Proj");
            fsCoordName = varyingFSName;
        } else {
            fsCoordName = "inCoord";
            fsCoordName.appendS32(stageNum);
            segments->fFSCode.appendf("\t%s %s = %s%s / %s%s;\n",
                                GrGLShaderVar::TypeString(float_vector_type(coordDims)),
                                fsCoordName.c_str(),
                                varyingFSName,
                                vector_nonhomog_coords(varyingDims),
                                varyingFSName,
                                vector_homog_coord(varyingDims));
        }
    }

    GrStringBuilder sampleCoords;
    bool complexCoord = false;
    switch (desc.fCoordMapping) {
    case StageDesc::kIdentity_CoordMapping:
        sampleCoords = fsCoordName;
        break;
    case StageDesc::kSweepGradient_CoordMapping:
        sampleCoords.printf("vec2(atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5, 0.5)", fsCoordName.c_str(), fsCoordName.c_str());
        complexCoord = true;
        break;
    case StageDesc::kRadialGradient_CoordMapping:
        sampleCoords.printf("vec2(length(%s.xy), 0.5)", fsCoordName.c_str());
        complexCoord = true;
        break;
    case StageDesc::kRadial2Gradient_CoordMapping:
        complexCoord = genRadial2GradientCoordMapping(
                           stageNum, segments,
                           radial2VaryingFSName, radial2ParamsName,
                           sampleCoords, fsCoordName,
                           varyingDims, coordDims);

        break;
    case StageDesc::kRadial2GradientDegenerate_CoordMapping:
        complexCoord = genRadial2GradientDegenerateCoordMapping(
                           stageNum, segments,
                           radial2VaryingFSName, radial2ParamsName,
                           sampleCoords, fsCoordName,
                           varyingDims, coordDims);
        break;

    };

    const char* smear;
    if (desc.fModulation == StageDesc::kAlpha_Modulation) {
        smear = ".aaaa";
    } else {
        smear = "";
    }
    GrStringBuilder modulate;
    if (NULL != fsInColor) {
        modulate.printf(" * %s", fsInColor);
    }

    if (desc.fOptFlags &
        StageDesc::kCustomTextureDomain_OptFlagBit) {
        GrStringBuilder texDomainName;
        tex_domain_name(stageNum, &texDomainName);
        segments->fFSUnis.push_back().set(GrGLShaderVar::kVec4f_Type, texDomainName);
        GrStringBuilder coordVar("clampCoord");
        segments->fFSCode.appendf("\t%s %s = clamp(%s, %s.xy, %s.zw);\n",
                                  float_vector_type_str(coordDims),
                                  coordVar.c_str(),
                                  sampleCoords.c_str(),
                                  texDomainName.c_str(),
                                  texDomainName.c_str());
        sampleCoords = coordVar;
        locations->fTexDomUni = kUseUniform;
    }

    switch (desc.fFetchMode) {
    case StageDesc::k2x2_FetchMode:
        gen2x2FS(stageNum, segments, locations, &sampleCoords,
            samplerName, texelSizeName, smear, fsOutColor,
            texFunc, modulate, complexCoord, coordDims);
        break;
    case ProgramDesc::StageDesc::kConvolution_FetchMode:
        genConvolutionFS(stageNum, desc, segments,
            samplerName, kernelName, smear, imageIncrementName, fsOutColor,
            sampleCoords, texFunc, modulate);
        break;
    default:
        segments->fFSCode.appendf("\t%s = %s(%s, %s)%s%s;\n",
                                  fsOutColor, texFunc.c_str(), 
                                  samplerName, sampleCoords.c_str(),
                                  smear, modulate.c_str());
    }
}


