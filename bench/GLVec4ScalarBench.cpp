/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GLBench.h"
#include "GrShaderCaps.h"
#include "GrShaderVar.h"
#include "gl/GrGLContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "../private/GrGLSL.h"

#include <stdio.h>

/**
 * This is a GL benchmark for comparing the performance of using vec4 or float for coverage in GLSL.
 * The generated shader code from this bench will draw several overlapping circles, one in each
 * stage, to simulate coverage calculations.  The number of circles (i.e. the number of stages) can
 * be set as a parameter.
 */

class GLVec4ScalarBench : public GLBench {
public:
    /*
     * Use float or vec4 as GLSL data type for the output coverage
     */
    enum CoverageSetup {
        kUseScalar_CoverageSetup,
        kUseVec4_CoverageSetup,
    };

    /*
     * numStages determines the number of shader stages before the XP,
     * which consequently determines how many circles are drawn
     */
    GLVec4ScalarBench(CoverageSetup coverageSetup, uint32_t numStages)
        : fCoverageSetup(coverageSetup)
        , fNumStages(numStages)
        , fVboId(0)
        , fProgram(0) {
        fName = NumStagesSetupToStr(coverageSetup, numStages);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void setup(const GrGLContext*) override;
    void glDraw(int loops, const GrGLContext*) override;
    void teardown(const GrGLInterface*) override;

private:
    void setupSingleVbo(const GrGLInterface*, const SkMatrix*);
    GrGLuint setupShader(const GrGLContext*);


    static SkString NumStagesSetupToStr(CoverageSetup coverageSetup, uint32_t numStages) {
        SkString name("GLVec4ScalarBench");
        switch (coverageSetup) {
            default:
            case kUseScalar_CoverageSetup:
                name.appendf("_scalar_%u_stage", numStages);
                break;
            case kUseVec4_CoverageSetup:
                name.appendf("_vec4_%u_stage", numStages);
                break;
        }
        return name;
    }

    static const GrGLuint kScreenWidth = 800;
    static const GrGLuint kScreenHeight = 600;
    static const uint32_t kNumTriPerDraw = 512;
    static const uint32_t kVerticesPerTri = 3;

    SkString fName;
    CoverageSetup fCoverageSetup;
    uint32_t fNumStages;
    GrGLuint fVboId;
    GrGLuint fProgram;
    GrGLuint fFboTextureId;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLuint GLVec4ScalarBench::setupShader(const GrGLContext* ctx) {
    const GrShaderCaps* shaderCaps = ctx->caps()->shaderCaps();
    const char* version = shaderCaps->versionDeclString();

    // this shader draws fNumStages overlapping circles of increasing opacity (coverage) and
    // decreasing size, with the center of each subsequent circle closer to the bottom-right
    // corner of the screen than the previous circle.

    // set up vertex shader; this is a trivial vertex shader that passes through position and color
    GrShaderVar aPosition("a_position", kVec2f_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar oPosition("o_position", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier);
    GrShaderVar aColor("a_color", kVec3f_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar oColor("o_color", kVec3f_GrSLType, GrShaderVar::kOut_TypeModifier);

    SkString vshaderTxt(version);
    aPosition.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";\n");
    aColor.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";\n");
    oPosition.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";\n");
    oColor.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";\n");

    vshaderTxt.append(
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
            "    o_position = a_position;\n"
            "    o_color = a_color;\n"
            "}\n");

    // set up fragment shader; this fragment shader will have fNumStages coverage stages plus an
    // XP stage at the end.  Each coverage stage computes the pixel's distance from some hard-
    // coded center and compare that to some hard-coded circle radius to compute a coverage.
    // Then, this coverage is mixed with the coverage from the previous stage and passed to the
    // next stage.
    GrShaderVar oFragColor("o_FragColor", kVec4f_GrSLType, GrShaderVar::kOut_TypeModifier);
    SkString fshaderTxt(version);
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kMedium_GrSLPrecision, *shaderCaps, &fshaderTxt);
    oPosition.setTypeModifier(GrShaderVar::kIn_TypeModifier);
    oPosition.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";\n");
    oColor.setTypeModifier(GrShaderVar::kIn_TypeModifier);
    oColor.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";\n");

    const char* fsOutName;
    if (shaderCaps->mustDeclareFragmentShaderOutput()) {
        oFragColor.appendDecl(shaderCaps, &fshaderTxt);
        fshaderTxt.append(";\n");
        fsOutName = oFragColor.c_str();
    } else {
        fsOutName = "sk_FragColor";
    }


    fshaderTxt.appendf(
            "void main()\n"
            "{\n"
            "    vec4 outputColor;\n"
            "    %s outputCoverage;\n"
            "    outputColor = vec4(%s, 1.0);\n"
            "    outputCoverage = %s;\n",
            fCoverageSetup == kUseVec4_CoverageSetup ? "vec4" : "float",
            oColor.getName().c_str(),
            fCoverageSetup == kUseVec4_CoverageSetup ? "vec4(1.0)" : "1.0"
            );

    float radius = 1.0f;
    for (uint32_t i = 0; i < fNumStages; i++) {
        float centerX = 1.0f - radius;
        float centerY = 1.0f - radius;
        fshaderTxt.appendf(
            "    {\n"
            "        float d = length(%s - vec2(%f, %f));\n"
            "        float edgeAlpha = clamp(100.0 * (%f - d), 0.0, 1.0);\n"
            "        outputCoverage = 0.5 * outputCoverage + 0.5 * %s;\n"
            "    }\n",
            oPosition.getName().c_str(), centerX, centerY,
            radius,
            fCoverageSetup == kUseVec4_CoverageSetup ? "vec4(edgeAlpha)" : "edgeAlpha"
            );
        radius *= 0.8f;
    }
    fshaderTxt.appendf(
            "    {\n"
            "        %s = outputColor * outputCoverage;\n"
            "    }\n"
            "}\n",
            fsOutName);

    return CreateProgram(ctx, vshaderTxt.c_str(), fshaderTxt.c_str());
}

template<typename Func>
static void setup_matrices(int numQuads, Func f) {
    // We draw a really small triangle so we are not fill rate limited
    for (int i = 0 ; i < numQuads; i++) {
        SkMatrix m = SkMatrix::I();
        m.setScale(0.01f, 0.01f);
        f(m);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Vertex {
    SkPoint fPositions;
    GrGLfloat fColors[3];
};

void GLVec4ScalarBench::setupSingleVbo(const GrGLInterface* gl, const SkMatrix* viewMatrices) {
    // triangles drawn will alternate between the top-right half of the screen and the bottom-left
    // half of the screen
    Vertex vertices[kVerticesPerTri * kNumTriPerDraw];
    for (uint32_t i = 0; i < kNumTriPerDraw; i++) {
        Vertex* v = &vertices[i * kVerticesPerTri];
        if (i % 2 == 0) {
            v[0].fPositions.set(-1.0f, -1.0f);
            v[1].fPositions.set( 1.0f, -1.0f);
            v[2].fPositions.set( 1.0f,  1.0f);
        } else {
            v[0].fPositions.set(-1.0f, -1.0f);
            v[1].fPositions.set( 1.0f, 1.0f);
            v[2].fPositions.set( -1.0f, 1.0f);
        }
        SkPoint* position = reinterpret_cast<SkPoint*>(v);
        viewMatrices[i].mapPointsWithStride(position, sizeof(Vertex), kVerticesPerTri);

        GrGLfloat color[3] = {1.0f, 0.0f, 1.0f};
        for (uint32_t j = 0; j < kVerticesPerTri; j++) {
            v->fColors[0] = color[0];
            v->fColors[1] = color[1];
            v->fColors[2] = color[2];
            v++;
        }
    }

    GR_GL_CALL(gl, GenBuffers(1, &fVboId));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, fVboId));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, EnableVertexAttribArray(1));
    GR_GL_CALL(gl, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, sizeof(Vertex),
                                       (GrGLvoid*)0));
    GR_GL_CALL(gl, VertexAttribPointer(1, 3, GR_GL_FLOAT, GR_GL_FALSE, sizeof(Vertex),
                                       (GrGLvoid*)(sizeof(SkPoint))));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(vertices), vertices, GR_GL_STATIC_DRAW));
}

void GLVec4ScalarBench::setup(const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();
    if (!gl) {
        SkFAIL("GL interface is nullptr in setup()!\n");
    }
    fFboTextureId = SetupFramebuffer(gl, kScreenWidth, kScreenHeight);

    fProgram = this->setupShader(ctx);

    int index = 0;
    SkMatrix viewMatrices[kNumTriPerDraw];
    setup_matrices(kNumTriPerDraw, [&index, &viewMatrices](const SkMatrix& m) {
        viewMatrices[index++] = m;
    });
    this->setupSingleVbo(gl, viewMatrices);

    GR_GL_CALL(gl, UseProgram(fProgram));
}

void GLVec4ScalarBench::glDraw(int loops, const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();

    for (int i = 0; i < loops; i++) {
        GR_GL_CALL(gl, DrawArrays(GR_GL_TRIANGLES, 0, kVerticesPerTri * kNumTriPerDraw));
    }

// using -w when running nanobench will not produce correct images;
// changing this to #if 1 will write the correct images to the Skia folder.
#if 0
    SkString filename("out");
    filename.appendf("_%s.png", this->getName());
    DumpImage(gl, kScreenWidth, kScreenHeight, filename.c_str());
#endif
}

void GLVec4ScalarBench::teardown(const GrGLInterface* gl) {
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, 0));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
    GR_GL_CALL(gl, DeleteTextures(1, &fFboTextureId));
    GR_GL_CALL(gl, DeleteProgram(fProgram));
    GR_GL_CALL(gl, DeleteBuffers(1, &fVboId));
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseScalar_CoverageSetup, 1) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseVec4_CoverageSetup, 1) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseScalar_CoverageSetup, 2) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseVec4_CoverageSetup, 2) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseScalar_CoverageSetup, 4) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseVec4_CoverageSetup, 4) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseScalar_CoverageSetup, 6) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseVec4_CoverageSetup, 6) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseScalar_CoverageSetup, 8) )
DEF_BENCH( return new GLVec4ScalarBench(GLVec4ScalarBench::kUseVec4_CoverageSetup, 8) )

#endif
