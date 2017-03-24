/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImageEncoder.h"

#if SK_SUPPORT_GPU
#include "GLBench.h"
#include "GrShaderCaps.h"
#include "GrShaderVar.h"
#include "gl/GrGLContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "../private/GrGLSL.h"
#include <stdio.h>

/*
 * This is a native GL benchmark for determining the cost of uploading vertex attributes
 */
class GLVertexAttributesBench : public GLBench {
public:
    GLVertexAttributesBench(uint32_t attribs)
        : fTexture(0)
        , fBuffers(0)
        , fProgram(0)
        , fVBO(0)
        , fAttribs(attribs)
        , fStride(2 * sizeof(SkPoint) + fAttribs * sizeof(GrGLfloat) * 4) {
        fName.appendf("GLVertexAttributesBench_%d", fAttribs);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    void setup(const GrGLContext*) override;
    void glDraw(int loops, const GrGLContext*) override;
    void teardown(const GrGLInterface*) override;

    static const GrGLuint kScreenWidth = 800;
    static const GrGLuint kScreenHeight = 600;
    static const uint32_t kNumTri = 10000;
    static const uint32_t kVerticesPerTri = 3;
    static const uint32_t kDrawMultiplier = 512;
    static const uint32_t kMaxAttribs = 7;

private:
    GrGLuint setupShader(const GrGLContext*, uint32_t attribs, uint32_t maxAttribs);

    GrGLuint fTexture;
    SkTArray<GrGLuint> fBuffers;
    GrGLuint fProgram;
    GrGLuint fVBO;
    SkTArray<unsigned char> fVertices;
    uint32_t fAttribs;
    size_t fStride;
    SkString fName;
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLuint GLVertexAttributesBench::setupShader(const GrGLContext* ctx, uint32_t attribs,
                                              uint32_t maxAttribs) {
    const GrShaderCaps* shaderCaps = ctx->caps()->shaderCaps();
    const char* version = shaderCaps->versionDeclString();

    // setup vertex shader
    GrShaderVar aPosition("a_position", kVec4f_GrSLType, GrShaderVar::kIn_TypeModifier);
    SkTArray<GrShaderVar> aVars;
    SkTArray<GrShaderVar> oVars;

    SkString vshaderTxt(version);
    aPosition.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";\n");

    for (uint32_t i = 0; i < attribs; i++) {
        SkString aname;
        aname.appendf("a_color_%d", i);
        aVars.push_back(GrShaderVar(aname.c_str(),
                                    kVec4f_GrSLType,
                                    GrShaderVar::kIn_TypeModifier));
        aVars.back().appendDecl(shaderCaps, &vshaderTxt);
        vshaderTxt.append(";\n");

    }

    for (uint32_t i = 0; i < maxAttribs; i++) {
        SkString oname;
        oname.appendf("o_color_%d", i);
        oVars.push_back(GrShaderVar(oname.c_str(),
                                    kVec4f_GrSLType,
                                    GrShaderVar::kOut_TypeModifier));
        oVars.back().appendDecl(shaderCaps, &vshaderTxt);
        vshaderTxt.append(";\n");
    }

    vshaderTxt.append(
            "void main()\n"
            "{\n"
                "gl_Position = a_position;\n");

    for (uint32_t i = 0; i < attribs; i++) {
        vshaderTxt.appendf("%s = %s;\n", oVars[i].c_str(), aVars[i].c_str());
    }

    // Passthrough position as a dummy
    for (uint32_t i = attribs; i < maxAttribs; i++) {
        vshaderTxt.appendf("%s = vec4(0, 0, 0, 1);\n", oVars[i].c_str());
    }

    vshaderTxt.append("}\n");

    // setup fragment shader
    GrShaderVar oFragColor("o_FragColor", kVec4f_GrSLType, GrShaderVar::kOut_TypeModifier);
    SkString fshaderTxt(version);
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kMedium_GrSLPrecision, *shaderCaps, &fshaderTxt);

    const char* fsOutName;
    if (shaderCaps->mustDeclareFragmentShaderOutput()) {
        oFragColor.appendDecl(shaderCaps, &fshaderTxt);
        fshaderTxt.append(";\n");
        fsOutName = oFragColor.c_str();
    } else {
        fsOutName = "sk_FragColor";
    }

    for (uint32_t i = 0; i < maxAttribs; i++) {
        oVars[i].setTypeModifier(GrShaderVar::kIn_TypeModifier);
        oVars[i].appendDecl(shaderCaps, &fshaderTxt);
        fshaderTxt.append(";\n");
    }

    fshaderTxt.appendf(
            "void main()\n"
            "{\n"
               "%s = ", fsOutName);

    fshaderTxt.appendf("%s", oVars[0].c_str());
    for (uint32_t i = 1; i < maxAttribs; i++) {
        fshaderTxt.appendf(" + %s", oVars[i].c_str());
    }

    fshaderTxt.append(";\n"
                      "}\n");

    return CreateProgram(ctx, vshaderTxt.c_str(), fshaderTxt.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void GLVertexAttributesBench::setup(const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();
    fTexture = SetupFramebuffer(gl, kScreenWidth, kScreenHeight);

    fProgram = setupShader(ctx, fAttribs, kMaxAttribs);

    // setup matrices
    SkMatrix viewMatrices[kNumTri];
    for (uint32_t i = 0 ; i < kNumTri; i++) {
        SkMatrix m = SkMatrix::I();
        m.setScale(0.0001f, 0.0001f);
        viewMatrices[i] = m;
    }

    // presetup vertex attributes, color is set to be a light gray no matter how many vertex
    // attributes are used
    float targetColor = 0.9f;
    float colorContribution = targetColor / fAttribs;
    fVertices.reset(static_cast<int>(kVerticesPerTri * kNumTri * fStride));
    for (uint32_t i = 0; i < kNumTri; i++) {
        unsigned char* ptr = &fVertices[static_cast<int>(i * kVerticesPerTri * fStride)];
        SkPoint* p = reinterpret_cast<SkPoint*>(ptr);
        p->set(-1.0f, -1.0f); p++; p->set( 0.0f, 1.0f);
        p = reinterpret_cast<SkPoint*>(ptr + fStride);
        p->set( 1.0f, -1.0f); p++; p->set( 0.0f, 1.0f);
        p = reinterpret_cast<SkPoint*>(ptr + fStride * 2);
        p->set( 1.0f,  1.0f); p++; p->set( 0.0f, 1.0f);

        SkPoint* position = reinterpret_cast<SkPoint*>(ptr);
        viewMatrices[i].mapPointsWithStride(position, fStride, kVerticesPerTri);

        // set colors
        for (uint32_t j = 0; j < kVerticesPerTri; j++) {
            GrGLfloat* f = reinterpret_cast<GrGLfloat*>(ptr + 2 * sizeof(SkPoint) + fStride * j);
            for (uint32_t k = 0; k < fAttribs * 4; k += 4) {
                f[k] = colorContribution;
                f[k + 1] = colorContribution;
                f[k + 2] = colorContribution;
                f[k + 3] = 1.0f;
            }
        }
    }

    GR_GL_CALL(gl, GenBuffers(1, &fVBO));
    fBuffers.push_back(fVBO);

    // clear screen
    GR_GL_CALL(gl, ClearColor(0.03f, 0.03f, 0.03f, 1.0f));
    GR_GL_CALL(gl, Clear(GR_GL_COLOR_BUFFER_BIT));

    // set us up to draw
    GR_GL_CALL(gl, UseProgram(fProgram));
}

void GLVertexAttributesBench::glDraw(int loops, const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();

    // upload vertex attributes
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, fVBO));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, VertexAttribPointer(0, 4, GR_GL_FLOAT, GR_GL_FALSE, (GrGLsizei)fStride,
                                       (GrGLvoid*)0));

    size_t runningStride = 2 * sizeof(SkPoint);
    for (uint32_t i = 0; i < fAttribs; i++) {
        int attribId = i + 1;
        GR_GL_CALL(gl, EnableVertexAttribArray(attribId));
        GR_GL_CALL(gl, VertexAttribPointer(attribId, 4, GR_GL_FLOAT, GR_GL_FALSE,
                                           (GrGLsizei)fStride, (GrGLvoid*)(runningStride)));
        runningStride += sizeof(GrGLfloat) * 4;
    }

    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, fVertices.count(), fVertices.begin(),
                              GR_GL_STREAM_DRAW));

    uint32_t maxTrianglesPerFlush = kNumTri;
    uint32_t trianglesToDraw = loops * kDrawMultiplier;

    while (trianglesToDraw > 0) {
        uint32_t triangles = SkTMin(trianglesToDraw, maxTrianglesPerFlush);
        GR_GL_CALL(gl, DrawArrays(GR_GL_TRIANGLES, 0, kVerticesPerTri * triangles));
        trianglesToDraw -= triangles;
    }

#if 0
    //const char* filename = "/data/local/tmp/out.png";
    SkString filename("out");
    filename.appendf("_%s.png", this->getName());
    DumpImage(gl, kScreenWidth, kScreenHeight, filename.c_str());
#endif
}

void GLVertexAttributesBench::teardown(const GrGLInterface* gl) {
    // teardown
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, 0));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
    GR_GL_CALL(gl, DeleteTextures(1, &fTexture));
    GR_GL_CALL(gl, DeleteProgram(fProgram));
    GR_GL_CALL(gl, DeleteBuffers(fBuffers.count(), fBuffers.begin()));
    fBuffers.reset();
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GLVertexAttributesBench(0) )
DEF_BENCH( return new GLVertexAttributesBench(1) )
DEF_BENCH( return new GLVertexAttributesBench(2) )
DEF_BENCH( return new GLVertexAttributesBench(3) )
DEF_BENCH( return new GLVertexAttributesBench(4) )
DEF_BENCH( return new GLVertexAttributesBench(5) )
DEF_BENCH( return new GLVertexAttributesBench(6) )
DEF_BENCH( return new GLVertexAttributesBench(7) )
#endif
