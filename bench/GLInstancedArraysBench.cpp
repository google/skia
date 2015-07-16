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
#include "gl/GrGLGLSL.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLShaderVar.h"
#include "gl/GrGLUtil.h"
#include "glsl/GrGLSLCaps.h"

/*
 * This is a native GL benchmark for instanced arrays vs vertex buffer objects.  To benchmark this
 * functionality, we draw n * kDrawMultipier triangles per run.  If this number is less than
 * kNumTri then we do a single draw, either with instances, or drawArrays.  Otherwise we do
 * multiple draws.
 *
 * Additionally, there is a divisor, which if > 0 will act as a multiplier for the number of draws
 * issued.
 */

class GLCpuPosInstancedArraysBench : public GLBench {
public:
    /*
     * Clients can decide to use either:
     * kUseOne_VboSetup      - one vertex buffer with colors and positions interleaved
     * kUseTwo_VboSetup      - two vertex buffers, one for colors, one for positions
     * kUseInstance_VboSetup - two vertex buffers, one with per vertex indices, one with per
     *                         instance colors
     */
    enum VboSetup {
        kUseOne_VboSetup,
        kUseTwo_VboSetup,
        kUseInstance_VboSetup,
    };

    /*
     * drawDiv will act as a multiplier for the number of draws we issue if > 0. ie, 2 will issue
     * 2x as many draws, 4 will issue 4x as many draws etc.  There is a limit however, which is
     * kDrawMultipier.
     */
    GLCpuPosInstancedArraysBench(VboSetup vboSetup, int32_t drawDiv)
        : fVboSetup(vboSetup)
        , fDrawDiv(drawDiv)
        , fProgram(0)
        , fVAO(0) {
        fName = VboSetupToStr(vboSetup, fDrawDiv);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    const GrGLContext* onGetGLContext(const GrGLContext*) override;
    void setup(const GrGLContext*) override;
    void glDraw(const int loops, const GrGLContext*) override;
    void teardown(const GrGLInterface*) override;

private:
    void setupInstanceVbo(const GrGLInterface*, const SkMatrix*);
    void setupDoubleVbo(const GrGLInterface*, const SkMatrix*);
    void setupSingleVbo(const GrGLInterface*, const SkMatrix*);
    GrGLuint setupShader(const GrGLContext*);

    static SkString VboSetupToStr(VboSetup vboSetup, uint32_t drawDiv) {
        SkString name("GLInstancedArraysBench");
        switch (vboSetup) {
            default:
            case kUseOne_VboSetup:
                name.appendf("_one_%u", drawDiv);
                break;
            case kUseTwo_VboSetup:
                name.appendf("_two_%u", drawDiv);
                break;
            case kUseInstance_VboSetup:
                name.append("_instance");
                break;
        }
        return name;
    }

    static const GrGLuint kScreenWidth = 800;
    static const GrGLuint kScreenHeight = 600;
    static const uint32_t kNumTri = 10000;
    static const uint32_t kVerticesPerTri = 3;
    static const uint32_t kDrawMultiplier = 512;

    SkString fName;
    VboSetup fVboSetup;
    uint32_t fDrawDiv;
    SkTArray<GrGLuint> fBuffers;
    GrGLuint fProgram;
    GrGLuint fVAO;
    GrGLuint fTexture;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLuint GLCpuPosInstancedArraysBench::setupShader(const GrGLContext* ctx) {
    const char* version = GrGLGetGLSLVersionDecl(*ctx);

    // setup vertex shader
    GrGLShaderVar aPosition("a_position", kVec2f_GrSLType, GrShaderVar::kAttribute_TypeModifier);
    GrGLShaderVar aColor("a_color", kVec3f_GrSLType, GrShaderVar::kAttribute_TypeModifier);
    GrGLShaderVar oColor("o_color", kVec3f_GrSLType, GrShaderVar::kVaryingOut_TypeModifier);

    SkString vshaderTxt(version);
    aPosition.appendDecl(*ctx, &vshaderTxt);
    vshaderTxt.append(";\n");
    aColor.appendDecl(*ctx, &vshaderTxt);
    vshaderTxt.append(";\n");
    oColor.appendDecl(*ctx, &vshaderTxt);
    vshaderTxt.append(";\n");

    vshaderTxt.append(
            "void main()\n"
            "{\n"
                "gl_Position = vec4(a_position, 0.f, 1.f);\n"
                "o_color = a_color;\n"
            "}\n");

    const GrGLInterface* gl = ctx->interface();

    // setup fragment shader
    GrGLShaderVar oFragColor("o_FragColor", kVec4f_GrSLType, GrShaderVar::kOut_TypeModifier);
    SkString fshaderTxt(version);
    GrGLAppendGLSLDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision, gl->fStandard,
                                                   &fshaderTxt);
    oColor.setTypeModifier(GrShaderVar::kVaryingIn_TypeModifier);
    oColor.appendDecl(*ctx, &fshaderTxt);
    fshaderTxt.append(";\n");

    const char* fsOutName;
    if (ctx->caps()->glslCaps()->mustDeclareFragmentShaderOutput()) {
        oFragColor.appendDecl(*ctx, &fshaderTxt);
        fshaderTxt.append(";\n");
        fsOutName = oFragColor.c_str();
    } else {
        fsOutName = "gl_FragColor";
    }

    fshaderTxt.appendf(
            "void main()\n"
            "{\n"
                "%s = vec4(o_color, 1.0f);\n"
            "}\n", fsOutName);

    return CreateProgram(gl, vshaderTxt.c_str(), fshaderTxt.c_str());
}

template<typename Func>
static void setup_matrices(int numQuads, Func f) {
    // We draw a really small triangle so we are not fill rate limited
    for (int i = 0 ; i < numQuads; i++) {
        SkMatrix m = SkMatrix::I();
        m.setScale(0.0001f, 0.0001f);
        f(m);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const GrGLContext* GLCpuPosInstancedArraysBench::onGetGLContext(const GrGLContext* ctx) {
    // We only care about gpus with drawArraysInstanced support
    if (!ctx->interface()->fFunctions.fDrawArraysInstanced) {
        return NULL;
    }
    return ctx;
}

void GLCpuPosInstancedArraysBench::setupInstanceVbo(const GrGLInterface* gl,
                                                    const SkMatrix* viewMatrices) {
    // We draw all of the instances at a single place because we aren't allowed to have per vertex
    // per instance attributes
    SkPoint positions[kVerticesPerTri];
    positions[0].set(-1.0f, -1.0f);
    positions[1].set( 1.0f, -1.0f);
    positions[2].set( 1.0f,  1.0f);
    viewMatrices[0].mapPointsWithStride(positions, sizeof(SkPoint), kVerticesPerTri);

    // setup colors so we can detect we are actually drawing instances(the last triangle will be
    // a different color)
    GrGLfloat colors[kVerticesPerTri * kNumTri];
    for (uint32_t i = 0; i < kNumTri; i++) {
        // set colors
        uint32_t offset = i * kVerticesPerTri;
        float color = i == kNumTri - 1 ? 1.0f : 0.0f;
        colors[offset++] = color; colors[offset++] = 0.0f; colors[offset++] = 0.0f;
    }

    GrGLuint posVBO;
    // setup position VBO
    GR_GL_CALL(gl, GenBuffers(1, &posVBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, posVBO));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(positions), positions, GR_GL_STATIC_DRAW));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, 2 * sizeof(GrGLfloat),
                                       (GrGLvoid*)0));

    // setup color VBO
    GrGLuint instanceVBO;
    GR_GL_CALL(gl, GenBuffers(1, &instanceVBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, instanceVBO));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(colors), colors, GR_GL_STATIC_DRAW));
    GR_GL_CALL(gl, EnableVertexAttribArray(1));
    GR_GL_CALL(gl, VertexAttribPointer(1, 3, GR_GL_FLOAT, GR_GL_FALSE, 3 * sizeof(GrGLfloat),
                                       (GrGLvoid*)0));
    GR_GL_CALL(gl, VertexAttribDivisor(1, 1));
    fBuffers.push_back(posVBO);
    fBuffers.push_back(instanceVBO);
}

void GLCpuPosInstancedArraysBench::setupDoubleVbo(const GrGLInterface* gl,
                                                  const SkMatrix* viewMatrices) {
    // Constants for our various shader programs
    SkPoint positions[kVerticesPerTri * kNumTri];
    GrGLfloat colors[kVerticesPerTri * kNumTri * 3];
    for (uint32_t i = 0; i < kNumTri; i++) {
        SkPoint* position = &positions[i * kVerticesPerTri];
        position[0].set(-1.0f, -1.0f);
        position[1].set( 1.0f, -1.0f);
        position[2].set( 1.0f,  1.0f);
        viewMatrices[i].mapPointsWithStride(position, sizeof(SkPoint), kVerticesPerTri);

        // set colors
        float color = i == kNumTri - 1 ? 1.0f : 0.0f;
        uint32_t offset = i * kVerticesPerTri * 3;
        for (uint32_t j = 0; j < kVerticesPerTri; j++) {
            colors[offset++] = color; colors[offset++] = 0.0f; colors[offset++] = 0.0f;
        }
    }

    GrGLuint posVBO, colorVBO;
    // setup position VBO
    GR_GL_CALL(gl, GenBuffers(1, &posVBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, posVBO));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, 2 * sizeof(GrGLfloat),
                                       (GrGLvoid*)0));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(positions), positions, GR_GL_STATIC_DRAW));

    // setup color VBO
    GR_GL_CALL(gl, GenBuffers(1, &colorVBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, colorVBO));
    GR_GL_CALL(gl, EnableVertexAttribArray(1));
    GR_GL_CALL(gl, VertexAttribPointer(1, 3, GR_GL_FLOAT, GR_GL_FALSE, 3 * sizeof(GrGLfloat),
                                       (GrGLvoid*)0));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(colors), colors, GR_GL_STATIC_DRAW));

    fBuffers.push_back(posVBO);
    fBuffers.push_back(colorVBO);
}

struct Vertex {
    SkPoint fPositions;
    GrGLfloat fColors[3];
};

void GLCpuPosInstancedArraysBench::setupSingleVbo(const GrGLInterface* gl,
                                                  const SkMatrix* viewMatrices) {
    // Constants for our various shader programs
    Vertex vertices[kVerticesPerTri * kNumTri];
    for (uint32_t i = 0; i < kNumTri; i++) {
        Vertex* v = &vertices[i * kVerticesPerTri];
        v[0].fPositions.set(-1.0f, -1.0f);
        v[1].fPositions.set( 1.0f, -1.0f);
        v[2].fPositions.set( 1.0f,  1.0f);

        SkPoint* position = reinterpret_cast<SkPoint*>(v);
        viewMatrices[i].mapPointsWithStride(position, sizeof(Vertex), kVerticesPerTri);

        // set colors
        float color = i == kNumTri - 1 ? 1.0f : 0.0f;
        for (uint32_t j = 0; j < kVerticesPerTri; j++) {
            uint32_t offset = 0;
            v->fColors[offset++] = color; v->fColors[offset++] = 0.0f; v->fColors[offset++] = 0.0f;
            v++;
        }
    }

    GrGLuint vbo;
    // setup VBO
    GR_GL_CALL(gl, GenBuffers(1, &vbo));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, vbo));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, EnableVertexAttribArray(1));
    GR_GL_CALL(gl, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, sizeof(Vertex),
                                       (GrGLvoid*)0));
    GR_GL_CALL(gl, VertexAttribPointer(1, 3, GR_GL_FLOAT, GR_GL_FALSE, sizeof(Vertex),
                                       (GrGLvoid*)(sizeof(SkPoint))));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(vertices), vertices, GR_GL_STATIC_DRAW));
    fBuffers.push_back(vbo);
}

void GLCpuPosInstancedArraysBench::setup(const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();
    fTexture = SetupFramebuffer(gl, kScreenWidth, kScreenHeight);

    fProgram = this->setupShader(ctx);

    // setup matrices
    int index = 0;
    SkMatrix viewMatrices[kNumTri];
    setup_matrices(kNumTri, [&index, &viewMatrices](const SkMatrix& m) {
        viewMatrices[index++] = m;
    });

    // setup VAO
    GR_GL_CALL(gl, GenVertexArrays(1, &fVAO));
    GR_GL_CALL(gl, BindVertexArray(fVAO));

    switch (fVboSetup) {
        case kUseOne_VboSetup:
            this->setupSingleVbo(gl, viewMatrices);
            break;
        case kUseTwo_VboSetup:
            this->setupDoubleVbo(gl, viewMatrices);
            break;
        case kUseInstance_VboSetup:
            this->setupInstanceVbo(gl, viewMatrices);
            break;
    }

    // clear screen
    GR_GL_CALL(gl, ClearColor(0.03f, 0.03f, 0.03f, 1.0f));
    GR_GL_CALL(gl, Clear(GR_GL_COLOR_BUFFER_BIT));

    // set us up to draw
    GR_GL_CALL(gl, UseProgram(fProgram));
    GR_GL_CALL(gl, BindVertexArray(fVAO));
}

void GLCpuPosInstancedArraysBench::glDraw(const int loops, const GrGLContext* ctx) {
    const GrGLInterface* gl = ctx->interface();

    uint32_t maxTrianglesPerFlush = fDrawDiv == 0 ?  kNumTri :
                                                     kDrawMultiplier / fDrawDiv;
    uint32_t trianglesToDraw = loops * kDrawMultiplier;

    if (kUseInstance_VboSetup == fVboSetup) {
        while (trianglesToDraw > 0) {
            uint32_t triangles = SkTMin(trianglesToDraw, maxTrianglesPerFlush);
            GR_GL_CALL(gl, DrawArraysInstanced(GR_GL_TRIANGLES, 0, kVerticesPerTri, triangles));
            trianglesToDraw -= triangles;
        }
    } else {
        while (trianglesToDraw > 0) {
            uint32_t triangles = SkTMin(trianglesToDraw, maxTrianglesPerFlush);
            GR_GL_CALL(gl, DrawArrays(GR_GL_TRIANGLES, 0, kVerticesPerTri * triangles));
            trianglesToDraw -= triangles;
        }
    }

#if 0
    //const char* filename = "/data/local/tmp/out.png";
    SkString filename("out");
    filename.appendf("_%s.png", this->getName());
    DumpImage(gl, kScreenWidth, kScreenHeight, filename.c_str());
#endif
}

void GLCpuPosInstancedArraysBench::teardown(const GrGLInterface* gl) {
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, 0));
    GR_GL_CALL(gl, BindVertexArray(0));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
    GR_GL_CALL(gl, DeleteTextures(1, &fTexture));
    GR_GL_CALL(gl, DeleteProgram(fProgram));
    GR_GL_CALL(gl, DeleteBuffers(fBuffers.count(), fBuffers.begin()));
    GR_GL_CALL(gl, DeleteVertexArrays(1, &fVAO));
    fBuffers.reset();
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseInstance_VboSetup, 0) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseOne_VboSetup, 0) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseTwo_VboSetup, 0) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseOne_VboSetup, 1) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseTwo_VboSetup, 1) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseOne_VboSetup, 2) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseTwo_VboSetup, 2) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseOne_VboSetup, 4) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseTwo_VboSetup, 4) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseOne_VboSetup, 8) )
DEF_BENCH( return new GLCpuPosInstancedArraysBench(GLCpuPosInstancedArraysBench::kUseTwo_VboSetup, 8) )

#endif
