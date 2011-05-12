/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "GrGLProgram.h"

#include "GrBinHashKey.h"
#include "GrGLConfig.h"
#include "GrGLEffect.h"
#include "GrMemory.h"

#include "SkXfermode.h"

namespace {

const char* GrPrecision() {
    if (GR_GL_SUPPORT_ES2) {
        return "mediump";
    } else {
        return " ";
    }
}

const char* GrShaderPrecision() {
    if (GR_GL_SUPPORT_ES2) {
        return "precision mediump float;\n";
    } else {
        return "";
    }
}

}  // namespace

#define PRINT_SHADERS 0

#if GR_GL_ATTRIBUTE_MATRICES
    #define VIEW_MATRIX_NAME "aViewM"
#else
    #define VIEW_MATRIX_NAME "uViewM"
#endif

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"
#define COL_UNI_NAME "uColor"
#define EDGES_UNI_NAME "uEdges"
#define COL_FILTER_UNI_NAME "uColorFilter"

static inline void tex_attr_name(int coordIdx, GrStringBuilder* s) {
    *s = "aTexCoord";
    s->appendS32(coordIdx);
}

static inline const char* float_vector_type(int count) {
    static const char* FLOAT_VECS[] = {"ERROR", "float", "vec2", "vec3", "vec4"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(FLOAT_VECS));
    return FLOAT_VECS[count];
}

static inline const char* vector_homog_coord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

static inline const char* vector_nonhomog_coords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
}

static inline const char* vector_all_coords(int count) {
    static const char* ALL[] = {"ERROR", "", ".xy", ".xyz", ".xyzw"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(ALL));
    return ALL[count];
}

static void tex_matrix_name(int stage, GrStringBuilder* s) {
#if GR_GL_ATTRIBUTE_MATRICES
    *s = "aTexM";
#else
    *s = "uTexM";
#endif
    s->appendS32(stage);
}

static void normalized_texel_size_name(int stage, GrStringBuilder* s) {
    *s = "uTexelSize";
    s->appendS32(stage);
}

static void sampler_name(int stage, GrStringBuilder* s) {
    *s = "uSampler";
    s->appendS32(stage);
}

static void stage_varying_name(int stage, GrStringBuilder* s) {
    *s = "vStage";
    s->appendS32(stage);
}

static void radial2_param_name(int stage, GrStringBuilder* s) {
    *s = "uRadial2Params";
    s->appendS32(stage);
}

static void radial2_varying_name(int stage, GrStringBuilder* s) {
    *s = "vB";
    s->appendS32(stage);
}

GrGLProgram::GrGLProgram() {
    for(int stage = 0; stage < GrDrawTarget::kNumStages; ++stage) {
        fStageEffects[stage] = NULL;
    }
}

GrGLProgram::~GrGLProgram() {
}

void GrGLProgram::buildKey(GrBinHashKeyBuilder& key) const {
    // Add stage configuration to the key
    key.keyData(reinterpret_cast<const uint8_t*>(&fProgramDesc), sizeof(ProgramDesc));

    for(int stage = 0; stage < GrDrawTarget::kNumStages; ++stage) {
        // First pass: count effects and write the count to the key.
        // This may seem like  we are adding redundant data to the
        // key, but in ensures the one key cannot be a prefix of
        // another key, or identical to the key of a different program.
        GrGLEffect* currentEffect = fStageEffects[stage];
        uint8_t effectCount = 0;
        while (currentEffect) {
            GrAssert(effectCount < 255); // overflow detection
            ++effectCount;
            currentEffect = currentEffect->nextEffect();
        }
        key.keyData(reinterpret_cast<const uint8_t*>(&effectCount), sizeof(uint8_t));

        // Second pass: continue building key using the effects
        currentEffect = fStageEffects[stage];
        while (currentEffect) {
            fStageEffects[stage]->buildKey(key);
        }
    }
}

bool GrGLProgram::doGLSetup(GrPrimitiveType type, 
                            GrGLProgram::CachedData* programData) const {
    for (int stage = 0; stage < GrDrawTarget::kNumStages; ++stage) {
        GrGLEffect* effect = fStageEffects[stage];
        if (effect) {
            if (!effect->doGLSetup(type, programData->fProgramID)) {
                return false;
            }
        }
    }

    return true;
}

void GrGLProgram::doGLPost() const {
    for (int stage = 0; stage < GrDrawTarget::kNumStages; ++stage) {
        GrGLEffect* effect = fStageEffects[stage];
        if (effect) {
            effect->doGLPost(); 
        }    
    }
}

/**
 * Create a text coefficient to be used in fragment shader code.
 */
static void coefficientString(GrStringBuilder* str, SkXfermode::Coeff coeff,
            const char* src, const char* dst) {
    switch (coeff) {
    case SkXfermode::kZero_Coeff:    /** 0 */
        *str = "0.0";
        break;
    case SkXfermode::kOne_Coeff:     /** 1 */
        *str = "1.0";
        break;
    case SkXfermode::kSA_Coeff:      /** src alpha */
        str->appendf("%s.a", src);
        break;
    case SkXfermode::kISA_Coeff:     /** inverse src alpha (i.e. 1 - sa) */
        str->appendf("(1.0 - %s.a)", src);
        break;
    case SkXfermode::kDA_Coeff:      /** dst alpha */
        str->appendf("%s.a", dst);
        break;
    case SkXfermode::kIDA_Coeff:     /** inverse dst alpha (i.e. 1 - da) */
        str->appendf("(1.0 - %s.a)", dst);
        break;
    case SkXfermode::kSC_Coeff:
        str->append(src);
        break;
    default:
        break;
    }
}

/**
 * Adds a line to the fragment shader code which modifies the color by
 * the specified color filter.
 */
static void addColorFilter(GrStringBuilder* FSCode, const char * outputVar,
            SkXfermode::Mode colorFilterXfermode, const char* dstColor) {
    SkXfermode::Coeff srcCoeff, dstCoeff;
    SkDEBUGCODE(bool success =)
    SkXfermode::ModeAsCoeff(colorFilterXfermode, &srcCoeff, &dstCoeff);
    // We currently do not handle modes that cannot be represented as
    // coefficients.
    GrAssert(success);
    GrStringBuilder srcCoeffStr, dstCoeffStr;
    coefficientString(&srcCoeffStr, srcCoeff, COL_FILTER_UNI_NAME, dstColor);
    coefficientString(&dstCoeffStr, dstCoeff, COL_FILTER_UNI_NAME, dstColor);
    FSCode->appendf("\t%s = %s*%s + %s*%s;\n", outputVar, srcCoeffStr.c_str(),
            COL_FILTER_UNI_NAME, dstCoeffStr.c_str(), dstColor);
}

bool GrGLProgram::genProgram(GrGLProgram::CachedData* programData) const {

    ShaderCodeSegments segments;
    const uint32_t& layout = fProgramDesc.fVertexLayout;

    programData->fUniLocations.reset();

#if GR_GL_ATTRIBUTE_MATRICES
    segments.fVSAttrs += "attribute mat3 " VIEW_MATRIX_NAME ";\n";
    programData->fUniLocations.fViewMatrixUni = kSetAsAttribute;
#else
    segments.fVSUnis  += "uniform mat3 " VIEW_MATRIX_NAME ";\n";
    programData->fUniLocations.fViewMatrixUni = kUseUniform;
#endif
    segments.fVSAttrs += "attribute vec2 " POS_ATTR_NAME ";\n";

    segments.fVSCode.append(
        "void main() {\n"
            "\tvec3 pos3 = " VIEW_MATRIX_NAME " * vec3("POS_ATTR_NAME", 1);\n"
            "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n");

    // incoming color to current stage being processed.
    GrStringBuilder inColor;

    switch (fProgramDesc.fColorType) {
    case ProgramDesc::kAttribute_ColorType:
        segments.fVSAttrs.append( "attribute vec4 " COL_ATTR_NAME ";\n");
        segments.fVaryings.append("varying vec4 vColor;\n");
        segments.fVSCode.append(    "\tvColor = " COL_ATTR_NAME ";\n");
        inColor = "vColor";
        break;
    case ProgramDesc::kUniform_ColorType:
        segments.fFSUnis.append(  "uniform vec4 " COL_UNI_NAME ";\n");
        programData->fUniLocations.fColorUni = kUseUniform;
        inColor = COL_UNI_NAME;
        break;
    case ProgramDesc::kNone_ColorType:
        inColor = "";
        break;
    }

    if (fProgramDesc.fUsesEdgeAA) {
        segments.fFSUnis.append("uniform vec3 " EDGES_UNI_NAME "[6];\n");
    }

    if (fProgramDesc.fEmitsPointSize){
        segments.fVSCode.append("\tgl_PointSize = 1.0;\n");
    }

    segments.fFSCode.append("void main() {\n");

    // add texture coordinates that are used to the list of vertex attr decls
    GrStringBuilder texCoordAttrs[GrDrawTarget::kMaxTexCoords];
    for (int t = 0; t < GrDrawTarget::kMaxTexCoords; ++t) {
        if (GrDrawTarget::VertexUsesTexCoordIdx(t, layout)) {
            tex_attr_name(t, texCoordAttrs + t);
            segments.fVSAttrs.appendf("attribute vec2 %s;\n", texCoordAttrs[t].c_str());
        }
    }

    bool useColorFilter = 
            // The rest of transfer mode color filters have not been implemented
            fProgramDesc.fColorFilterXfermode <= SkXfermode::kMultiply_Mode
            // This mode has no effect.
            && fProgramDesc.fColorFilterXfermode != SkXfermode::kDst_Mode;
    bool onlyUseColorFilter = useColorFilter
            && (fProgramDesc.fColorFilterXfermode == SkXfermode::kClear_Mode
            || fProgramDesc.fColorFilterXfermode == SkXfermode::kSrc_Mode);
    if (useColorFilter) {
        // Set up a uniform for the color
        segments.fFSUnis.append(  "uniform vec4 " COL_FILTER_UNI_NAME ";\n");
        programData->fUniLocations.fColorFilterUni = kUseUniform;
    }

    // for each enabled stage figure out what the input coordinates are
    // and count the number of stages in use.
    const char* stageInCoords[GrDrawTarget::kNumStages];
    int numActiveStages = 0;

    if (!onlyUseColorFilter) {
        for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
            if (fProgramDesc.fStages[s].fEnabled) {
                if (GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s) & layout) {
                    stageInCoords[s] = POS_ATTR_NAME;
                } else {
                    int tcIdx = GrDrawTarget::VertexTexCoordsForStage(s, layout);
                     // we better have input tex coordinates if stage is enabled.
                    GrAssert(tcIdx >= 0);
                    GrAssert(texCoordAttrs[tcIdx].size());
                    stageInCoords[s] = texCoordAttrs[tcIdx].c_str();
                }
                ++numActiveStages;
            }
        }
    }

    // if we have active stages string them together, feeding the output color
    // of each to the next and generating code for each stage.
    if (numActiveStages) {
        int currActiveStage = 0;
        GrStringBuilder outColor;
        for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
            if (fProgramDesc.fStages[s].fEnabled) {
                if (currActiveStage < (numActiveStages - 1) || useColorFilter) {
                    outColor = "color";
                    outColor.appendS32(currActiveStage);
                    segments.fFSCode.appendf("\tvec4 %s;\n", outColor.c_str());
                } else {
                    outColor = "gl_FragColor";
                }

                genStageCode(s,
                             fProgramDesc.fStages[s],
                             inColor.size() ? inColor.c_str() : NULL,
                             outColor.c_str(),
                             stageInCoords[s],
                             &segments,
                             &programData->fUniLocations.fStages[s]);
                ++currActiveStage;
                inColor = outColor;
            }
        }
        if (useColorFilter) {
            addColorFilter(&segments.fFSCode, "gl_FragColor",
                    fProgramDesc.fColorFilterXfermode, outColor.c_str());
        }

    } else {
        if (fProgramDesc.fUsesEdgeAA) {
            // FIXME:  put the a's in a loop
            segments.fFSCode.append(
                "\tvec3 pos = vec3(gl_FragCoord.xy, 1);\n"
                "\tfloat a0 = clamp(dot(uEdges[0], pos), 0.0, 1.0);\n"
                "\tfloat a1 = clamp(dot(uEdges[1], pos), 0.0, 1.0);\n"
                "\tfloat a2 = clamp(dot(uEdges[2], pos), 0.0, 1.0);\n"
                "\tfloat a3 = clamp(dot(uEdges[3], pos), 0.0, 1.0);\n"
                "\tfloat a4 = clamp(dot(uEdges[4], pos), 0.0, 1.0);\n"
                "\tfloat a5 = clamp(dot(uEdges[5], pos), 0.0, 1.0);\n"
                "\tfloat edgeAlpha = min(min(a0 * a1, a2 * a3), a4 * a5);\n");
            if (inColor.size()) {
                inColor.append(" * edgeAlpha");
            } else {
                inColor = "vec4(edgeAlpha)";
            }
        }
        // we may not have any incoming color
        const char * incomingColor = (inColor.size() ? inColor.c_str()
                : "vec4(1,1,1,1)");
        if (useColorFilter) {
            addColorFilter(&segments.fFSCode, "gl_FragColor",
                    fProgramDesc.fColorFilterXfermode, incomingColor);
        } else {
            segments.fFSCode.appendf("\tgl_FragColor = %s;\n", incomingColor);
        }
    }
    segments.fVSCode.append("}\n");
    segments.fFSCode.append("}\n");

    if (!CompileFSAndVS(segments, programData)) {
        return false;
    }

    if (!this->bindAttribsAndLinkProgram(texCoordAttrs, programData)) {
        return false;
    }

    this->getUniformLocationsAndInitCache(programData);

    return true;
}

bool GrGLProgram::CompileFSAndVS(const ShaderCodeSegments& segments,
                                 CachedData* programData) {

    const char* strings[4];
    int lengths[4];
    int stringCnt = 0;

    if (segments.fVSUnis.size()) {
        strings[stringCnt] = segments.fVSUnis.c_str();
        lengths[stringCnt] = segments.fVSUnis.size();
        ++stringCnt;
    }
    if (segments.fVSAttrs.size()) {
        strings[stringCnt] = segments.fVSAttrs.c_str();
        lengths[stringCnt] = segments.fVSAttrs.size();
        ++stringCnt;
    }
    if (segments.fVaryings.size()) {
        strings[stringCnt] = segments.fVaryings.c_str();
        lengths[stringCnt] = segments.fVaryings.size();
        ++stringCnt;
    }

    GrAssert(segments.fVSCode.size());
    strings[stringCnt] = segments.fVSCode.c_str();
    lengths[stringCnt] = segments.fVSCode.size();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf(segments.fVSUnis.c_str());
    GrPrintf(segments.fVSAttrs.c_str());
    GrPrintf(segments.fVaryings.c_str());
    GrPrintf(segments.fVSCode.c_str());
    GrPrintf("\n");
#endif
    programData->fVShaderID = CompileShader(GR_GL_VERTEX_SHADER,
                                        stringCnt,
                                        strings,
                                        lengths);

    if (!programData->fVShaderID) {
        return false;
    }

    stringCnt = 0;

    if (strlen(GrShaderPrecision()) > 1) {
        strings[stringCnt] = GrShaderPrecision();
        lengths[stringCnt] = strlen(GrShaderPrecision());
        ++stringCnt;
    }
    if (segments.fFSUnis.size()) {
        strings[stringCnt] = segments.fFSUnis.c_str();
        lengths[stringCnt] = segments.fFSUnis.size();
        ++stringCnt;
    }
    if (segments.fVaryings.size()) {
        strings[stringCnt] = segments.fVaryings.c_str();
        lengths[stringCnt] = segments.fVaryings.size();
        ++stringCnt;
    }

    GrAssert(segments.fFSCode.size());
    strings[stringCnt] = segments.fFSCode.c_str();
    lengths[stringCnt] = segments.fFSCode.size();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf(GrShaderPrecision());
    GrPrintf(segments.fFSUnis.c_str());
    GrPrintf(segments.fVaryings.c_str());
    GrPrintf(segments.fFSCode.c_str());
    GrPrintf("\n");
#endif
    programData->fFShaderID = CompileShader(GR_GL_FRAGMENT_SHADER,
                                            stringCnt,
                                            strings,
                                            lengths);

    if (!programData->fFShaderID) {
        return false;
    }
    return true;
}

GrGLuint GrGLProgram::CompileShader(GrGLenum type,
                                      int stringCnt,
                                      const char** strings,
                                      int* stringLengths) {
    GrGLuint shader = GR_GL(CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    GrGLint compiled = GR_GL_INIT_ZERO;
    GR_GL(ShaderSource(shader, stringCnt, strings, stringLengths));
    GR_GL(CompileShader(shader));
    GR_GL(GetShaderiv(shader, GR_GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GR_GL(GetShaderiv(shader, GR_GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetShaderInfoLog(shader, infoLen+1, NULL, (char*)log.get()));
            for (int i = 0; i < stringCnt; ++i) {
                if (NULL == stringLengths || stringLengths[i] < 0) {
                    GrPrintf(strings[i]);
                } else {
                    GrPrintf("%.*s", stringLengths[i], strings[i]);
                }
            }
            GrPrintf("\n%s", log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GR_GL(DeleteShader(shader));
        return 0;
    }
    return shader;
}

bool GrGLProgram::bindAttribsAndLinkProgram(GrStringBuilder texCoordAttrNames[],
                                            CachedData* programData) const {
    programData->fProgramID = GR_GL(CreateProgram());
    if (!programData->fProgramID) {
        return false;
    }
    const GrGLint& progID = programData->fProgramID;

    GR_GL(AttachShader(progID, programData->fVShaderID));
    GR_GL(AttachShader(progID, programData->fFShaderID));

    // Bind the attrib locations to same values for all shaders
    GR_GL(BindAttribLocation(progID, PositionAttributeIdx(), POS_ATTR_NAME));
    for (int t = 0; t < GrDrawTarget::kMaxTexCoords; ++t) {
        if (texCoordAttrNames[t].size()) {
            GR_GL(BindAttribLocation(progID,
                                     TexCoordAttributeIdx(t),
                                     texCoordAttrNames[t].c_str()));
        }
    }


    if (kSetAsAttribute == programData->fUniLocations.fViewMatrixUni) {
        GR_GL(BindAttribLocation(progID,
                             ViewMatrixAttributeIdx(),
                             VIEW_MATRIX_NAME));
    }

    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        const StageUniLocations& unis = programData->fUniLocations.fStages[s];
        if (kSetAsAttribute == unis.fTextureMatrixUni) {
            GrStringBuilder matName;
            tex_matrix_name(s, &matName);
            GR_GL(BindAttribLocation(progID,
                                     TextureMatrixAttributeIdx(s),
                                     matName.c_str()));
        }
    }


    GR_GL(BindAttribLocation(progID, ColorAttributeIdx(), COL_ATTR_NAME));

    GR_GL(LinkProgram(progID));

    GrGLint linked = GR_GL_INIT_ZERO;
    GR_GL(GetProgramiv(progID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GR_GL(GetProgramiv(progID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetProgramInfoLog(progID,
                                    infoLen+1,
                                    NULL,
                                    (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GR_GL(DeleteProgram(progID));
        programData->fProgramID = 0;
        return false;
    }
    return true;
}

void GrGLProgram::getUniformLocationsAndInitCache(CachedData* programData) const {
    const GrGLint& progID = programData->fProgramID;

    if (kUseUniform == programData->fUniLocations.fViewMatrixUni) {
        programData->fUniLocations.fViewMatrixUni =
                        GR_GL(GetUniformLocation(progID, VIEW_MATRIX_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fViewMatrixUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorUni) {
        programData->fUniLocations.fColorUni = 
                                GR_GL(GetUniformLocation(progID, COL_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorUni);
    }
    if (kUseUniform == programData->fUniLocations.fColorFilterUni) {
        programData->fUniLocations.fColorFilterUni = 
                        GR_GL(GetUniformLocation(progID, COL_FILTER_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fColorFilterUni);
    }

    if (fProgramDesc.fUsesEdgeAA) {
        programData->fUniLocations.fEdgesUni = 
            GR_GL(GetUniformLocation(progID, EDGES_UNI_NAME));
        GrAssert(kUnusedUniform != programData->fUniLocations.fEdgesUni);
    } else {
        programData->fUniLocations.fEdgesUni = kUnusedUniform;
    }

    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        StageUniLocations& locations = programData->fUniLocations.fStages[s];
        if (fProgramDesc.fStages[s].fEnabled) {
            if (kUseUniform == locations.fTextureMatrixUni) {
                GrStringBuilder texMName;
                tex_matrix_name(s, &texMName);
                locations.fTextureMatrixUni = GR_GL(GetUniformLocation(
                                                progID,
                                                texMName.c_str()));
                GrAssert(kUnusedUniform != locations.fTextureMatrixUni);
            }

            if (kUseUniform == locations.fSamplerUni) {
                GrStringBuilder samplerName;
                sampler_name(s, &samplerName);
                locations.fSamplerUni = GR_GL(GetUniformLocation(
                                                     progID,
                                                     samplerName.c_str()));
                GrAssert(kUnusedUniform != locations.fSamplerUni);
            }

            if (kUseUniform == locations.fNormalizedTexelSizeUni) {
                GrStringBuilder texelSizeName;
                normalized_texel_size_name(s, &texelSizeName);
                locations.fNormalizedTexelSizeUni = 
                   GR_GL(GetUniformLocation(progID, texelSizeName.c_str()));
                GrAssert(kUnusedUniform != locations.fNormalizedTexelSizeUni);
            }

            if (kUseUniform == locations.fRadial2Uni) {
                GrStringBuilder radial2ParamName;
                radial2_param_name(s, &radial2ParamName);
                locations.fRadial2Uni = GR_GL(GetUniformLocation(
                                             progID,
                                             radial2ParamName.c_str()));
                GrAssert(kUnusedUniform != locations.fRadial2Uni);
            }
        }
    }
    GR_GL(UseProgram(progID));

    // init sampler unis and set bogus values for state tracking
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if (kUnusedUniform != programData->fUniLocations.fStages[s].fSamplerUni) {
            GR_GL(Uniform1i(programData->fUniLocations.fStages[s].fSamplerUni, s));
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

void GrGLProgram::genStageCode(int stageNum,
                               const GrGLProgram::ProgramDesc::StageDesc& desc,
                               const char* fsInColor, // NULL means no incoming color
                               const char* fsOutColor,
                               const char* vsInCoord,
                               ShaderCodeSegments* segments,
                               StageUniLocations* locations) const {

    GrAssert(stageNum >= 0 && stageNum <= 9);

    GrStringBuilder varyingName;
    stage_varying_name(stageNum, &varyingName);

    // First decide how many coords are needed to access the texture
    // Right now it's always 2 but we could start using 1D textures for
    // gradients.
    static const int coordDims = 2;
    int varyingDims;
    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords
    // and whether the varying needs a perspective coord.
    GrStringBuilder texMName;
    tex_matrix_name(stageNum, &texMName);
    if (desc.fOptFlags & ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit) {
        varyingDims = coordDims;
    } else {
    #if GR_GL_ATTRIBUTE_MATRICES
        segments->fVSAttrs.appendf("attribute mat3 %s;\n", texMName.c_str());
        locations->fTextureMatrixUni = kSetAsAttribute;
    #else
        segments->fVSUnis.appendf("uniform mat3 %s;\n", texMName.c_str());
        locations->fTextureMatrixUni = kUseUniform;
    #endif
        if (desc.fOptFlags & ProgramDesc::StageDesc::kNoPerspective_OptFlagBit) {
            varyingDims = coordDims;
        } else {
            varyingDims = coordDims + 1;
        }
    }

    GrStringBuilder samplerName;
    sampler_name(stageNum, &samplerName);
    segments->fFSUnis.appendf("uniform sampler2D %s;\n", samplerName.c_str());
    locations->fSamplerUni = kUseUniform;

    GrStringBuilder texelSizeName;
    if (ProgramDesc::StageDesc::k2x2_FetchMode == desc.fFetchMode) {
        normalized_texel_size_name(stageNum, &texelSizeName);
        segments->fFSUnis.appendf("uniform vec2 %s;\n", texelSizeName.c_str());
    }

    segments->fVaryings.appendf("varying %s %s;\n", 
                                float_vector_type(varyingDims), varyingName.c_str());

    if (desc.fOptFlags & ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit) {
        GrAssert(varyingDims == coordDims);
        segments->fVSCode.appendf("\t%s = %s;\n", varyingName.c_str(), vsInCoord);
    } else {
        // varying = texMatrix * texCoord
        segments->fVSCode.appendf("\t%s = (%s * vec3(%s, 1))%s;\n",
                                  varyingName.c_str(), texMName.c_str(),
                                  vsInCoord, vector_all_coords(varyingDims));
    }

    GrStringBuilder radial2ParamsName;
    radial2_param_name(stageNum, &radial2ParamsName);
    // for radial grads without perspective we can pass the linear
    // part of the quadratic as a varying.
    GrStringBuilder radial2VaryingName;
    radial2_varying_name(stageNum, &radial2VaryingName);

    if (ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping == desc.fCoordMapping) {

        segments->fVSUnis.appendf("uniform %s float %s[6];\n", 
                                  GrPrecision(), radial2ParamsName.c_str());
        segments->fFSUnis.appendf("uniform float %s[6];\n", 
                                  radial2ParamsName.c_str());
        locations->fRadial2Uni = kUseUniform;

        // if there is perspective we don't interpolate this
        if (varyingDims == coordDims) {
            GrAssert(2 == coordDims);
            segments->fVaryings.appendf("varying float %s;\n", radial2VaryingName.c_str());

            // r2Var = 2 * (r2Parm[2] * varCoord.x - r2Param[3])
            segments->fVSCode.appendf("\t%s = 2.0 *(%s[2] * %s.x - %s[3]);\n",
                                      radial2VaryingName.c_str(), radial2ParamsName.c_str(),
                                      varyingName.c_str(), radial2ParamsName.c_str());
        }
    }

    /// Fragment Shader Stuff
    GrStringBuilder fsCoordName;
    // function used to access the shader, may be made projective
    GrStringBuilder texFunc("texture2D");
    if (desc.fOptFlags & (ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit |
                          ProgramDesc::StageDesc::kNoPerspective_OptFlagBit)) {
        GrAssert(varyingDims == coordDims);
        fsCoordName = varyingName;
    } else {
        // if we have to do some special op on the varyings to get
        // our final tex coords then when in perspective we have to
        // do an explicit divide. Otherwise, we can use a Proj func.
        if  (ProgramDesc::StageDesc::kIdentity_CoordMapping == desc.fCoordMapping &&
             ProgramDesc::StageDesc::kSingle_FetchMode == desc.fFetchMode) {
            texFunc.append("Proj");
            fsCoordName = varyingName;
        } else {
            fsCoordName = "inCoord";
            fsCoordName.appendS32(stageNum);
            segments->fFSCode.appendf("\t%s %s = %s%s / %s%s;\n",
                                       float_vector_type(coordDims),
                                       fsCoordName.c_str(),
                                       varyingName.c_str(),
                                       vector_nonhomog_coords(varyingDims),
                                       varyingName.c_str(),
                                       vector_homog_coord(varyingDims));
        }
    }

    GrStringBuilder sampleCoords;
    bool complexCoord = false;
    switch (desc.fCoordMapping) {
    case ProgramDesc::StageDesc::kIdentity_CoordMapping:
        sampleCoords = fsCoordName;
        break;
    case ProgramDesc::StageDesc::kSweepGradient_CoordMapping:
        sampleCoords.printf("vec2(atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5, 0.5)", fsCoordName.c_str(), fsCoordName.c_str());
        complexCoord = true;
        break;
    case ProgramDesc::StageDesc::kRadialGradient_CoordMapping:
        sampleCoords.printf("vec2(length(%s.xy), 0.5)", fsCoordName.c_str());
        complexCoord = true;
        break;
    case ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping: {
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
            bVar = radial2VaryingName;
            GrAssert(2 == varyingDims);
        } else {
            GrAssert(3 == varyingDims);
            bVar = "b";
            bVar.appendS32(stageNum);
            segments->fFSCode.appendf("\tfloat %s = 2.0 * (%s[2] * %s.x - %s[3]);\n",
                                        bVar.c_str(), radial2ParamsName.c_str(),
                                        fsCoordName.c_str(), radial2ParamsName.c_str());
        }

        // c = (x^2)+(y^2) - params[4]
        segments->fFSCode.appendf("\tfloat %s = dot(%s, %s) - %s[4];\n",
                                  cName.c_str(), fsCoordName.c_str(),
                                  fsCoordName.c_str(),
                                  radial2ParamsName.c_str());
        // ac4 = 4.0 * params[0] * c
        segments->fFSCode.appendf("\tfloat %s = %s[0] * 4.0 * %s;\n",
                                  ac4Name.c_str(), radial2ParamsName.c_str(),
                                  cName.c_str());

        // root = sqrt(b^2-4ac)
        // (abs to avoid exception due to fp precision)
        segments->fFSCode.appendf("\tfloat %s = sqrt(abs(%s*%s - %s));\n",
                                  rootName.c_str(), bVar.c_str(), bVar.c_str(),
                                  ac4Name.c_str());

        // x coord is: (-b + params[5] * sqrt(b^2-4ac)) * params[1]
        // y coord is 0.5 (texture is effectively 1D)
        sampleCoords.printf("vec2((-%s + %s[5] * %s) * %s[1], 0.5)",
                            bVar.c_str(), radial2ParamsName.c_str(),
                            rootName.c_str(), radial2ParamsName.c_str());
        complexCoord = true;
        break;}
    };

    const char* smear;
    if (desc.fModulation == ProgramDesc::StageDesc::kAlpha_Modulation) {
        smear = ".aaaa";
    } else {
        smear = "";
    }
    GrStringBuilder modulate;
    if (NULL != fsInColor) {
        modulate.printf(" * %s", fsInColor);
    }

    if (ProgramDesc::StageDesc::k2x2_FetchMode == desc.fFetchMode) {
        locations->fNormalizedTexelSizeUni = kUseUniform;
        if (complexCoord) {
            // assign the coord to a var rather than compute 4x.
            GrStringBuilder coordVar("tCoord");
            coordVar.appendS32(stageNum);
            segments->fFSCode.appendf("\t%s %s = %s;\n",
                                      float_vector_type(coordDims),
                                      coordVar.c_str(), sampleCoords.c_str());
            sampleCoords = coordVar;
        }
        GrAssert(2 == coordDims);
        GrStringBuilder accumVar("accum");
        accumVar.appendS32(stageNum);
        segments->fFSCode.appendf("\tvec4 %s  = %s(%s, %s + vec2(-%s.x,-%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName.c_str(), sampleCoords.c_str(), texelSizeName.c_str(), texelSizeName.c_str(), smear);
        segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(+%s.x,-%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName.c_str(), sampleCoords.c_str(), texelSizeName.c_str(), texelSizeName.c_str(), smear);
        segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(-%s.x,+%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName.c_str(), sampleCoords.c_str(), texelSizeName.c_str(), texelSizeName.c_str(), smear);
        segments->fFSCode.appendf("\t%s += %s(%s, %s + vec2(+%s.x,+%s.y))%s;\n", accumVar.c_str(), texFunc.c_str(), samplerName.c_str(), sampleCoords.c_str(), texelSizeName.c_str(), texelSizeName.c_str(), smear);
        segments->fFSCode.appendf("\t%s = .25 * %s%s;\n", fsOutColor, accumVar.c_str(), modulate.c_str());
    } else {
        segments->fFSCode.appendf("\t%s = %s(%s, %s)%s %s;\n", fsOutColor, texFunc.c_str(), samplerName.c_str(), sampleCoords.c_str(), smear, modulate.c_str());
    }

    if(fStageEffects[stageNum]) {
        fStageEffects[stageNum]->genShaderCode(segments);
    }
}

