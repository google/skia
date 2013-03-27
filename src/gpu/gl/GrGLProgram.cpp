/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrEffect.h"
#include "GrDrawEffect.h"
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

SK_CONF_DECLARE(bool, c_PrintShaders, "gpu.printShaders", false,
                "Print the source code for all shaders generated.");

#define COL_ATTR_NAME "aColor"
#define COV_ATTR_NAME "aCoverage"
#define EDGE_ATTR_NAME "aEdge"

namespace {
inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }
}

void GrGLProgramDesc::Build(const GrDrawState& drawState,
                            bool isPoints,
                            GrDrawState::BlendOptFlags blendOpts,
                            GrBlendCoeff srcCoeff,
                            GrBlendCoeff dstCoeff,
                            const GrGpuGL* gpu,
                            GrGLProgramDesc* desc) {

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

    bool requiresAttributeColors =
        !skipColor && SkToBool(desc->fAttribBindings & GrDrawState::kColor_AttribBindingsBit);
    bool requiresAttributeCoverage =
        !skipCoverage && SkToBool(desc->fAttribBindings & GrDrawState::kCoverage_AttribBindingsBit);

    // fColorInput/fCoverageInput records how colors are specified for the program So we strip the
    // bits from the bindings to avoid false negatives when searching for an existing program in the
    // cache.
    desc->fAttribBindings &=
        ~(GrDrawState::kColor_AttribBindingsBit | GrDrawState::kCoverage_AttribBindingsBit);

    desc->fColorFilterXfermode = skipColor ?
                                SkXfermode::kDst_Mode :
                                drawState.getColorFilterMode();

    // no reason to do edge aa or look at per-vertex coverage if coverage is ignored
    if (skipCoverage) {
        desc->fAttribBindings &= ~(GrDrawState::kCoverage_AttribBindingsBit);
    }

    bool colorIsTransBlack = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);
    bool colorIsSolidWhite = (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) ||
                             (!requiresAttributeColors && 0xffffffff == drawState.getColor());
    if (colorIsTransBlack) {
        desc->fColorInput = kTransBlack_ColorInput;
    } else if (colorIsSolidWhite) {
        desc->fColorInput = kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeColors) {
        desc->fColorInput = kUniform_ColorInput;
    } else {
        desc->fColorInput = kAttribute_ColorInput;
    }

    bool covIsSolidWhite = !requiresAttributeCoverage && 0xffffffff == drawState.getCoverage();

    if (skipCoverage) {
        desc->fCoverageInput = kTransBlack_ColorInput;
    } else if (covIsSolidWhite) {
        desc->fCoverageInput = kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeCoverage) {
        desc->fCoverageInput = kUniform_ColorInput;
    } else {
        desc->fCoverageInput = kAttribute_ColorInput;
    }

    int lastEnabledStage = -1;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {

        bool skip = s < drawState.getFirstCoverageStage() ? skipColor : skipCoverage;
        if (!skip && drawState.isStageEnabled(s)) {
            lastEnabledStage = s;
            const GrEffectRef& effect = *drawState.getStage(s).getEffect();
            const GrBackendEffectFactory& factory = effect->getFactory();
            bool explicitLocalCoords = (drawState.getAttribBindings() &
                                        GrDrawState::kLocalCoords_AttribBindingsBit);
            GrDrawEffect drawEffect(drawState.getStage(s), explicitLocalCoords);
            desc->fEffectKeys[s] = factory.glEffectKey(drawEffect, gpu->glCaps());
        } else {
            desc->fEffectKeys[s] = 0;
        }
    }

    desc->fDualSrcOutput = kNone_DualSrcOutput;

    // Currently the experimental GS will only work with triangle prims (and it doesn't do anything
    // other than pass through values from the VS to the FS anyway).
#if GR_GL_EXPERIMENTAL_GS
#if 0
    desc->fExperimentalGS = gpu->caps().geometryShaderSupport();
#else
    desc->fExperimentalGS = false;
#endif
#endif

    // We leave this set to kNumStages until we discover that the coverage/color distinction is
    // material to the generated program. We do this to avoid distinct keys that generate equivalent
    // programs.
    desc->fFirstCoverageStage = GrDrawState::kNumStages;
    // This tracks the actual first coverage stage.
    int firstCoverageStage = GrDrawState::kNumStages;
    desc->fDiscardIfZeroCoverage = false; // Enabled below if stenciling and there is coverage.
    bool hasCoverage = false;
    // If we're rendering coverage-as-color then its as though there are no coverage stages.
    if (!drawState.isCoverageDrawing()) {
        // We can have coverage either through a stage or coverage vertex attributes.
        if (drawState.getFirstCoverageStage() <= lastEnabledStage) {
            firstCoverageStage = drawState.getFirstCoverageStage();
            hasCoverage = true;
        } else {
            hasCoverage = requiresAttributeCoverage;
        }
    }

    if (hasCoverage) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != desc->fColorFilterXfermode) {
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        // If we're stenciling then we want to discard samples that have zero coverage
        if (drawState.getStencil().doesWrite()) {
            desc->fDiscardIfZeroCoverage = true;
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        if (gpu->caps()->dualSourceBlendingSupport() &&
            !(blendOpts & (GrDrawState::kEmitCoverage_BlendOptFlag |
                           GrDrawState::kCoverageAsAlpha_BlendOptFlag))) {
            if (kZero_GrBlendCoeff == dstCoeff) {
                // write the coverage value to second color
                desc->fDualSrcOutput =  kCoverage_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSA_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = kCoverageISA_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSC_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = kCoverageISC_DualSrcOutput;
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
    if (desc->fAttribBindings & GrDrawState::kLocalCoords_AttribBindingsBit) {
        desc->fLocalCoordsAttributeIndex = drawState.getAttribIndex(GrDrawState::kLocalCoords_AttribIndex);
    }

#if GR_DEBUG
    // Verify valid vertex attribute state. These assertions should probably be done somewhere
    // higher up the callstack
    const GrVertexAttrib* vertexAttribs = drawState.getVertexAttribs();
    GrAssert(desc->fPositionAttributeIndex < GrDrawState::kVertexAttribCnt);
    GrAssert(GrGLAttribTypeToLayout(vertexAttribs[desc->fPositionAttributeIndex].fType).fCount == 2);
    if (requiresAttributeColors) {
        GrAssert(desc->fColorAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(GrGLAttribTypeToLayout(vertexAttribs[desc->fColorAttributeIndex].fType).fCount == 4);
    }
    if (requiresAttributeCoverage) {
        GrAssert(desc->fCoverageAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(GrGLAttribTypeToLayout(vertexAttribs[desc->fCoverageAttributeIndex].fType).fCount == 4);
    }
    if (desc->fAttribBindings & GrDrawState::kLocalCoords_AttribBindingsBit) {
        GrAssert(desc->fLocalCoordsAttributeIndex < GrDrawState::kVertexAttribCnt);
        GrAssert(GrGLAttribTypeToLayout(vertexAttribs[desc->fLocalCoordsAttributeIndex].fType).fCount == 2);
    }
#endif
}

GrGLProgram* GrGLProgram::Create(const GrGLContext& gl,
                                 const GrGLProgramDesc& desc,
                                 const GrEffectStage* stages[]) {
    GrGLProgram* program = SkNEW_ARGS(GrGLProgram, (gl, desc, stages));
    if (!program->succeeded()) {
        delete program;
        program = NULL;
    }
    return program;
}

GrGLProgram::GrGLProgram(const GrGLContext& gl,
                         const GrGLProgramDesc& desc,
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
        case GrGLProgramDesc::kNone_DualSrcOutput:
            break;
        // the prog will write a coverage value to the secondary
        // output and the dst is blended by one minus that value.
        case GrGLProgramDesc::kCoverage_DualSrcOutput:
        case GrGLProgramDesc::kCoverageISA_DualSrcOutput:
        case GrGLProgramDesc::kCoverageISC_DualSrcOutput:
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
void add_color_filter(GrGLShaderBuilder* builder,
                      const char * outputVar,
                      SkXfermode::Coeff uniformCoeff,
                      SkXfermode::Coeff colorCoeff,
                      const char* filterColor,
                      const char* inColor) {
    SkString colorStr, constStr;
    blend_term_string(&colorStr, colorCoeff, filterColor, inColor, inColor);
    blend_term_string(&constStr, uniformCoeff, filterColor, inColor, filterColor);

    SkString sum;
    GrGLSLAdd4f(&sum, colorStr.c_str(), constStr.c_str());
    builder->fsCodeAppendf("\t%s = %s;\n", outputVar, sum.c_str());
}
}

void GrGLProgram::genInputColor(GrGLShaderBuilder* builder, SkString* inColor) {
    switch (fDesc.fColorInput) {
        case GrGLProgramDesc::kAttribute_ColorInput: {
            builder->addAttribute(kVec4f_GrSLType, COL_ATTR_NAME);
            const char *vsName, *fsName;
            builder->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
            builder->vsCodeAppendf("\t%s = " COL_ATTR_NAME ";\n", vsName);
            *inColor = fsName;
            } break;
        case GrGLProgramDesc::kUniform_ColorInput: {
            const char* name;
            fUniformHandles.fColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                            kVec4f_GrSLType, "Color", &name);
            *inColor = name;
            break;
        }
        case GrGLProgramDesc::kTransBlack_ColorInput:
            GrAssert(!"needComputedColor should be false.");
            break;
        case GrGLProgramDesc::kSolidWhite_ColorInput:
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
        builder->fsCodeAppendf("\tvec4 uniCoverage = %s * %s;\n",
                               covUniName, inOutCoverage->c_str());
        *inOutCoverage = "uniCoverage";
    } else {
        *inOutCoverage = covUniName;
    }
}

namespace {
void gen_attribute_coverage(GrGLShaderBuilder* builder,
                            SkString* inOutCoverage) {
    builder->addAttribute(kVec4f_GrSLType, COV_ATTR_NAME);
    const char *vsName, *fsName;
    builder->addVarying(kVec4f_GrSLType, "Coverage", &vsName, &fsName);
    builder->vsCodeAppendf("\t%s = " COV_ATTR_NAME ";\n", vsName);
    if (inOutCoverage->size()) {
        builder->fsCodeAppendf("\tvec4 attrCoverage = %s * %s;\n", fsName, inOutCoverage->c_str());
        *inOutCoverage = "attrCoverage";
    } else {
        *inOutCoverage = fsName;
    }
}
}

void GrGLProgram::genGeometryShader(GrGLShaderBuilder* builder) const {
#if GR_GL_EXPERIMENTAL_GS
    // TODO: The builder should add all this glue code.
    if (fDesc.fExperimentalGS) {
        GrAssert(fContext.info().glslGeneration() >= k150_GrGLSLGeneration);
        builder->fGSHeader.append("layout(triangles) in;\n"
                                   "layout(triangle_strip, max_vertices = 6) out;\n");
        builder->gsCodeAppend("\tfor (int i = 0; i < 3; ++i) {\n"
                              "\t\tgl_Position = gl_in[i].gl_Position;\n");
        if (fDesc.fEmitsPointSize) {
            builder->gsCodeAppend("\t\tgl_PointSize = 1.0;\n");
        }
        GrAssert(builder->fGSInputs.count() == builder->fGSOutputs.count());
        int count = builder->fGSInputs.count();
        for (int i = 0; i < count; ++i) {
            builder->gsCodeAppendf("\t\t%s = %s[i];\n",
                                   builder->fGSOutputs[i].getName().c_str(),
                                   builder->fGSInputs[i].getName().c_str());
        }
        builder->gsCodeAppend("\t\tEmitVertex();\n"
                              "\t}\n"
                              "\tEndPrimitive();\n");
    }
#endif
}

const char* GrGLProgram::adjustInColor(const SkString& inColor) const {
    if (inColor.size()) {
          return inColor.c_str();
    } else {
        if (GrGLProgramDesc::kSolidWhite_ColorInput == fDesc.fColorInput) {
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

    const GrAttribBindings& attribBindings = fDesc.fAttribBindings;
    bool hasExplicitLocalCoords =
        SkToBool(attribBindings & GrDrawState::kLocalCoords_AttribBindingsBit);
    GrGLShaderBuilder builder(fContext.info(), fUniformManager, hasExplicitLocalCoords);

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
    if (GrGLProgramDesc::kTransBlack_ColorInput == fDesc.fCoverageInput) {
        colorCoeff = SkXfermode::kZero_Coeff;
        uniformCoeff = SkXfermode::kZero_Coeff;
    }

    // If we know the final color is going to be all zeros then we can
    // simplify the color filter coefficients. needComputedColor will then
    // come out false below.
    if (GrGLProgramDesc::kTransBlack_ColorInput == fDesc.fColorInput) {
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


    builder.vsCodeAppendf("\tvec3 pos3 = %s * vec3(%s, 1);\n"
                          "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n",
                          viewMName, builder.positionAttribute().getName().c_str());

    // incoming color to current stage being processed.
    SkString inColor;

    if (needComputedColor) {
        this->genInputColor(&builder, &inColor);
    }

    // we output point size in the GS if present
    if (fDesc.fEmitsPointSize && !builder.fUsesGS){
        builder.vsCodeAppend("\tgl_PointSize = 1.0;\n");
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
                builder.fsCodeAppendf("\tvec4 %s;\n", outColor.c_str());

                builder.setCurrentStage(s);
                fEffects[s] = builder.createAndEmitGLEffect(*stages[s],
                                                            fDesc.fEffectKeys[s],
                                                            inColor.size() ? inColor.c_str() : NULL,
                                                            outColor.c_str(),
                                                            &fUniformHandles.fSamplerUnis[s]);
                builder.setNonStage();
                inColor = outColor;
            }
        }
    }

    // if have all ones or zeros for the "dst" input to the color filter then we
    // may be able to make additional optimizations.
    if (needColorFilterUniform && needComputedColor && !inColor.size()) {
        GrAssert(GrGLProgramDesc::kSolidWhite_ColorInput == fDesc.fColorInput);
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
        builder.fsCodeAppendf("\t%s = %s;\n", colorOutput.getName().c_str(), GrGLSLZerosVecf(4));
        wroteFragColorZero = true;
    } else if (SkXfermode::kDst_Mode != fDesc.fColorFilterXfermode) {
        builder.fsCodeAppend("\tvec4 filteredColor;\n");
        const char* color = adjustInColor(inColor);
        add_color_filter(&builder, "filteredColor", uniformCoeff,
                         colorCoeff, colorFilterColorUniName, color);
        inColor = "filteredColor";
    }

    ///////////////////////////////////////////////////////////////////////////
    // compute the partial coverage (coverage stages and edge aa)

    SkString inCoverage;
    bool coverageIsZero = GrGLProgramDesc::kTransBlack_ColorInput == fDesc.fCoverageInput;
    // we don't need to compute coverage at all if we know the final shader
    // output will be zero and we don't have a dual src blend output.
    if (!wroteFragColorZero || GrGLProgramDesc::kNone_DualSrcOutput != fDesc.fDualSrcOutput) {

        if (!coverageIsZero) {
            switch (fDesc.fCoverageInput) {
                case GrGLProgramDesc::kSolidWhite_ColorInput:
                    // empty string implies solid white
                    break;
                case GrGLProgramDesc::kAttribute_ColorInput:
                    gen_attribute_coverage(&builder, &inCoverage);
                    break;
                case GrGLProgramDesc::kUniform_ColorInput:
                    this->genUniformCoverage(&builder, &inCoverage);
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
                    builder.fsCodeAppendf("\tvec4 %s;\n", outCoverage.c_str());

                    builder.setCurrentStage(s);
                    fEffects[s] = builder.createAndEmitGLEffect(
                                                    *stages[s],
                                                    fDesc.fEffectKeys[s],
                                                    inCoverage.size() ? inCoverage.c_str() : NULL,
                                                    outCoverage.c_str(),
                                                    &fUniformHandles.fSamplerUnis[s]);
                    builder.setNonStage();
                    inCoverage = outCoverage;
                }
            }

            // discard if coverage is zero
            if (fDesc.fDiscardIfZeroCoverage && !outCoverage.isEmpty()) {
                builder.fsCodeAppendf(
                    "\tif (all(lessThanEqual(%s, vec4(0.0)))) {\n\t\tdiscard;\n\t}\n",
                    outCoverage.c_str());
            }
        }

        if (GrGLProgramDesc::kNone_DualSrcOutput != fDesc.fDualSrcOutput) {
            builder.fFSOutputs.push_back().set(kVec4f_GrSLType,
                                               GrGLShaderVar::kOut_TypeModifier,
                                               dual_source_output_name());
            bool outputIsZero = coverageIsZero;
            SkString coeff;
            if (!outputIsZero &&
                GrGLProgramDesc::kCoverage_DualSrcOutput != fDesc.fDualSrcOutput && !wroteFragColorZero) {
                if (!inColor.size()) {
                    outputIsZero = true;
                } else {
                    if (GrGLProgramDesc::kCoverageISA_DualSrcOutput == fDesc.fDualSrcOutput) {
                        coeff.printf("(1 - %s.a)", inColor.c_str());
                    } else {
                        coeff.printf("(vec4(1,1,1,1) - %s)", inColor.c_str());
                    }
                }
            }
            if (outputIsZero) {
                builder.fsCodeAppendf("\t%s = %s;\n", dual_source_output_name(), GrGLSLZerosVecf(4));
            } else {
                SkString modulate;
                GrGLSLModulate4f(&modulate, coeff.c_str(), inCoverage.c_str());
                builder.fsCodeAppendf("\t%s = %s;\n", dual_source_output_name(), modulate.c_str());
            }
            dualSourceOutputWritten = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    if (!wroteFragColorZero) {
        if (coverageIsZero) {
            builder.fsCodeAppendf("\t%s = %s;\n", colorOutput.getName().c_str(), GrGLSLZerosVecf(4));
        } else {
            SkString modulate;
            GrGLSLModulate4f(&modulate, inColor.c_str(), inCoverage.c_str());
            builder.fsCodeAppendf("\t%s = %s;\n", colorOutput.getName().c_str(), modulate.c_str());
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

    if (fDesc.fAttribBindings & GrDrawState::kLocalCoords_AttribBindingsBit) {
        GL_CALL(BindAttribLocation(fProgramID,
                                   fDesc.fLocalCoordsAttributeIndex,
                                   builder.localCoordsAttribute().c_str()));
    }

    const GrGLShaderBuilder::AttributePair* attribEnd = builder.getEffectAttributes().end();
    for (const GrGLShaderBuilder::AttributePair* attrib = builder.getEffectAttributes().begin();
         attrib != attribEnd;
         ++attrib) {
         GL_CALL(BindAttribLocation(fProgramID, attrib->fIndex, attrib->fName.c_str()));
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

            bool explicitLocalCoords =
                (fDesc.fAttribBindings & GrDrawState::kLocalCoords_AttribBindingsBit);
            GrDrawEffect drawEffect(stage, explicitLocalCoords);
            fEffects[s]->setData(fUniformManager, drawEffect);
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
            case GrGLProgramDesc::kAttribute_ColorInput:
                if (sharedState->fConstAttribColor != color) {
                    // OpenGL ES only supports the float varieties of glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    GL_CALL(VertexAttrib4fv(fDesc.fColorAttributeIndex, c));
                    sharedState->fConstAttribColor = color;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
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
            case GrGLProgramDesc::kSolidWhite_ColorInput:
            case GrGLProgramDesc::kTransBlack_ColorInput:
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
            case GrGLProgramDesc::kAttribute_ColorInput:
                if (sharedState->fConstAttribCoverage != coverage) {
                    // OpenGL ES only supports the float varieties of  glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    GL_CALL(VertexAttrib4fv(fDesc.fCoverageAttributeIndex, c));
                    sharedState->fConstAttribCoverage = coverage;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
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
            case GrGLProgramDesc::kSolidWhite_ColorInput:
            case GrGLProgramDesc::kTransBlack_ColorInput:
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
