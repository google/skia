/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLBench.h"

#if SK_SUPPORT_GPU
#include "GrGpu.h"
#include "GrTest.h"
#include "gl/GrGLContext.h"
#include "gl/builders/GrGLShaderStringBuilder.h"
#include "SkSLCompiler.h"
#include <stdio.h>

#include "sk_tool_utils.h"

const GrGLContext* GLBench::getGLContext(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    if (nullptr == canvas->getGrContext()) {
        return nullptr;
    }
    GrContext* context = canvas->getGrContext();
    GrGpu* gpu = context->getGpu();
    if (!gpu) {
        SkDebugf("Couldn't get Gr gpu.");
        return nullptr;
    }

    const GrGLContext* ctx = gpu->glContextForTesting();
    if (!ctx) {
        SkDebugf("Couldn't get an interface\n");
        return nullptr;
    }

    return this->onGetGLContext(ctx);
}

void GLBench::onPreDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->setup(ctx);
}

void GLBench::onPostDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->teardown(ctx->interface());
}

void GLBench::onDraw(int loops, SkCanvas* canvas) {
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->glDraw(loops, ctx);
    canvas->getGrContext()->resetContext();
}

GrGLuint GLBench::CompileShader(const GrGLContext* context, const char* sksl, GrGLenum type) {
    const GrGLInterface* gl = context->interface();
    SkSL::String glsl;
    SkSL::Program::Settings settings;
    settings.fCaps = context->caps()->shaderCaps();
    std::unique_ptr<SkSL::Program> program = context->compiler()->convertProgram(
                                        type == GR_GL_VERTEX_SHADER ? SkSL::Program::kVertex_Kind
                                                                    : SkSL::Program::kFragment_Kind,
                                        SkString(sksl),
                                        settings);
    if (!program || !context->compiler()->toGLSL(*program, &glsl)) {
        SkDebugf("SkSL compilation failed:\n%s\n%s\n", sksl,
                 context->compiler()->errorText().c_str());
    }
    GrGLuint shader;
    // Create the shader object
    GR_GL_CALL_RET(gl, shader, CreateShader(type));

    // Load the shader source
    const char* glslPtr = glsl.c_str();
    GR_GL_CALL(gl, ShaderSource(shader, 1, (const char**) &glslPtr, nullptr));

    // Compile the shader
    GR_GL_CALL(gl, CompileShader(shader));

    // Check for compile time errors
    GrGLint success = GR_GL_INIT_ZERO;
    GrGLchar infoLog[512];
    GR_GL_CALL(gl, GetShaderiv(shader, GR_GL_COMPILE_STATUS, &success));
    if (!success) {
        GR_GL_CALL(gl, GetShaderInfoLog(shader, 512, nullptr, infoLog));
        SkDebugf("ERROR::SHADER::COMPLIATION_FAILED: %s\n", infoLog);
    }

    return shader;
}

GrGLuint GLBench::CreateProgram(const GrGLContext* context, const char* vshader, 
                                const char* fshader) {
    const GrGLInterface* gl = context->interface();
    GrGLuint vertexShader = CompileShader(context, vshader, GR_GL_VERTEX_SHADER);
    GrGLuint fragmentShader = CompileShader(context, fshader, GR_GL_FRAGMENT_SHADER);

    GrGLuint shaderProgram;
    GR_GL_CALL_RET(gl, shaderProgram, CreateProgram());
    GR_GL_CALL(gl, AttachShader(shaderProgram, vertexShader));
    GR_GL_CALL(gl, AttachShader(shaderProgram, fragmentShader));
    GR_GL_CALL(gl, LinkProgram(shaderProgram));

    // Check for linking errors
    GrGLint success = GR_GL_INIT_ZERO;
    GrGLchar infoLog[512];
    GR_GL_CALL(gl, GetProgramiv(shaderProgram, GR_GL_LINK_STATUS, &success));
    if (!success) {
        GR_GL_CALL(gl, GetProgramInfoLog(shaderProgram, 512, nullptr, infoLog));
        SkDebugf("Linker Error: %s\n", infoLog);
    }
    GR_GL_CALL(gl, DeleteShader(vertexShader));
    GR_GL_CALL(gl, DeleteShader(fragmentShader));

    return shaderProgram;
}

GrGLuint GLBench::SetupFramebuffer(const GrGLInterface* gl, int screenWidth, int screenHeight) {
    //Setup framebuffer
    GrGLuint texture;
    GR_GL_CALL(gl, GenTextures(1, &texture));
    GR_GL_CALL(gl, ActiveTexture(GR_GL_TEXTURE7));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, texture));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(gl, TexImage2D(GR_GL_TEXTURE_2D,
                              0, //level
                              GR_GL_RGBA, //internal format
                              screenWidth, // width
                              screenHeight, // height
                              0, //border
                              GR_GL_RGBA, //format
                              GR_GL_UNSIGNED_BYTE, // type
                              nullptr));

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
    return texture;
}


void GLBench::DumpImage(const GrGLInterface* gl, uint32_t screenWidth, uint32_t screenHeight,
                        const char* filename) {
    // read back pixels
    SkAutoTArray<uint32_t> readback(screenWidth * screenHeight);
    GR_GL_CALL(gl, ReadPixels(0, // x
                              0, // y
                              screenWidth, // width
                              screenHeight, // height
                              GR_GL_RGBA, //format
                              GR_GL_UNSIGNED_BYTE, //type
                              readback.get()));

    // dump png
    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(screenWidth, screenHeight))) {
        SkDebugf("couldn't allocate bitmap\n");
        return;
    }

    bm.setPixels(readback.get());

    if (!sk_tool_utils::EncodeImageToFile(filename, bm, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return;
    }
}

#endif
