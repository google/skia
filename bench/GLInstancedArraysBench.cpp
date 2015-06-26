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
#include "GrTest.h"
#include "gl/GrGLGLSL.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLShaderVar.h"
#include "gl/GrGLUtil.h"
#include "glsl/GrGLSLCaps.h"
#include <stdio.h>

/*
 * This is a native GL benchmark for instanced arrays vs vertex buffer objects.  To benchmark this
 * functionality, we draw n * kDrawMultipier triangles per run.  If this number is less than
 * kNumTri then we do a single draw, either with instances, or drawArrays.  Otherwise we do
 * multiple draws.
 *
 * Additionally, there is a divisor, which if > 0 will act as a multiplier for the number of draws
 * issued.
 */
class GLInstancedArraysBench : public Benchmark {
protected:
    void onPerCanvasPreDraw(SkCanvas* canvas) override;
    virtual void setup(const GrGLContext*)=0;
    void onPerCanvasPostDraw(SkCanvas* canvas) override;
    virtual void teardown(const GrGLInterface*)=0;

    static const GrGLuint kScreenWidth = 800;
    static const GrGLuint kScreenHeight = 600;
    static const uint32_t kNumTri = 10000;
    static const uint32_t kVerticesPerTri = 3;
    static const uint32_t kDrawMultiplier = 512;

private:
    GrGLuint fTexture;
    typedef Benchmark INHERITED;
};

#if 0
class GLGpuPosInstancedArraysBench : public GLInstancedArraysBench {
protected:
    const char* onGetName() override {
        return "GLInstancedArraysBench_gpupos";
    }

    void setup(const GrGLContext*) override;
    void onDraw(const int loops, SkCanvas* canvas) override;
};
#endif

class GLCpuPosInstancedArraysBench : public GLInstancedArraysBench {
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
        , fDrawDiv(drawDiv) {
        fName = VboSetupToStr(vboSetup, fDrawDiv);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void setup(const GrGLContext*) override;
    void onDraw(const int loops, SkCanvas* canvas) override;
    void teardown(const GrGLInterface*) override;

private:
    void setupInstanceVbo(const GrGLInterface*, const SkMatrix*);
    void setupDoubleVbo(const GrGLInterface*, const SkMatrix*);
    void setupSingleVbo(const GrGLInterface*, const SkMatrix*);

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

    SkString fName;
    VboSetup fVboSetup;
    uint32_t fDrawDiv;
    SkTArray<GrGLuint> fBuffers;
    GrGLuint fProgram;
    GrGLuint fVAO;
};

static const GrGLContext* get_gl_context(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    if (NULL == canvas->getGrContext()) {
        return NULL;
    }
    GrContext* context = canvas->getGrContext();

    GrTestTarget tt;
    context->getTestTarget(&tt);
    if (!tt.target()) {
        SkDebugf("Couldn't get Gr test target.");
        return NULL;
    }

    const GrGLContext* ctx = tt.glContext();
    if (!ctx) {
        SkDebugf("Couldn't get an interface\n");
        return NULL;
    }

    // We only care about gpus with drawArraysInstanced support
    if (!ctx->interface()->fFunctions.fDrawArraysInstanced) {
        return NULL;
    }
    return ctx;
}

void GLInstancedArraysBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = get_gl_context(canvas);
    if (!ctx) {
        return;
    }
    this->setup(ctx);
}

void GLInstancedArraysBench::onPerCanvasPostDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = get_gl_context(canvas);
    if (!ctx) {
        return;
    }

    const GrGLInterface* gl = ctx->interface();

    // teardown
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, 0));
    GR_GL_CALL(gl, BindVertexArray(0));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
    GR_GL_CALL(gl, DeleteTextures(1, &fTexture));

    this->teardown(gl);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// TODO move all of the gpu positioning stuff to a new file
#ifdef GPU_POS
static const char* gpu_vertex_shader =
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec3 color;\n"
        "layout (location = 2) in mat3 offset;\n"

        "out vec3 fColor;\n"

        "void main()\n"
        "{\n"
            "gl_Position = vec4(offset * vec3(position, 1.0f), 1.f);\n"
            "fColor = color;\n"
        "}\n";
#endif

static GrGLuint load_shader(const GrGLInterface* gl, const char* shaderSrc, GrGLenum type) {
    GrGLuint shader;
    // Create the shader object
    GR_GL_CALL_RET(gl, shader, CreateShader(type));

    // Load the shader source
    GR_GL_CALL(gl, ShaderSource(shader, 1, &shaderSrc, NULL));

    // Compile the shader
    GR_GL_CALL(gl, CompileShader(shader));

    // Check for compile time errors
    GrGLint success;
    GrGLchar infoLog[512];
    GR_GL_CALL(gl, GetShaderiv(shader, GR_GL_COMPILE_STATUS, &success));
    if (!success)
    {
     GR_GL_CALL(gl, GetShaderInfoLog(shader, 512, NULL, infoLog));
     SkDebugf("ERROR::SHADER::COMPLIATION_FAILED: %s\n", infoLog);
    }

    return shader;
}

static GrGLuint compile_shader(const GrGLContext* ctx) {
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
    GrGLuint vertexShader = load_shader(gl, vshaderTxt.c_str(), GR_GL_VERTEX_SHADER);

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

    GrGLuint fragmentShader = load_shader(gl, fshaderTxt.c_str(), GR_GL_FRAGMENT_SHADER);

    GrGLint shaderProgram;
    GR_GL_CALL_RET(gl, shaderProgram, CreateProgram());
    GR_GL_CALL(gl, AttachShader(shaderProgram, vertexShader));
    GR_GL_CALL(gl, AttachShader(shaderProgram, fragmentShader));
    GR_GL_CALL(gl, LinkProgram(shaderProgram));

    // Check for linking errors
    GrGLint success;
    GrGLchar infoLog[512];
    GR_GL_CALL(gl, GetProgramiv(shaderProgram, GR_GL_LINK_STATUS, &success));
    if (!success) {
        GR_GL_CALL(gl, GetProgramInfoLog(shaderProgram, 512, NULL, infoLog));
        SkDebugf("Linker Error: %s\n", infoLog);
    }
    GR_GL_CALL(gl, DeleteShader(vertexShader));
    GR_GL_CALL(gl, DeleteShader(fragmentShader));

    return shaderProgram;
}

//#define DUMP_IMAGES
#ifdef DUMP_IMAGES
static void dump_image(const GrGLInterface* gl, uint32_t screenWidth, uint32_t screenHeight,
                       const char* filename) {
    // read back pixels
    uint32_t readback[screenWidth * screenHeight];
    GR_GL_CALL(gl, ReadPixels(0, // x
                              0, // y
                              screenWidth, // width
                              screenHeight, // height
                              GR_GL_RGBA, //format
                              GR_GL_UNSIGNED_BYTE, //type
                              readback));

    // dump png
    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(screenWidth, screenHeight))) {
        SkDebugf("couldn't allocate bitmap\n");
        return;
    }

    bm.setPixels(readback);

    if (!SkImageEncoder::EncodeFile(filename, bm, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return;
    }
}
#endif

static void setup_framebuffer(const GrGLInterface* gl, int screenWidth, int screenHeight) {
    //Setup framebuffer
    GrGLuint texture;
    GR_GL_CALL(gl, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    GR_GL_CALL(gl, PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    GR_GL_CALL(gl, GenTextures(1, &texture));
    GR_GL_CALL(gl, ActiveTexture(GR_GL_TEXTURE15));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, texture));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(gl, TexImage2D(GR_GL_TEXTURE_2D,
                              0, //level
                              GR_GL_RGBA8, //internal format
                              screenWidth, // width
                              screenHeight, // height
                              0, //border
                              GR_GL_RGBA, //format
                              GR_GL_UNSIGNED_BYTE, // type
                              NULL));

    // bind framebuffer
    GrGLuint framebuffer;
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, GenFramebuffers(1, &framebuffer));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, framebuffer));
    GR_GL_CALL(gl, FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_TEXTURE_2D,
                                        texture, 0));
    GR_GL_CALL(gl, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    GR_GL_CALL(gl, Viewport(0, 0, screenWidth, screenHeight));
}

template<typename Func>
static void setup_matrices(int numQuads, Func f) {
#if 0
    float max = sqrt(numQuads);
    float pos = 1.f / (2 * max);
    GrGLfloat offset = pos * 2;
    for(GrGLint row = 0; row < max; row++) {
        for(GrGLint col = 0; col < max; col++) {
            SkScalar xOffset = col / max * 2.f - 1.f + offset;
            SkScalar yOffset = row / max * 2.f - 1.f + offset;
            SkMatrix translation;
            SkRandom random;
            translation.setScale(pos, pos);
            translation.postTranslate(xOffset, yOffset);
            f(translation);
        }
    }
#endif
    // We draw a really small triangle so we are not fill rate limited
    for (int i = 0 ; i < numQuads; i++) {
        SkMatrix m = SkMatrix::I();
        m.setScale(0.0001f, 0.0001f);
        f(m);
    }
}

#ifdef GPU_POS
void GLGpuPosInstancedArraysBench::setup(const GrGLInterface* gl) {
    setup_framebuffer(gl, kScreenWidth, kScreenHeight);

    // compile and use shaders
    GrGLint shaderProgram = compile_shader(gl, gpu_vertex_shader, fragment_shader);

    // translations
    int index = 0;
    GrGLfloat viewMatrices[fNumQuads * fSkMatrixNumCells];
    setup_matrices(fNumQuads, [&index, &viewMatrices](const SkMatrix& m) {
        GrGLGetMatrix<3>(&viewMatrices[index], m);
        index += fSkMatrixNumCells;
    });

    // Constants for our various shader programs
    GrGLfloat quad_vertices[] = {
            // Positions // Colors
            -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,

            -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
             1.0f,  1.0f,  0.0f, 1.0f, 1.0f
    };

    // update vertex data
    GrGLuint quadVAO, quadVBO;
    GR_GL_CALL(gl, GenVertexArrays(1, &quadVAO));
    GR_GL_CALL(gl, GenBuffers(1, &quadVBO));
    GR_GL_CALL(gl, BindVertexArray(quadVAO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, quadVBO));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, 5 * sizeof(GrGLfloat), (GrGLvoid*)0));
    GR_GL_CALL(gl, EnableVertexAttribArray(1));
    GR_GL_CALL(gl, VertexAttribPointer(1, 3, GR_GL_FLOAT, GR_GL_FALSE, 5 * sizeof(GrGLfloat), (GrGLvoid*)(2 * sizeof(GrGLfloat))));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GR_GL_STATIC_DRAW));

    // Also set instance data
    GrGLuint instanceVBO;
    GR_GL_CALL(gl, GenBuffers(1, &instanceVBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, instanceVBO));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER, sizeof(GrGLfloat) * fSkMatrixNumCells * fNumQuads,
                              &viewMatrices[0], GR_GL_STATIC_DRAW));
    GR_GL_CALL(gl, EnableVertexAttribArray(2));
    GR_GL_CALL(gl, EnableVertexAttribArray(3));
    GR_GL_CALL(gl, EnableVertexAttribArray(4));
    GR_GL_CALL(gl, VertexAttribPointer(2, 3, GR_GL_FLOAT, GR_GL_FALSE, 9 * sizeof(GrGLfloat), (GrGLvoid*)0));
    GR_GL_CALL(gl, VertexAttribPointer(3, 3, GR_GL_FLOAT, GR_GL_FALSE, 9 * sizeof(GrGLfloat), (GrGLvoid*)(3 * sizeof(GrGLfloat))));
    GR_GL_CALL(gl, VertexAttribPointer(4, 3, GR_GL_FLOAT, GR_GL_FALSE, 9 * sizeof(GrGLfloat), (GrGLvoid*)(6 * sizeof(GrGLfloat))));
    GR_GL_CALL(gl, VertexAttribDivisor(2, 1));
    GR_GL_CALL(gl, VertexAttribDivisor(3, 1));
    GR_GL_CALL(gl, VertexAttribDivisor(4, 1));

    // draw
    GR_GL_CALL(gl, ClearColor(0.03f, 0.03f, 0.03f, 1.0f));
    GR_GL_CALL(gl, Clear(GR_GL_COLOR_BUFFER_BIT));

    // set us up to draw
    GR_GL_CALL(gl, UseProgram(shaderProgram));
    GR_GL_CALL(gl, BindVertexArray(quadVAO));
}

void GLGpuPosInstancedArraysBench::onDraw(const int loops, SkCanvas* canvas) {
    const GrGLInterface* gl = get_interface(canvas);
    if (!gl) {
        return;
    }

    GR_GL_CALL(gl, DrawArraysInstanced(GR_GL_TRIANGLES, 0, 6, fNumQuads));

#ifdef DUMP_IMAGES
    const char* filename = "out.png";
    dump_image(gl, kScreenWidth, kScreenHeight, filename);
#endif
    SkFAIL("done\n");
}

static uint32_t setup_quad_index_buffer(const GrGLInterface* gl) {
    static const int kMaxQuads = 1;//1 << 12; // max possible: (1 << 14) - 1;
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 0, 2, 3 };
    static const int kPatternSize = 6;
    static const int kVertCount = 4;
    static const int kIndicesCount = kPatternSize * kMaxQuads;
    int size = kPatternSize * kMaxQuads * sizeof(uint16_t);

    uint16_t* data = SkNEW_ARRAY(uint16_t, kMaxQuads * kPatternSize);

    for (int i = 0; i < kMaxQuads; ++i) {
        int baseIdx = i * kPatternSize;
        uint16_t baseVert = (uint16_t)(i * kVertCount);
        for (int j = 0; j < kPatternSize; ++j) {
            data[baseIdx+j] = baseVert + kPattern[j];
        }
    }

    GrGLuint quadIBO;
    GR_GL_CALL(gl, GenBuffers(1, &quadIBO));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, quadIBO));
    GR_GL_CALL(gl, BufferData(GR_GL_ELEMENT_ARRAY_BUFFER, size, data, GR_GL_STATIC_DRAW));

    SkDELETE_ARRAY(data);
    return kIndicesCount;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

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
    setup_framebuffer(gl, kScreenWidth, kScreenHeight);

    fProgram = compile_shader(ctx);

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

void GLCpuPosInstancedArraysBench::onDraw(const int loops, SkCanvas* canvas) {
    const GrGLContext* ctx = get_gl_context(canvas);
    if (!ctx) {
        return;
    }

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

#ifdef DUMP_IMAGES
    //const char* filename = "/data/local/tmp/out.png";
    SkString filename("out");
    filename.appendf("_%s.png", this->getName());
    dump_image(gl, kScreenWidth, kScreenHeight, filename.c_str());
#endif
}

void GLCpuPosInstancedArraysBench::teardown(const GrGLInterface* gl) {
    GR_GL_CALL(gl, DeleteProgram(fProgram));
    GR_GL_CALL(gl, DeleteBuffers(fBuffers.count(), fBuffers.begin()));
    GR_GL_CALL(gl, DeleteVertexArrays(1, &fVAO));
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
