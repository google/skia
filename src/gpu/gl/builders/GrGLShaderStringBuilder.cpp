/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderStringBuilder.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

SK_CONF_DECLARE(bool, c_PrintShaders, "gpu.printShaders", false,
                "Print the source code for all shaders generated.");

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char** strings,
                                    int* lengths,
                                    int count,
                                    GrGpu::Stats* stats) {
    const GrGLInterface* gli = glCtx.interface();

    GrGLuint shaderId;
    GR_GL_CALL_RET(gli, shaderId, CreateShader(type));
    if (0 == shaderId) {
        return 0;
    }

#ifdef SK_DEBUG
    SkString prettySource = GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, false);
    const GrGLchar* sourceStr = prettySource.c_str();
    GrGLint sourceLength = static_cast<GrGLint>(prettySource.size());
    GR_GL_CALL(gli, ShaderSource(shaderId, 1, &sourceStr, &sourceLength));
#else
    GR_GL_CALL(gli, ShaderSource(shaderId, count, strings, lengths));
#endif

    // If tracing is enabled in chrome then we pretty print
    bool traceShader;
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), &traceShader);
    if (traceShader) {
        SkString shader = GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, false);
        TRACE_EVENT_INSTANT1(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "skia_gpu::GLShader",
                             TRACE_EVENT_SCOPE_THREAD, "shader", TRACE_STR_COPY(shader.c_str()));
    }

    stats->incShaderCompilations();
    GR_GL_CALL(gli, CompileShader(shaderId));

    // Calling GetShaderiv in Chromium is quite expensive. Assume success in release builds.
    bool checkCompiled = kChromium_GrGLDriver != glCtx.driver();
#ifdef SK_DEBUG
    checkCompiled = true;
#endif
    if (checkCompiled) {
        GrGLint compiled = GR_GL_INIT_ZERO;
        GR_GL_CALL(gli, GetShaderiv(shaderId, GR_GL_COMPILE_STATUS, &compiled));

        if (!compiled) {
            GrGLint infoLen = GR_GL_INIT_ZERO;
            GR_GL_CALL(gli, GetShaderiv(shaderId, GR_GL_INFO_LOG_LENGTH, &infoLen));
            SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
            if (infoLen > 0) {
                // retrieve length even though we don't need it to workaround bug in Chromium cmd
                // buffer param validation.
                GrGLsizei length = GR_GL_INIT_ZERO;
                GR_GL_CALL(gli, GetShaderInfoLog(shaderId, infoLen+1, &length, (char*)log.get()));
                SkDebugf("%s", GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, true).c_str());
                SkDebugf("\n%s", log.get());
            }
            SkDEBUGFAIL("Shader compilation failed!");
            GR_GL_CALL(gli, DeleteShader(shaderId));
            return 0;
        }
    }

    if (c_PrintShaders) {
        SkDebugf("%s", GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, true).c_str());
        SkDebugf("\n");
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));

    return shaderId;
}
