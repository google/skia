/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderStringBuilder.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "SkTraceEvent.h"
#include "SkSLCompiler.h"
#include "SkSLGLSLCodeGenerator.h"
#include "ir/SkSLProgram.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

// Print the source code for all shaders generated.
static const bool c_PrintShaders{false};

static void print_shader_source(const char** strings, int* lengths, int count);

SkSL::GLCaps GrGLSkSLCapsForContext(const GrGLContext& context) {
    GrGLStandard standard = context.standard();
    const GrGLCaps* caps = context.caps();
    const GrGLSLCaps* glslCaps = caps->glslCaps();
    SkSL::GLCaps result;
    switch (standard) {
        case kGL_GrGLStandard:
            result.fStandard = SkSL::GLCaps::kGL_Standard;
            break;
        case kGLES_GrGLStandard:
            result.fStandard = SkSL::GLCaps::kGLES_Standard;
            break;
        default:
            SkASSERT(false);
            result.fStandard = SkSL::GLCaps::kGL_Standard;
    }

    switch (glslCaps->generation()) {
        case k110_GrGLSLGeneration:
            if (kGLES_GrGLStandard == standard) {
                // ES2's shader language is based on GLSL 1.20 but is version 1.00 of the ES 
                // language
                result.fVersion = 100;
            } else {
                SkASSERT(kGL_GrGLStandard == standard);
                result.fVersion = 110;
            }
            break;
        case k130_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            result.fVersion = 130;
            break;
        case k140_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            result.fVersion = 140;
            break;
        case k150_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            result.fVersion = 150;
            break;
        case k330_GrGLSLGeneration:
            if (kGLES_GrGLStandard == standard) {
                result.fVersion = 300;
            } else {
                SkASSERT(kGL_GrGLStandard == standard);
                result.fVersion = 330;
            }
            break;
        case k400_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            result.fVersion = 400;
            break;
        case k310es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == standard);
            result.fVersion = 310;
            break;
        case k320es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == standard);
            result.fVersion = 320;
            break;
    }
    result.fIsCoreProfile = caps->isCoreProfile();
    result.fUsesPrecisionModifiers = glslCaps->usesPrecisionModifiers();
    result.fMustDeclareFragmentShaderOutput = glslCaps->mustDeclareFragmentShaderOutput();
    result.fShaderDerivativeSupport = glslCaps->shaderDerivativeSupport();
    if (result.fShaderDerivativeSupport && glslCaps->shaderDerivativeExtensionString()) {
        result.fShaderDerivativeExtensionString = glslCaps->shaderDerivativeExtensionString();
    }
    result.fCanUseMinAndAbsTogether = glslCaps->canUseMinAndAbsTogether();
    result.fMustForceNegatedAtanParamToFloat = glslCaps->mustForceNegatedAtanParamToFloat();
    return result;
}

static void dump_string(std::string s) {
    // on Android, SkDebugf only displays the first 1K characters of output, which results in
    // incomplete shader source code. Print each line individually to avoid this problem.
    size_t index = 0;
    for (;;) {
        size_t next = s.find("\n", index);
        if (next == std::string::npos) {
            SkDebugf("%s", s.substr(index).c_str());
            break;
        } else {
            SkDebugf("%s", s.substr(index, next - index + 1).c_str());
            index = next + 1;
        }
    }
}

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

    std::string sksl;
#ifdef SK_DEBUG
    SkString prettySource = GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, false);
    sksl = std::string(prettySource.c_str());
#else
    for (int i = 0; i < count; i++) {
        sksl.append(strings[i], lengths[i]);
    }
#endif

    std::string glsl;
    SkSL::Compiler& compiler = *glCtx.compiler();
    SkSL::GLCaps caps = GrGLSkSLCapsForContext(glCtx);
    SkASSERT(type == GR_GL_VERTEX_SHADER || type == GR_GL_FRAGMENT_SHADER);
    SkDEBUGCODE(bool result = )compiler.toGLSL(type == GR_GL_VERTEX_SHADER 
                                                                    ? SkSL::Program::kVertex_Kind
                                                                    : SkSL::Program::kFragment_Kind,
                                               std::string(sksl.c_str()),
                                               caps,
                                               &glsl);
#ifdef SK_DEBUG
    if (!result) {
        SkDebugf("SKSL compilation error\n----------------------\n");
        SkDebugf("SKSL:\n");
        dump_string(sksl);
        SkDebugf("\nErrors:\n%s\n", compiler.errorText().c_str());
        SkDEBUGFAIL("SKSL compilation failed!\n");
    }
#endif

    const char* glslChars = glsl.c_str();
    GrGLint glslLength = (GrGLint) glsl.length();
    GR_GL_CALL(gli, ShaderSource(shaderId, 1, &glslChars, &glslLength));

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
                SkDebugf("GLSL compilation error\n----------------------\n");
                SkDebugf("SKSL:\n");
                dump_string(sksl);
                SkDebugf("GLSL:\n");
                dump_string(glsl);
                SkDebugf("Errors:\n%s\n", (const char*) log.get());
            }
            SkDEBUGFAIL("GLSL compilation failed!");
            GR_GL_CALL(gli, DeleteShader(shaderId));
            return 0;
        }
    }

    if (c_PrintShaders) {
        print_shader_source(strings, lengths, count);
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));

    return shaderId;
}

static void print_shader_source(const char** strings, int* lengths, int count) {
    const SkString& pretty = GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, true);
    SkTArray<SkString> lines;
    SkStrSplit(pretty.c_str(), "\n", &lines);
    for (const SkString& line : lines) {
        // Print the shader one line at the time so it doesn't get truncated by the adb log.
        SkDebugf("%s\n", line.c_str());
    }
}
