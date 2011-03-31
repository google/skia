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
#include "GrStringBuilder.h"

namespace {

const char* GrPrecision() {
    if (GR_GL_SUPPORT_ES2) {
        return "mediump";
    } else {
        return "";
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

#if ATTRIBUTE_MATRIX
    #define VIEW_MATRIX_NAME "aViewM"
#else
    #define VIEW_MATRIX_NAME "uViewM"
#endif

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"

// for variable names etc
typedef GrSStringBuilder<16> GrTokenString;

static inline void tex_attr_name(int coordIdx, GrStringBuilder* s) {
    *s = "aTexCoord";
    s->appendInt(coordIdx);
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
#if ATTRIBUTE_MATRIX
    *s = "aTexM";
#else
    *s = "uTexM";
#endif
    s->appendInt(stage);
}

static void sampler_name(int stage, GrStringBuilder* s) {
    *s = "uSampler";
    s->appendInt(stage);
}

static void stage_varying_name(int stage, GrStringBuilder* s) {
    *s = "vStage";
    s->appendInt(stage);
}

static void radial2_param_name(int stage, GrStringBuilder* s) {
    *s = "uRadial2Params";
    s->appendInt(stage);
}

static void radial2_varying_name(int stage, GrStringBuilder* s) {
    *s = "vB";
    s->appendInt(stage);
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

void GrGLProgram::genProgram(GrGLProgram::CachedData* programData, 
                             const GrDrawTarget* target) const {

    ShaderCodeSegments segments;
    const uint32_t& layout = fProgramDesc.fVertexLayout;

    memset(&programData->fUniLocations, 0, sizeof(UniLocations));

    bool haveColor = !(ProgramDesc::kVertexColorAllOnes_OptFlagBit &
                       fProgramDesc.fOptFlags);

#if ATTRIBUTE_MATRIX
    segments.fVSAttrs = "attribute mat3 " VIEW_MATRIX_NAME ";\n";
#else
    segments.fVSUnis  = "uniform mat3 " VIEW_MATRIX_NAME ";\n";
    segments.fVSAttrs = "";
#endif
    segments.fVSAttrs += "attribute vec2 " POS_ATTR_NAME ";\n";
    if (haveColor) {
        segments.fVSAttrs += "attribute vec4 " COL_ATTR_NAME ";\n";
        segments.fVaryings = "varying vec4 vColor;\n";
    } else {
        segments.fVaryings = "";
    }

    segments.fVSCode   = "void main() {\n"
                         "\tvec3 pos3 = " VIEW_MATRIX_NAME " * vec3(" POS_ATTR_NAME ", 1);\n"
                         "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n";
    if (haveColor) {
        segments.fVSCode += "\tvColor = " COL_ATTR_NAME ";\n";
    }

    if (!(fProgramDesc.fOptFlags & ProgramDesc::kNotPoints_OptFlagBit)) {
        segments.fVSCode += "\tgl_PointSize = 1.0;\n";
    }
    segments.fFSCode   = "void main() {\n";

    // add texture coordinates that are used to the list of vertex attr decls
    GrTokenString texCoordAttrs[GrDrawTarget::kMaxTexCoords];
    for (int t = 0; t < GrDrawTarget::kMaxTexCoords; ++t) {
        if (target->VertexUsesTexCoordIdx(t, layout)) {
            tex_attr_name(t, texCoordAttrs + t);

            segments.fVSAttrs += "attribute vec2 ";
            segments.fVSAttrs += texCoordAttrs[t];
            segments.fVSAttrs += ";\n";
        }
    }

    // for each enabled stage figure out what the input coordinates are
    // and count the number of stages in use.
    const char* stageInCoords[GrDrawTarget::kNumStages];
    int numActiveStages = 0;

    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if (fProgramDesc.fStages[s].fEnabled) {
            if (GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s) & layout) {
                stageInCoords[s] = POS_ATTR_NAME;
            } else {
                int tcIdx = GrDrawTarget::VertexTexCoordsForStage(s, layout);
                 // we better have input tex coordinates if stage is enabled.
                GrAssert(tcIdx >= 0);
                GrAssert(texCoordAttrs[tcIdx].length());
                stageInCoords[s] = texCoordAttrs[tcIdx].cstr();
            }
            ++numActiveStages;
        }
    }

    GrTokenString inColor = "vColor";

    // if we have active stages string them together, feeding the output color
    // of each to the next and generating code for each stage.
    if (numActiveStages) {
        int currActiveStage = 0;
        for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
            if (fProgramDesc.fStages[s].fEnabled) {
                GrTokenString outColor;
                if (currActiveStage < (numActiveStages - 1)) {
                    outColor = "color";
                    outColor.appendInt(currActiveStage);
                    segments.fFSCode += "\tvec4 ";
                    segments.fFSCode += outColor;
                    segments.fFSCode += ";\n";
                } else {
                    outColor = "gl_FragColor";
                }

                genStageCode(s,
                             fProgramDesc.fStages[s],
                             haveColor ? inColor.cstr() : NULL,
                             outColor.cstr(),
                             stageInCoords[s],
                             &segments,
                             &programData->fUniLocations.fStages[s]);
                ++currActiveStage;
                inColor = outColor;
                haveColor = true;
            }
        }
    } else {
        segments.fFSCode += "\tgl_FragColor = ";
        if (haveColor) {
            segments.fFSCode += inColor;
        } else {
            segments.fFSCode += "vec4(1,1,1,1)";
        }
        segments.fFSCode += ";\n";
    }
    segments.fFSCode += "}\n";
    segments.fVSCode += "}\n";


    const char* strings[4];
    int lengths[4];
    int stringCnt = 0;

    if (segments.fVSUnis.length()) {
        strings[stringCnt] = segments.fVSUnis.cstr();
        lengths[stringCnt] = segments.fVSUnis.length();
        ++stringCnt;
    }
    if (segments.fVSAttrs.length()) {
        strings[stringCnt] = segments.fVSAttrs.cstr();
        lengths[stringCnt] = segments.fVSAttrs.length();
        ++stringCnt;
    }
    if (segments.fVaryings.length()) {
        strings[stringCnt] = segments.fVaryings.cstr();
        lengths[stringCnt] = segments.fVaryings.length();
        ++stringCnt;
    }

    GrAssert(segments.fVSCode.length());
    strings[stringCnt] = segments.fVSCode.cstr();
    lengths[stringCnt] = segments.fVSCode.length();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf("%s%s%s%s\n",
             segments.fVSUnis.cstr(),
             segments.fVSAttrs.cstr(),
             segments.fVaryings.cstr(),
             segments.fVSCode.cstr());
#endif
    programData->fVShaderID = CompileShader(GR_GL_VERTEX_SHADER,
                                        stringCnt,
                                        strings,
                                        lengths);

    stringCnt = 0;

    if (strlen(GrShaderPrecision()) > 1) {
        strings[stringCnt] = GrShaderPrecision();
        lengths[stringCnt] = strlen(GrShaderPrecision());
        ++stringCnt;
    }
    if (segments.fFSUnis.length()) {
        strings[stringCnt] = segments.fFSUnis.cstr();
        lengths[stringCnt] = segments.fFSUnis.length();
        ++stringCnt;
    }
    if (segments.fVaryings.length()) {
        strings[stringCnt] = segments.fVaryings.cstr();
        lengths[stringCnt] = segments.fVaryings.length();
        ++stringCnt;
    }

    GrAssert(segments.fFSCode.length());
    strings[stringCnt] = segments.fFSCode.cstr();
    lengths[stringCnt] = segments.fFSCode.length();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf("%s%s%s%s\n",
             GR_SHADER_PRECISION,
             segments.fFSUnis.cstr(),
             segments.fVaryings.cstr(),
             segments.fFSCode.cstr());
#endif
    programData->fFShaderID = CompileShader(GR_GL_FRAGMENT_SHADER,
                                            stringCnt,
                                            strings,
                                            lengths);

    programData->fProgramID = GR_GL(CreateProgram());
    const GrGLint& progID = programData->fProgramID;

    GR_GL(AttachShader(progID, programData->fVShaderID));
    GR_GL(AttachShader(progID, programData->fFShaderID));

    // Bind the attrib locations to same values for all shaders
    GR_GL(BindAttribLocation(progID, POS_ATTR_LOCATION, POS_ATTR_NAME));
    for (int t = 0; t < GrDrawTarget::kMaxTexCoords; ++t) {
        if (texCoordAttrs[t].length()) {
            GR_GL(BindAttribLocation(progID,
                                     TEX_ATTR_LOCATION(t),
                                     texCoordAttrs[t].cstr()));
        }
    }

#if ATTRIBUTE_MATRIX
    // set unis to a bogus value so that checks against -1 before
    // flushing will pass.
    GR_GL(BindAttribLocation(progID,
                             VIEWMAT_ATTR_LOCATION,
                             VIEW_MATRIX_NAME));

    program->fUniLocations.fViewMatrixUni = BOGUS_MATRIX_UNI_LOCATION;

    for (int s = 0; s < kNumStages; ++s) {
        if (fProgramDesc.fStages[s].fEnabled) {
            GrStringBuilder matName;
            tex_matrix_name(s, &matName);
            GR_GL(BindAttribLocation(progID,
                                     TEXMAT_ATTR_LOCATION(s),
                                     matName.cstr()));
            program->fUniLocations.fStages[s].fTextureMatrixUni =
                                                    BOGUS_MATRIX_UNI_LOCATION;
        }
    }
#endif

    GR_GL(BindAttribLocation(progID, COL_ATTR_LOCATION, COL_ATTR_NAME));

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
        return;
    }

    // Get uniform locations
#if !ATTRIBUTE_MATRIX
    programData->fUniLocations.fViewMatrixUni =
                    GR_GL(GetUniformLocation(progID, VIEW_MATRIX_NAME));
    GrAssert(-1 != programData->fUniLocations.fViewMatrixUni);
#endif
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        StageUniLocations& locations = programData->fUniLocations.fStages[s];
        if (fProgramDesc.fStages[s].fEnabled) {
#if !ATTRIBUTE_MATRIX
            if (locations.fTextureMatrixUni) {
                GrTokenString texMName;
                tex_matrix_name(s, &texMName);
                locations.fTextureMatrixUni = GR_GL(GetUniformLocation(
                                                progID,
                                                texMName.cstr()));
                GrAssert(-1 != locations.fTextureMatrixUni);
            } else {
                locations.fTextureMatrixUni = -1;

            }
#endif

            if (locations.fSamplerUni) {
                GrTokenString samplerName;
                sampler_name(s, &samplerName);
                locations.fSamplerUni = GR_GL(GetUniformLocation(
                                                     progID,
                                                     samplerName.cstr()));
                GrAssert(-1 != locations.fSamplerUni);
            } else {
                locations.fSamplerUni = -1;
            }

            if (locations.fRadial2Uni) {
                GrTokenString radial2ParamName;
                radial2_param_name(s, &radial2ParamName);
                locations.fRadial2Uni = GR_GL(GetUniformLocation(
                                             progID,
                                             radial2ParamName.cstr()));
                GrAssert(-1 != locations.fRadial2Uni);
            } else {
                locations.fRadial2Uni = -1;
            }
        } else {
            locations.fSamplerUni = -1;
            locations.fRadial2Uni = -1;
            locations.fTextureMatrixUni = -1;
        }
    }
    GR_GL(UseProgram(progID));

    // init sampler unis and set bogus values for state tracking
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if (-1 != programData->fUniLocations.fStages[s].fSamplerUni) {
            GR_GL(Uniform1i(programData->fUniLocations.fStages[s].fSamplerUni, s));
        }
        programData->fTextureMatrices[s] = GrMatrix::InvalidMatrix();
        programData->fRadial2CenterX1[s] = GR_ScalarMax;
        programData->fRadial2Radius0[s] = -GR_ScalarMax;
    }
    programData->fViewMatrix = GrMatrix::InvalidMatrix();
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

    GrTokenString varyingName;
    stage_varying_name(stageNum, &varyingName);

    // First decide how many coords are needed to access the texture
    // Right now it's always 2 but we could start using 1D textures for
    // gradients.
    static const int coordDims = 2;
    int varyingDims;
    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords
    // and whether the varying needs a perspective coord.
    GrTokenString texMName;
    tex_matrix_name(stageNum, &texMName);
    if (desc.fOptFlags & ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit) {
        varyingDims = coordDims;
    } else {
    #if ATTRIBUTE_MATRIX
        segments->fVSAttrs += "attribute mat3 ";
        segments->fVSAttrs += texMName;
        segments->fVSAttrs += ";\n";
    #else
        segments->fVSUnis += "uniform mat3 ";
        segments->fVSUnis += texMName;
        segments->fVSUnis += ";\n";
        locations->fTextureMatrixUni = 1;
    #endif
        if (desc.fOptFlags & ProgramDesc::StageDesc::kNoPerspective_OptFlagBit) {
            varyingDims = coordDims;
        } else {
            varyingDims = coordDims + 1;
        }
    }

    GrTokenString samplerName;
    sampler_name(stageNum, &samplerName);
    segments->fFSUnis += "uniform sampler2D ";
    segments->fFSUnis += samplerName;
    segments->fFSUnis += ";\n";
    locations->fSamplerUni = 1;

    segments->fVaryings += "varying ";
    segments->fVaryings += float_vector_type(varyingDims);
    segments->fVaryings += " ";
    segments->fVaryings += varyingName;
    segments->fVaryings += ";\n";

    if (desc.fOptFlags & ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit) {
        GrAssert(varyingDims == coordDims);
        segments->fVSCode += "\t";
        segments->fVSCode += varyingName;
        segments->fVSCode += " = ";
        segments->fVSCode += vsInCoord;
        segments->fVSCode += ";\n";
    } else {
        segments->fVSCode += "\t";
        segments->fVSCode += varyingName;
        segments->fVSCode += " = (";
        segments->fVSCode += texMName;
        segments->fVSCode += " * vec3(";
        segments->fVSCode += vsInCoord;
        segments->fVSCode += ", 1))";
        segments->fVSCode += vector_all_coords(varyingDims);
        segments->fVSCode += ";\n";
    }

    GrTokenString radial2ParamsName;
    radial2_param_name(stageNum, &radial2ParamsName);
    // for radial grads without perspective we can pass the linear
    // part of the quadratic as a varying.
    GrTokenString radial2VaryingName;
    radial2_varying_name(stageNum, &radial2VaryingName);

    if (ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping == desc.fCoordMapping) {

        segments->fVSUnis += "uniform ";
        segments->fVSUnis += GrPrecision();
        segments->fVSUnis += " float ";
        segments->fVSUnis += radial2ParamsName;
        segments->fVSUnis += "[6];\n";

        segments->fFSUnis += "uniform ";
        segments->fFSUnis += GrPrecision();
        segments->fFSUnis += " float ";
        segments->fFSUnis += radial2ParamsName;
        segments->fFSUnis += "[6];\n";
        locations->fRadial2Uni = 1;

        // if there is perspective we don't interpolate this
        if (varyingDims == coordDims) {
            GrAssert(2 == coordDims);
            segments->fVaryings += "varying float ";
            segments->fVaryings += radial2VaryingName;
            segments->fVaryings += ";\n";

            segments->fVSCode += "\t";
            segments->fVSCode += radial2VaryingName;
            segments->fVSCode += " = 2.0 * (";
            segments->fVSCode += radial2ParamsName;
            segments->fVSCode += "[2] * ";
            segments->fVSCode += varyingName;
            segments->fVSCode += ".x ";
            segments->fVSCode += " - ";
            segments->fVSCode += radial2ParamsName;
            segments->fVSCode += "[3]);\n";
        }
    }

    /// Fragment Shader Stuff
    GrTokenString fsCoordName;
    // function used to access the shader, may be made projective
    GrTokenString texFunc("texture2D");
    if (desc.fOptFlags & (ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit |
                          ProgramDesc::StageDesc::kNoPerspective_OptFlagBit)) {
        GrAssert(varyingDims == coordDims);
        fsCoordName = varyingName;
    } else {
        // if we have to do some non-matrix op on the varyings to get
        // our final tex coords then when in perspective we have to
        // do an explicit divide
        if  (ProgramDesc::StageDesc::kIdentity_CoordMapping == desc.fCoordMapping) {
            texFunc += "Proj";
            fsCoordName = varyingName;
        } else {
            fsCoordName = "tCoord";
            fsCoordName.appendInt(stageNum);

            segments->fFSCode += "\t";
            segments->fFSCode += float_vector_type(coordDims);
            segments->fFSCode += " ";
            segments->fFSCode += fsCoordName;
            segments->fFSCode += " = ";
            segments->fFSCode += varyingName;
            segments->fFSCode += vector_nonhomog_coords(varyingDims);
            segments->fFSCode += " / ";
            segments->fFSCode += varyingName;
            segments->fFSCode += vector_homog_coord(varyingDims);
            segments->fFSCode += ";\n";
        }
    }

    GrSStringBuilder<96> sampleCoords;
    switch (desc.fCoordMapping) {
    case ProgramDesc::StageDesc::kIdentity_CoordMapping:
        sampleCoords = fsCoordName;
        break;
    case ProgramDesc::StageDesc::kSweepGradient_CoordMapping:
        sampleCoords = "vec2(atan(-";
        sampleCoords += fsCoordName;
        sampleCoords += ".y, -";
        sampleCoords += fsCoordName;
        sampleCoords += ".x)*0.1591549430918 + 0.5, 0.5)";
        break;
    case ProgramDesc::StageDesc::kRadialGradient_CoordMapping:
        sampleCoords = "vec2(length(";
        sampleCoords += fsCoordName;
        sampleCoords += ".xy), 0.5)";
        break;
    case ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping: {
        GrTokenString cName    = "c";
        GrTokenString ac4Name  = "ac4";
        GrTokenString rootName = "root";

        cName.appendInt(stageNum);
        ac4Name.appendInt(stageNum);
        rootName.appendInt(stageNum);

        GrTokenString bVar;
        if (coordDims == varyingDims) {
            bVar = radial2VaryingName;
            GrAssert(2 == varyingDims);
        } else {
            GrAssert(3 == varyingDims);
            bVar = "b";
            bVar.appendInt(stageNum);
            segments->fFSCode += "\tfloat ";
            segments->fFSCode += bVar;
            segments->fFSCode += " = 2.0 * (";
            segments->fFSCode += radial2ParamsName;
            segments->fFSCode += "[2] * ";
            segments->fFSCode += fsCoordName;
            segments->fFSCode += ".x ";
            segments->fFSCode += " - ";
            segments->fFSCode += radial2ParamsName;
            segments->fFSCode += "[3]);\n";
        }

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += cName;
        segments->fFSCode += " = dot(";
        segments->fFSCode += fsCoordName;
        segments->fFSCode += ", ";
        segments->fFSCode += fsCoordName;
        segments->fFSCode += ") + ";
        segments->fFSCode += " - ";
        segments->fFSCode += radial2ParamsName;
        segments->fFSCode += "[4];\n";

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += ac4Name;
        segments->fFSCode += " = ";
        segments->fFSCode += radial2ParamsName;
        segments->fFSCode += "[0] * 4.0 * ";
        segments->fFSCode += cName;
        segments->fFSCode += ";\n";

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += rootName;
        segments->fFSCode += " = sqrt(abs(";
        segments->fFSCode += bVar;
        segments->fFSCode += " * ";
        segments->fFSCode += bVar;
        segments->fFSCode += " - ";
        segments->fFSCode += ac4Name;
        segments->fFSCode += "));\n";

        sampleCoords = "vec2((-";
        sampleCoords += bVar;
        sampleCoords += " + ";
        sampleCoords += radial2ParamsName;
        sampleCoords += "[5] * ";
        sampleCoords += rootName;
        sampleCoords += ") * ";
        sampleCoords += radial2ParamsName;
        sampleCoords += "[1], 0.5)\n";
        break;}
    };

    segments->fFSCode += "\t";
    segments->fFSCode += fsOutColor;
    segments->fFSCode += " = ";
    if (NULL != fsInColor) {
        segments->fFSCode += fsInColor;
        segments->fFSCode += " * ";
    }
    segments->fFSCode += texFunc;
    segments->fFSCode += "(";
    segments->fFSCode += samplerName;
    segments->fFSCode += ", ";
    segments->fFSCode += sampleCoords;
    segments->fFSCode += ")";
    if (desc.fModulation == ProgramDesc::StageDesc::kAlpha_Modulation) {
        segments->fFSCode += ".aaaa";
    }
    segments->fFSCode += ";\n";

    if(fStageEffects[stageNum]) {
        fStageEffects[stageNum]->genShaderCode(segments);
    }
}

