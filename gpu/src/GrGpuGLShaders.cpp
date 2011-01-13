/*
    Copyright 2010 Google Inc.

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


#include "GrGLConfig.h"

#if GR_SUPPORT_GLES2 || GR_SUPPORT_GLDESKTOP

#include "GrGpuGLShaders.h"
#include "GrGpuVertex.h"
#include "GrMemory.h"

#define ATTRIBUTE_MATRIX        0

#define ATTRIBUTE_TEXT_COLOR    1

#if ATTRIBUTE_MATRIX
    #define DECL_MATRIX(name) "attribute mat3 " #name ";\n"
#else
    #define DECL_MATRIX(name) "uniform mat3 " #name ";\n"
#endif

#define SKIP_CACHE_CHECK    true

#if GR_SUPPORT_GLES2
    #define GR_PRECISION            "mediump"
    #define GR_SHADER_PRECISION     "precision mediump float;\n"
#else
    #define GR_PRECISION            ""
    #define GR_SHADER_PRECISION     ""
#endif

static const char* gvshad[] = {
    // 0: kTextureVertCoords_Program, kTextureVertCoordsProj_Program,
    //    kRadialTextureVertCoords_Program, kSweepTextureVertCoords_Program
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   gl_PointSize = 1.0;\n"
    "   vTexture = texM * vec3(aPosition,1);\n"
    "   vColor = aColor;\n"
    "}\n",

    // 1: kTextureTexCoords_Program, kTextureTexCoordsProj_Program,
    //    kRadialTextureTexCoords_Program, kSweepTextureTexCoords_Program
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexture;\n"
    "attribute vec4 aColor;\n"
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   gl_PointSize = 1.0;\n"
    "   vTexture = texM * vec3(aTexture,1);\n"
    "   vColor = aColor;\n"
    "}\n",

     // 2: kText_Program
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexture;\n"
    "varying vec2 vTexture;\n"
    DECL_MATRIX(viewM)
#if ATTRIBUTE_TEXT_COLOR
    "varying vec4 vColor;\n"
    "attribute vec4 aColor;\n"
#endif
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   vTexture = aTexture;\n"
#if ATTRIBUTE_TEXT_COLOR
    "   vColor = aColor;\n"
#endif
    "}\n",

    // 3: kNoTexture_Program
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    DECL_MATRIX(viewM)
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   gl_PointSize = 1.0;\n"
    "   vColor = aColor;\n"
    "}\n",

    // 4: kTextureVertCoordsNoColor_Program
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec3 vTexture;\n"
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   vTexture = texM * vec3(aPosition,1);\n"
    "}\n",

    // 5: kTextureTexCoordsNoColor_Program
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexture;\n"
    "varying vec3 vTexture;\n"
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "void main() {\n"
    "   vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "   gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "   gl_PointSize = 1.0;\n"
    "   vTexture = texM * vec3(aTexture,1);\n"
    "}\n",

    // 6: kTwoPointRadialTextureVertCoords_Program
    "uniform " GR_PRECISION " float uParams[6];\n"
        // 0 is t^2 term of quadratic
        // 1 is one-half the inverse of above
        // 2 is x offset of the second circle (post tex-matrix)
        // 3 is the radius of the first circle (post tex-matrix)
        // 4 is the first circle radius squared
        // 5 is 1 to use + in the quadratic eq or -1 to use -
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "varying float vB;\n"           // t coeffecient of quadratic.
    "varying vec2 t;\n"             // coordinates in canonical space
    "void main() {\n"
    "    vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "    gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "    t = vec2(texM * vec3(aPosition,1));\n"
    "    vColor = aColor;\n"
    "    vB = 2.0 * (uParams[2] * t.x - uParams[3]);\n"
    "}\n",

    // 6: kTwoPointRadialTextureVertCoords_Program
    "uniform " GR_PRECISION " float uParams[6];\n"
    DECL_MATRIX(viewM)
    DECL_MATRIX(texM)
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexture;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "varying float vB;\n"           // t coeffecient of quadratic.
    "varying vec2 t;\n"             // coordinates in canonical space
    "void main() {\n"
    "    vec3 pos3 = viewM*vec3(aPosition,1);\n"
    "    gl_Position = vec4(pos3.xy,0,pos3.z);\n"
    "    t = vec2(texM * vec3(aTexture,1));\n"
    "    vColor = aColor;\n"
    "    vB = 2.0 * (uParams[2] * t.x - uParams[3]);\n"
    "}\n",
};

static const char* gfshad[] = {
    // 0: kTextureVertCoords_Program, kTextureTexCoords_Program
    GR_SHADER_PRECISION
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
    "   gl_FragColor = vColor * texture2D(sTexture, vTexture.xy);\n"
    "}\n",

    // 1: kTextureVertCoordsProj_Program, kTextureTexCoordsProj_Program
    GR_SHADER_PRECISION
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
    // On Brian's PC laptop with Intel Gfx texture2DProj seems to be broken
    // but it works everywhere else tested.
#if GR_GLSL_2DPROJ_BROKEN
    "   gl_FragColor = vColor * texture2D(sTexture, vTexture.xy / vTexture.z);\n"
#else
    "   gl_FragColor = vColor * texture2DProj(sTexture, vTexture);\n"
#endif

    "}\n",

    // 2: kText_Program
    GR_SHADER_PRECISION
    "varying vec2 vTexture;\n"
#if ATTRIBUTE_TEXT_COLOR
    "varying vec4 vColor;\n"
#else
    "uniform vec4 uColor;\n"
#endif
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
#if ATTRIBUTE_TEXT_COLOR
    "   gl_FragColor = vColor * texture2D(sTexture, vTexture).a;\n"
#else
    "   gl_FragColor = uColor * texture2D(sTexture, vTexture).a;\n"
#endif
    "}\n",

    // 3: kNoTexture_Program
    GR_SHADER_PRECISION
    "varying vec4 vColor;\n"
    "void main() {\n"
    "   gl_FragColor = vColor;\n"
    "}\n",

    // 4: kTextureVertCoordsNoColor_Program
    GR_SHADER_PRECISION
    "varying vec3 vTexture;\n"
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(sTexture, vTexture.xy);\n"
    "}\n",

    // 5: kRadialTextureVertCoords_Program, kRadialTextureTexCoords_Program
    GR_SHADER_PRECISION
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
    "   gl_FragColor = vColor * texture2D(sTexture, vec2(length(vTexture.xy), 0.5));\n"
    "}\n",

    // 6: kSweepTextureVertCoords_Program, kSweepTextureTexCoords_Program
    GR_SHADER_PRECISION
    "varying vec3 vTexture;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D sTexture;\n"
    "void main() {\n"
    "   vec2 t = vec2(atan(-vTexture.y, -vTexture.x)*0.1591549430918 + 0.5,\n"
    "                 0.5);\n"
    "   gl_FragColor = vColor * texture2D(sTexture, t);\n"
    "}\n",

    // 7: kTwoPointRadialTextureVertCoords_Program, kTwoPointRadialTextureTexCoords_Program
    GR_SHADER_PRECISION
    "varying vec4 vColor;\n"
    "varying float vB;\n"             // t coeffecient of quadratic.
    "varying vec2 t;\n"               // coordinates in canonical radial gradient space
    "uniform sampler2D sTexture;\n"
    "uniform float uParams[6];\n"
    "void main() {\n"
        "float c = t.x*t.x + t.y*t.y - uParams[4];\n"
        "float ac4 = uParams[0] * c * 4.0;\n"
        "float root = sqrt(abs(vB * vB - ac4));\n"
        "float t = (-vB + uParams[5] * root) * uParams[1];\n"
        "gl_FragColor = vColor * texture2D(sTexture, vec2(t,0.5))\n;"
    "}\n",
};

// determines which frag/vert shaders are used for each program in Programs enum

static const struct {
    int fVShaderIdx;
    int fFShaderIdx;
    bool fHasTexMatrix;
    bool fHasTexCoords;
    bool fTwoPointRadial;
    GrGpuGLShaders::ColorType fColorType;
} gProgramLoadData[] = {
    // kTextureVertCoords_Program
    {0, 0, true,  false, false, GrGpuGLShaders::kAttrib_ColorType },
    // kTextureVertCoordsProj_Program
    {0, 1, true,  false, false, GrGpuGLShaders::kAttrib_ColorType },
    // kTextureTexCoords_Program
    {1, 0, true,  true,  false, GrGpuGLShaders::kAttrib_ColorType },
    // kTextureTexCoordsProj_Program
    {1, 1, true,  true,  false, GrGpuGLShaders::kAttrib_ColorType },
    // kTextureVertCoordsNoColor_Program
    {4, 4, true,  false, false, GrGpuGLShaders::kNone_ColorType },
    // kTextureTexCoordsNoColor_Program
    {5, 4, true,  false, false, GrGpuGLShaders::kNone_ColorType },
    // kText_Program
#if ATTRIBUTE_TEXT_COLOR
    {2, 2, false, true,  false, GrGpuGLShaders::kAttrib_ColorType },
#else
    {2, 2, false, true,  false, GrGpuGLShaders::kUniform_ColorType },
#endif
    // kRadialTextureVertCoords_Program
    {0, 5, true,  false, false, GrGpuGLShaders::kAttrib_ColorType },
    // kRadialTextureTexCoords_Program
    {1, 5, true,  true,  false, GrGpuGLShaders::kAttrib_ColorType },
    // kSweepTextureVertCoords_Program
    {0, 6, true,  false, false, GrGpuGLShaders::kAttrib_ColorType },
    // kSweepTextureTexCoords_Program
    {1, 6, true,  true,  false, GrGpuGLShaders::kAttrib_ColorType },
    // kTwoPointRadialTextureVertCoords_Program
    {6, 7, true,  false, true,  GrGpuGLShaders::kAttrib_ColorType },
    // kTwoPointRadialTextureTexCoords_Program
    {7, 7, true,  true,  true,  GrGpuGLShaders::kAttrib_ColorType },
    // kNoTexture_Program
    {3, 3, false, false, false, GrGpuGLShaders::kAttrib_ColorType },
};

#define GR_GL_POS_ATTR_LOCATION 0
#define GR_GL_TEX_ATTR_LOCATION 1
#define GR_GL_COL_ATTR_LOCATION 2
#if ATTRIBUTE_MATRIX
    #define GR_GL_MAT_ATTR_LOCATION 3
    #define GR_GL_TEXMAT_ATTR_LOCATION 6
#endif

GLuint GrGpuGLShaders::loadShader(GLenum type, const char* src) {
    GLuint shader = GR_GL(CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    GR_GL(ShaderSource(shader, 1, &src, NULL));
    GR_GL(CompileShader(shader));

    GLint compiled = GR_GL_INIT_ZERO;
    GR_GL(GetShaderiv(shader, GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GLint infoLen = GR_GL_INIT_ZERO;
        GR_GL(GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetShaderInfoLog(shader, infoLen+1, NULL, (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GR_GL(DeleteShader(shader));
        return 0;
    }
    return shader;
}

bool GrGpuGLShaders::createProgram(GLuint vshader, GLuint fshader,
                                   bool hasTexMatrix,
                                   bool hasTexCoords,
                                   GrGpuGLShaders::ColorType colorType,
                                   bool twoPointRadial,
                                   ProgramData* program) {
    program->fProgramID = GR_GL(CreateProgram());
    program->fVShaderID = vshader;
    program->fFShaderID = fshader;

    GrAssert(0 != program->fProgramID);

    GR_GL(AttachShader(program->fProgramID, vshader));
    GR_GL(AttachShader(program->fProgramID, fshader));

    GR_GL(BindAttribLocation(program->fProgramID,
                             GR_GL_POS_ATTR_LOCATION,
                             "aPosition"));
    if (hasTexCoords) {
        GR_GL(BindAttribLocation(program->fProgramID,
                                 GR_GL_TEX_ATTR_LOCATION,
                                 "aTexture"));
    }
#if ATTRIBUTE_MATRIX
    if (hasTexMatrix) {
        GR_GL(BindAttribLocation(program->fProgramID,
                                 GR_GL_TEXMAT_ATTR_LOCATION,
                                 "texM"));
        // set to something arbitrary to signal to flush that program
        // uses the texture matrix.
        program->fTexMatrixLocation = 1000;
    }
#endif
    if (colorType == kAttrib_ColorType) {
        GR_GL(BindAttribLocation(program->fProgramID,
                                 GR_GL_COL_ATTR_LOCATION,
                                 "aColor"));
    }
#if ATTRIBUTE_MATRIX
    GR_GL(BindAttribLocation(program->fProgramID,
                             GR_GL_MAT_ATTR_LOCATION,
                             "viewM"));
#endif

    GR_GL(LinkProgram(program->fProgramID));

    GLint linked = GR_GL_INIT_ZERO;
    GR_GL(GetProgramiv(program->fProgramID, GL_LINK_STATUS, &linked));
    if (!linked) {
        GLint infoLen = GR_GL_INIT_ZERO;
        GR_GL(GetProgramiv(program->fProgramID, GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetProgramInfoLog(program->fProgramID,
                                    infoLen+1,
                                    NULL,
                                    (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GR_GL(DeleteProgram(program->fProgramID));
        program->fProgramID = 0;
        return false;
    }
    program->fColorType = colorType;

#if !ATTRIBUTE_MATRIX
    program->fMatrixLocation =
        GR_GL(GetUniformLocation(program->fProgramID, "viewM"));
    program->fTexMatrixLocation =
        GR_GL(GetUniformLocation(program->fProgramID, "texM"));
#endif
    program->fColorLocation =
        GR_GL(GetUniformLocation(program->fProgramID, "uColor"));
    program->fTwoPointParamsLocation =
        GR_GL(GetUniformLocation(program->fProgramID, "uParams"));

    GLint samplerLocation =
                GR_GL(GetUniformLocation(program->fProgramID, "sTexture"));

#if !ATTRIBUTE_MATRIX
    if (-1 == program->fMatrixLocation)    {
        GrAssert(!"Cannot find matrix uniform in program");
        GR_GL(DeleteProgram(program->fProgramID));
        program->fProgramID = 0;
        return false;
    }
#endif

    bool hasTexture = hasTexCoords || hasTexMatrix;

    if (-1 == samplerLocation && hasTexture) {
        GrAssert(!"Expected to find texture sampler");
        GR_GL(DeleteProgram(program->fProgramID));
        program->fProgramID = 0;
        return false;
    } else if (-1 != samplerLocation && !hasTexture) {
        GrAssert(!"unexpectedly found texture sampler");
    }
#if !ATTRIBUTE_MATRIX
    if (-1 == program->fTexMatrixLocation && hasTexMatrix) {
        GrAssert(!"Expected to find texture matrix");
        GR_GL(DeleteProgram(program->fProgramID));
        program->fProgramID = 0;
        return false;
    } else if (-1 != program->fTexMatrixLocation && !hasTexMatrix) {
        GrAssert(!"unexpectedly found texture matrix");
    }
#endif

    if (-1 == program->fColorLocation &&
        (kUniform_ColorType == colorType)) {
        GR_GL(DeleteProgram(program->fProgramID));
        program->fProgramID = 0;
        return false;
    } else if (-1 != program->fColorLocation &&
        (kUniform_ColorType != colorType)) {
        GrAssert(!"Unexpectedly found color uniform");
    }

    if (twoPointRadial) {
        if (-1 == program->fTwoPointParamsLocation) {
            GrAssert(!"Didn't find expected uniform for 2pt radial gradient");
            GR_GL(DeleteProgram(program->fProgramID));
            program->fProgramID = 0;
            return false;
        }
    } else {
        GrAssert(-1 == program->fTwoPointParamsLocation);
    }

    GR_GL(UseProgram(program->fProgramID));
    if (-1 != samplerLocation) {
        GR_GL(Uniform1i(samplerLocation, 0));
    }

    return true;
}

GrGpuGLShaders::GrGpuGLShaders() {

    resetContextHelper();

    GLuint vshadIDs[GR_ARRAY_COUNT(gvshad)];
    for (size_t s = 0; s < GR_ARRAY_COUNT(gvshad); ++s) {
        vshadIDs[s] = loadShader(GL_VERTEX_SHADER, gvshad[s]);
    }

    GLuint fshadIDs[GR_ARRAY_COUNT(gfshad)];
    for (size_t s = 0; s < GR_ARRAY_COUNT(gfshad); ++s) {
        fshadIDs[s] = loadShader(GL_FRAGMENT_SHADER, gfshad[s]);
    }

    GR_STATIC_ASSERT(kProgramCount == GR_ARRAY_COUNT(gProgramLoadData));
    for (int p = 0; p < kProgramCount; ++p) {
        GR_DEBUGCODE(bool result = )
        createProgram(vshadIDs[gProgramLoadData[p].fVShaderIdx],
                      fshadIDs[gProgramLoadData[p].fFShaderIdx],
                      gProgramLoadData[p].fHasTexMatrix,
                      gProgramLoadData[p].fHasTexCoords,
                      gProgramLoadData[p].fColorType,
                      gProgramLoadData[p].fTwoPointRadial,
                      &fPrograms[p]);
        GR_DEBUGASSERT(result);

        fPrograms[p].fViewMatrix.setScale(GR_ScalarMax, GR_ScalarMax);
        for (int s = 0; s < kNumStages; ++s) {
            fPrograms[p].fTextureMatrices[s].setScale(GR_ScalarMax, 
                                                      GR_ScalarMax); // illegal
        };
        fPrograms[p].fColor = GrColor_ILLEGAL;
        fPrograms[p].fTextureOrientation = (GrGLTexture::Orientation)-1; // illegal

        // these aren't strictly invalid, just really unlikely.
        fPrograms[p].fRadial2CenterX1 = GR_ScalarMin;
        fPrograms[p].fRadial2Radius0  = GR_ScalarMin;
        fPrograms[p].fRadial2PosRoot  = true; // arbitrary
    }
}

GrGpuGLShaders::~GrGpuGLShaders() {
    // shaders get deleted once for each program that uses them, do we care?
    // probably not
    for (int i = 0; i < kProgramCount; ++i) {
        GR_GL(DeleteProgram(fPrograms[i].fProgramID));
        GR_GL(DeleteShader(fPrograms[i].fVShaderID));
        GR_GL(DeleteShader(fPrograms[i].fFShaderID));
    }
}

void GrGpuGLShaders::resetContext() {
    INHERITED::resetContext();
    resetContextHelper();
}

void GrGpuGLShaders::resetContextHelper() {
    fHWProgram = (Programs)-1;
    fTextureOrientation = (GrGLTexture::Orientation)-1; // illegal

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fPositionPtr  = (void*) ~0;
    GR_GL(DisableVertexAttribArray(GR_GL_COL_ATTR_LOCATION));
    GR_GL(DisableVertexAttribArray(GR_GL_TEX_ATTR_LOCATION));
    GR_GL(EnableVertexAttribArray(GR_GL_POS_ATTR_LOCATION));
}


void GrGpuGLShaders::flushMatrix(GLint location) {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
    GrMatrix m (
        GrIntToScalar(2) / fCurrDrawState.fRenderTarget->width(), 0, -GR_Scalar1,
        0,-GrIntToScalar(2) / fCurrDrawState.fRenderTarget->height(), GR_Scalar1,
        0, 0, GrMatrix::I()[8]);
    m.setConcat(m, fCurrDrawState.fViewMatrix);

    // ES doesn't allow you to pass true to the transpose param,
    // so do our own transpose
    GrScalar mt[]  = {
        m[GrMatrix::kScaleX],
        m[GrMatrix::kSkewY],
        m[GrMatrix::kPersp0],
        m[GrMatrix::kSkewX],
        m[GrMatrix::kScaleY],
        m[GrMatrix::kPersp1],
        m[GrMatrix::kTransX],
        m[GrMatrix::kTransY],
        m[GrMatrix::kPersp2]
    };
#if ATTRIBUTE_MATRIX
    glVertexAttrib4fv(GR_GL_MAT_ATTR_LOCATION+0, mt+0);
    glVertexAttrib4fv(GR_GL_MAT_ATTR_LOCATION+1, mt+3);
    glVertexAttrib4fv(GR_GL_MAT_ATTR_LOCATION+2, mt+6);
#else
    GR_GL(UniformMatrix3fv(location,1,false,mt));
#endif
}

void GrGpuGLShaders::flushTexMatrix(GLint location,
                                    GrGLTexture::Orientation orientation) {
    GrMatrix* m;
    GrMatrix temp;
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        temp.setAll(
            GR_Scalar1, 0, 0,
            0, -GR_Scalar1, GR_Scalar1,
            0, 0, GrMatrix::I()[8]
        );
        temp.preConcat(fCurrDrawState.fTextureMatrices[0]);
        m = &temp;
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
        m = &fCurrDrawState.fTextureMatrices[0];
    }

    // ES doesn't allow you to pass true to the transpose param,
    // so do our own transpose
    GrScalar mt[]  = {
        (*m)[GrMatrix::kScaleX],
        (*m)[GrMatrix::kSkewY],
        (*m)[GrMatrix::kPersp0],
        (*m)[GrMatrix::kSkewX],
        (*m)[GrMatrix::kScaleY],
        (*m)[GrMatrix::kPersp1],
        (*m)[GrMatrix::kTransX],
        (*m)[GrMatrix::kTransY],
        (*m)[GrMatrix::kPersp2]
    };
#if ATTRIBUTE_MATRIX
    glVertexAttrib4fv(GR_GL_TEXMAT_ATTR_LOCATION+0, mt+0);
    glVertexAttrib4fv(GR_GL_TEXMAT_ATTR_LOCATION+1, mt+3);
    glVertexAttrib4fv(GR_GL_TEXMAT_ATTR_LOCATION+2, mt+6);
#else
    GR_GL(UniformMatrix3fv(location,1,false,mt));
#endif
}

void GrGpuGLShaders::flushTwoPointRadial(GLint paramsLocation,
                                         const GrSamplerState& state) {
    GrScalar centerX1 = state.getRadial2CenterX1();
    GrScalar radius0 = state.getRadial2Radius0();

    GrScalar a = GrMul(centerX1, centerX1) - GR_Scalar1;

    float unis[6] = {
        GrScalarToFloat(a),
        1 / (2.f * unis[0]),
        GrScalarToFloat(centerX1),
        GrScalarToFloat(radius0),
        GrScalarToFloat(GrMul(radius0, radius0)),
        state.isRadial2PosRoot() ? 1.f : -1.f
    };
    GR_GL(Uniform1fv(paramsLocation, 6, unis));
}

void GrGpuGLShaders::flushProgram(PrimitiveType type) {

    Programs nextProgram = kNoTexture_Program;

    GrTexture* texture = fCurrDrawState.fTextures[0];
    bool posAsTex = 
            StagePosAsTexCoordVertexLayoutBit(0) & fGeometrySrc.fVertexLayout;

    if (!VertexUsesStage(0, fGeometrySrc.fVertexLayout)) {
        goto HAVE_NEXT_PROGRAM;
    }

    GrAssert(NULL != texture);

    switch (fCurrDrawState.fSamplerStates[0].getSampleMode()) {
        case GrSamplerState::kRadial_SampleMode:
            GrAssert(!fCurrDrawState.fTextureMatrices[0].hasPerspective());
            GrAssert(GrTexture::kAlpha_8_PixelConfig != texture->config());
            if (posAsTex) {
                nextProgram = kRadialTextureVertCoords_Program;
            } else {
                nextProgram = kRadialTextureTexCoords_Program;
            }
            break;
        case GrSamplerState::kSweep_SampleMode:
            GrAssert(!fCurrDrawState.fTextureMatrices[0].hasPerspective());
            GrAssert(GrTexture::kAlpha_8_PixelConfig != texture->config());
            if (posAsTex) {
                nextProgram = kSweepTextureVertCoords_Program;
            } else {
                nextProgram = kSweepTextureTexCoords_Program;
            }
            break;
        case GrSamplerState::kRadial2_SampleMode:
            GrAssert(!fCurrDrawState.fTextureMatrices[0].hasPerspective());
            GrAssert(GrTexture::kAlpha_8_PixelConfig != texture->config());
            if (posAsTex) {
                nextProgram = kTwoPointRadialTextureVertCoords_Program;
            } else {
                nextProgram = kTwoPointRadialTextureTexCoords_Program;
            }
            break;
        case GrSamplerState::kNormal_SampleMode:
            if (GrTexture::kAlpha_8_PixelConfig == texture->config()) {
                GrAssert(((GrGLTexture*)texture)->orientation() == 
                         GrGLTexture::kTopDown_Orientation);
                GrAssert(!posAsTex);
                nextProgram = kText_Program;
            } else {
                bool persp = fCurrDrawState.fTextureMatrices[0].hasPerspective();
                if (posAsTex) {
                    nextProgram = persp ? kTextureVertCoordsProj_Program : 
                                          kTextureVertCoords_Program;
                } else {
                    nextProgram = persp ? kTextureTexCoordsProj_Program : 
                                          kTextureTexCoords_Program;
                }
                // check for case when frag shader can skip the color modulation
                if (!persp && !(fGeometrySrc.fVertexLayout
                                & kColor_VertexLayoutBit) &&
                    0xffffffff == fCurrDrawState.fColor) {
                    switch (nextProgram) {
                    case kTextureVertCoords_Program:
                        nextProgram = kTextureVertCoordsNoColor_Program;
                        break;
                    case kTextureTexCoords_Program:
                        nextProgram = kTextureTexCoordsNoColor_Program;
                        break;
                    default:
                        GrAssert("Unexpected");
                        break;
                    }
                }
            }
            break;
        default:
            GrAssert(!"Unknown samplemode");
            break;
    }

HAVE_NEXT_PROGRAM:
    if (fHWProgram != nextProgram) {
        GR_GL(UseProgram(fPrograms[nextProgram].fProgramID));
        fHWProgram = nextProgram;
#if GR_COLLECT_STATS
        ++fStats.fProgChngCnt;
#endif
    }
}

bool GrGpuGLShaders::flushGraphicsState(PrimitiveType type) {

   for (int s = 1; s < kNumStages; ++s) {
        if (VertexUsesStage(s, fGeometrySrc.fVertexLayout)) {
            unimpl("the hard-coded shaders used by this " 
                   "class only support 1 stage");
            return false;
        }
    }

    flushGLStateCommon(type);

    if (fRenderTargetChanged) {
        // our coords are in pixel space and the GL matrices map to NDC
        // so if the viewport changed, our matrix is now wrong.
#if ATTRIBUTE_MATRIX
        fHWDrawState.fViewMatrix.setScale(GR_ScalarMax, GR_ScalarMax);
#else
        // we assume all shader matrices may be wrong after viewport changes
        for (int p = 0; p < kProgramCount; ++p) {
            // set to illegal matrix
            fPrograms[p].fViewMatrix.setScale(GR_ScalarMax, GR_ScalarMax); 
        }
#endif
        fRenderTargetChanged = false;
    }

    flushProgram(type);

    if (fGeometrySrc.fVertexLayout & kColor_VertexLayoutBit) {
        // invalidate the immediate mode color
        fHWDrawState.fColor = GrColor_ILLEGAL;
    } else {
        // if we don't have per-vert colors either set the color attr
        // or color uniform (depending on which program).
        if (-1 != fPrograms[fHWProgram].fColorLocation) {
            GrAssert(kUniform_ColorType == fPrograms[fHWProgram].fColorType);
            if (fPrograms[fHWProgram].fColor != fCurrDrawState.fColor) {
                float c[] = {
                    GrColorUnpackR(fCurrDrawState.fColor) / 255.f,
                    GrColorUnpackG(fCurrDrawState.fColor) / 255.f,
                    GrColorUnpackB(fCurrDrawState.fColor) / 255.f,
                    GrColorUnpackA(fCurrDrawState.fColor) / 255.f
                };
                GR_GL(Uniform4fv(fPrograms[fHWProgram].fColorLocation, 1, c));
                fPrograms[fHWProgram].fColor = fCurrDrawState.fColor;
            }
        } else if (kAttrib_ColorType == fPrograms[fHWProgram].fColorType &&
                   fHWDrawState.fColor != fCurrDrawState.fColor) {
            // OpenGL ES only supports the float varities of glVertexAttrib
            float c[] = {
                GrColorUnpackR(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackG(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackB(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackA(fCurrDrawState.fColor) / 255.f
            };
            GR_GL(VertexAttrib4fv(GR_GL_COL_ATTR_LOCATION, c));
            fHWDrawState.fColor = fCurrDrawState.fColor;
        }
    }

#if ATTRIBUTE_MATRIX
    GrMatrix& currentViewMatrix = fHWDrawState.fViewMatrix;
    GrMatrix& currentTexMatrix = fHWDrawState.fTextureMatrices[0];
    GrGLTexture::Orientation& orientation = fTextureOrientation;
#else
    GrMatrix& currentViewMatrix = fPrograms[fHWProgram].fViewMatrix;
    GrMatrix& currentTexMatrix = fPrograms[fHWProgram].fTextureMatrices[0];   
    GrGLTexture::Orientation& orientation =
                                    fPrograms[fHWProgram].fTextureOrientation;
#endif

    if (currentViewMatrix !=
          fCurrDrawState.fViewMatrix) {
        flushMatrix(fPrograms[fHWProgram].fMatrixLocation);
        currentViewMatrix = fCurrDrawState.fViewMatrix;
    }

    GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[0];
    if (NULL != texture) {
        if (-1 != fPrograms[fHWProgram].fTexMatrixLocation &&
            (currentTexMatrix != fCurrDrawState.fTextureMatrices[0] ||
             orientation != texture->orientation())) {
            flushTexMatrix(fPrograms[fHWProgram].fTexMatrixLocation,
                           texture->orientation());
            currentTexMatrix = fCurrDrawState.fTextureMatrices[0];
            orientation = texture->orientation();
        }
    }

    const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[0];
    if (-1 != fPrograms[fHWProgram].fTwoPointParamsLocation &&
        (fPrograms[fHWProgram].fRadial2CenterX1 != sampler.getRadial2CenterX1() ||
         fPrograms[fHWProgram].fRadial2Radius0  != sampler.getRadial2Radius0()  ||
         fPrograms[fHWProgram].fRadial2PosRoot  != sampler.isRadial2PosRoot())) {

        flushTwoPointRadial(fPrograms[fHWProgram].fTwoPointParamsLocation,
                            sampler);
        fPrograms[fHWProgram].fRadial2CenterX1 = sampler.getRadial2CenterX1();
        fPrograms[fHWProgram].fRadial2Radius0 = sampler.getRadial2Radius0();
        fPrograms[fHWProgram].fRadial2PosRoot = sampler.isRadial2PosRoot();
    }

    return true;
}

void GrGpuGLShaders::setupGeometry(uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount) {

    int newColorOffset;
    int newTexCoordOffsets[kNumStages];

    GLsizei newStride = VertexSizeAndOffsetsByStage(fGeometrySrc.fVertexLayout,
                                                    newTexCoordOffsets, 
                                                    &newColorOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kNumStages];
    GLsizei oldStride = VertexSizeAndOffsetsByStage(fHWGeometryState.fVertexLayout,
                                                    oldTexCoordOffsets, 
                                                    &oldColorOffset);

    const GLvoid* posPtr = (GLvoid*)(newStride * startVertex);

    if (kBuffer_GeometrySrcType == fGeometrySrc.fVertexSrc) {
        GrAssert(NULL != fGeometrySrc.fVertexBuffer);
        GrAssert(!fGeometrySrc.fVertexBuffer->isLocked());
        if (fHWGeometryState.fVertexBuffer != fGeometrySrc.fVertexBuffer) {
            GrGLVertexBuffer* buf =
            (GrGLVertexBuffer*)fGeometrySrc.fVertexBuffer;
            GR_GL(BindBuffer(GL_ARRAY_BUFFER, buf->bufferID()));
            fHWGeometryState.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        }
    } else {
        if (kArray_GeometrySrcType == fGeometrySrc.fVertexSrc) {
            posPtr = (void*)((intptr_t)fGeometrySrc.fVertexArray +
                             (intptr_t)posPtr);
        } else {
            GrAssert(kReserved_GeometrySrcType == fGeometrySrc.fVertexSrc);
            posPtr = (void*)((intptr_t)fVertices.get() + (intptr_t)posPtr);
        }
        if (NULL != fHWGeometryState.fVertexBuffer) {
            GR_GL(BindBuffer(GL_ARRAY_BUFFER, 0));
            fHWGeometryState.fVertexBuffer = NULL;
        }
    }

    if (kBuffer_GeometrySrcType == fGeometrySrc.fIndexSrc) {
        GrAssert(NULL != fGeometrySrc.fIndexBuffer);
        GrAssert(!fGeometrySrc.fIndexBuffer->isLocked());
        if (fHWGeometryState.fIndexBuffer != fGeometrySrc.fIndexBuffer) {
            GrGLIndexBuffer* buf =
            (GrGLIndexBuffer*)fGeometrySrc.fIndexBuffer;
            GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->bufferID()));
            fHWGeometryState.fIndexBuffer = fGeometrySrc.fIndexBuffer;
        }
    } else if (NULL != fHWGeometryState.fIndexBuffer) {
        GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        fHWGeometryState.fIndexBuffer = NULL;
    }

    GLenum scalarType;
    bool texCoordNorm;
    if (fGeometrySrc.fVertexLayout & kTextFormat_VertexLayoutBit) {
        scalarType = GrGLTextType;
        texCoordNorm = GR_GL_TEXT_TEXTURE_NORMALIZED;
    } else {
        scalarType = GrGLType;
        texCoordNorm = false;
    }

    bool baseChange = posPtr != fHWGeometryState.fPositionPtr;
    bool scalarChange = (GrGLTextType != GrGLType) &&
                        (kTextFormat_VertexLayoutBit &
                         (fHWGeometryState.fVertexLayout ^
                          fGeometrySrc.fVertexLayout));
    bool strideChange = newStride != oldStride;
    bool posChange = baseChange || scalarChange || strideChange;

    if (posChange) {
        GR_GL(VertexAttribPointer(GR_GL_POS_ATTR_LOCATION, 2, scalarType,
                                  false, newStride, posPtr));
        fHWGeometryState.fPositionPtr = posPtr;
    }

    // this class only supports one stage.
    if (newTexCoordOffsets[0] > 0) {
        GLvoid* texCoordPtr = (int8_t*)posPtr + newTexCoordOffsets[0];
        if (oldTexCoordOffsets[0] <= 0) {
            GR_GL(EnableVertexAttribArray(GR_GL_TEX_ATTR_LOCATION));
        }
        if (posChange || newTexCoordOffsets[0] != oldTexCoordOffsets[0]) {
            GR_GL(VertexAttribPointer(GR_GL_TEX_ATTR_LOCATION, 2, scalarType,
                                      texCoordNorm, newStride, texCoordPtr));
        }
    } else if (oldTexCoordOffsets[0] > 0) {
        GR_GL(DisableVertexAttribArray(GR_GL_TEX_ATTR_LOCATION));
    }

    if (newColorOffset > 0) {
        GLvoid* colorPtr = (int8_t*)posPtr + newColorOffset;
        if (oldColorOffset <= 0) {
            GR_GL(EnableVertexAttribArray(GR_GL_COL_ATTR_LOCATION));
        }
        if (posChange || newColorOffset != oldColorOffset) {
            GR_GL(VertexAttribPointer(GR_GL_COL_ATTR_LOCATION, 4,
                                      GL_UNSIGNED_BYTE,
                                      true, newStride, colorPtr));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableVertexAttribArray(GR_GL_COL_ATTR_LOCATION));
    }

    fHWGeometryState.fVertexLayout = fGeometrySrc.fVertexLayout;
}
#endif
