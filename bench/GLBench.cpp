/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLBench.h"

#if SK_SUPPORT_GPU
#include "GrTest.h"
#include <stdio.h>

const GrGLContext* GLBench::getGLContext(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    if (NULL == canvas->getGrContext()) {
        return NULL;
    }
    GrContext* context = canvas->getGrContext();
    GrGpu* gpu = context->getGpu();
    if (!gpu) {
        SkDebugf("Couldn't get Gr gpu.");
        return NULL;
    }

    const GrGLContext* ctx = gpu->glContextForTesting();
    if (!ctx) {
        SkDebugf("Couldn't get an interface\n");
        return NULL;
    }

    return this->onGetGLContext(ctx);
}

void GLBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->setup(ctx);
}

void GLBench::onPerCanvasPostDraw(SkCanvas* canvas) {
    // This bench exclusively tests GL calls directly
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->teardown(ctx->interface());
}

void GLBench::onDraw(const int loops, SkCanvas* canvas) {
    const GrGLContext* ctx = this->getGLContext(canvas);
    if (!ctx) {
        return;
    }
    this->glDraw(loops, ctx);
}

GrGLuint GLBench::CompileShader(const GrGLInterface* gl, const char* shaderSrc, GrGLenum type) {
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
    if (!success) {
        GR_GL_CALL(gl, GetShaderInfoLog(shader, 512, NULL, infoLog));
        SkDebugf("ERROR::SHADER::COMPLIATION_FAILED: %s\n", infoLog);
    }

    return shader;
}

GrGLuint GLBench::CreateProgram(const GrGLInterface* gl, const char* vshader, const char* fshader) {

    GrGLuint vertexShader = CompileShader(gl, vshader, GR_GL_VERTEX_SHADER);
    GrGLuint fragmentShader = CompileShader(gl, fshader, GR_GL_FRAGMENT_SHADER);

    GrGLuint shaderProgram;
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

GrGLuint GLBench::SetupFramebuffer(const GrGLInterface* gl, int screenWidth, int screenHeight) {
    //Setup framebuffer
    GrGLuint texture;
    GR_GL_CALL(gl, GenTextures(1, &texture));
    GR_GL_CALL(gl, ActiveTexture(GR_GL_TEXTURE15));
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

    if (!SkImageEncoder::EncodeFile(filename, bm, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return;
    }
}

#endif
