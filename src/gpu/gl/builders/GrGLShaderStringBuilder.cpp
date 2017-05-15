/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderStringBuilder.h"
#include "GrSKSLPrettyPrint.h"
#include "SkAutoMalloc.h"
#include "SkSLCompiler.h"
#include "SkSLGLSLCodeGenerator.h"
#include "SkTraceEvent.h"
#include "gl/GrGLGpu.h"
#include "ir/SkSLProgram.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

// Print the source code for all shaders generated.
static const bool c_PrintShaders{false};

static SkString list_source_with_line_numbers(const char* source) {
    SkTArray<SkString> lines;
    SkStrSplit(source, "\n", kStrict_SkStrSplitMode, &lines);
    SkString result;
    for (int line = 0; line < lines.count(); ++line) {
        // Print the shader one line at the time so it doesn't get truncated by the adb log.
        result.appendf("%4i\t%s\n", line + 1, lines[line].c_str());
    }
    return result;
}

SkString list_shaders(const char** skslStrings, int* lengths, int count, const SkSL::String& glsl) {
    SkString sksl = GrSKSLPrettyPrint::PrettyPrint(skslStrings, lengths, count, false);
    SkString result("SKSL:\n");
    result.append(list_source_with_line_numbers(sksl.c_str()));
    if (!glsl.isEmpty()) {
        result.append("GLSL:\n");
        result.append(list_source_with_line_numbers(glsl.c_str()));
    }
    return result;
}

std::unique_ptr<SkSL::Program> translate_to_glsl(const GrGLContext& context, GrGLenum type,
                                                 const char** skslStrings, int* lengths, int count,
                                                 const SkSL::Program::Settings& settings,
                                                 SkSL::String* glsl) {
    SkString sksl;
#ifdef SK_DEBUG
    sksl = GrSKSLPrettyPrint::PrettyPrint(skslStrings, lengths, count, false);
#else
    for (int i = 0; i < count; i++) {
        sksl.append(skslStrings[i], lengths[i]);
    }
#endif
    if (type == GR_GL_VERTEX_SHADER || type == GR_GL_FRAGMENT_SHADER) {
        SkSL::Compiler* compiler = context.compiler();
        std::unique_ptr<SkSL::Program> program;
        program = compiler->convertProgram(type == GR_GL_VERTEX_SHADER
                                                   ? SkSL::Program::kVertex_Kind
                                                   : SkSL::Program::kFragment_Kind,
                                           sksl,
                                           settings);
        if (!program || !compiler->toGLSL(*program, glsl)) {
            SkDebugf("SKSL compilation error\n----------------------\n");
            SkDebugf(list_shaders(skslStrings, lengths, count, *glsl).c_str());
            SkDebugf("\nErrors:\n%s\n", compiler->errorText().c_str());
            SkDEBUGFAIL("SKSL compilation failed!\n");
            return nullptr;
        }
        return program;
    } else {
        // TODO: geometry shader support in sksl.
        SkASSERT(type == GR_GL_GEOMETRY_SHADER);
        return nullptr;
    }
}

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char** skslStrings,
                                    int* lengths,
                                    int count,
                                    GrGpu::Stats* stats,
                                    const SkSL::Program::Settings& settings,
                                    SkSL::Program::Inputs* outInputs) {
    const GrGLInterface* gli = glCtx.interface();

    SkSL::String glsl;
    auto program = translate_to_glsl(glCtx, type, skslStrings, lengths, count, settings, &glsl);
    if (!program) {
        return 0;
    }

    // Specify GLSL source to the driver.
    GrGLuint shaderId;
    GR_GL_CALL_RET(gli, shaderId, CreateShader(type));
    if (0 == shaderId) {
        return 0;
    }
    const char* glslChars = glsl.c_str();
    GrGLint glslLength = (GrGLint) glsl.size();
    GR_GL_CALL(gli, ShaderSource(shaderId, 1, &glslChars, &glslLength));

    // Lazy initialized pretty-printed shaders for dumping.
    SkString shaderDebugString;

    // Trace event for shader preceding driver compilation
    bool traceShader;
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), &traceShader);
    if (traceShader) {
        if (shaderDebugString.isEmpty()) {
            shaderDebugString = list_shaders(skslStrings, lengths, count, glsl);
        }
        TRACE_EVENT_INSTANT1(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "skia_gpu::GLShader",
                             TRACE_EVENT_SCOPE_THREAD, "shader",
                             TRACE_STR_COPY(shaderDebugString.c_str()));
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
            if (shaderDebugString.isEmpty()) {
                shaderDebugString = list_shaders(skslStrings, lengths, count, glsl);
            }
            SkDebugf("GLSL compilation error\n----------------------\n");
            SkDebugf(shaderDebugString.c_str());
            GrGLint infoLen = GR_GL_INIT_ZERO;
            GR_GL_CALL(gli, GetShaderiv(shaderId, GR_GL_INFO_LOG_LENGTH, &infoLen));
            SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
            if (infoLen > 0) {
                // retrieve length even though we don't need it to workaround bug in Chromium cmd
                // buffer param validation.
                GrGLsizei length = GR_GL_INIT_ZERO;
                GR_GL_CALL(gli, GetShaderInfoLog(shaderId, infoLen+1, &length, (char*)log.get()));
                SkDebugf("Errors:\n%s\n", (const char*) log.get());
            }
            SkDEBUGFAIL("GLSL compilation failed!");
            GR_GL_CALL(gli, DeleteShader(shaderId));
            return 0;
        }
    }

    if (c_PrintShaders) {
        const char* typeName = "Unknown";
        switch (type) {
            case GR_GL_VERTEX_SHADER: typeName = "Vertex"; break;
            case GR_GL_GEOMETRY_SHADER: typeName = "Geometry"; break;
            case GR_GL_FRAGMENT_SHADER: typeName = "Fragment"; break;
        }
        SkDebugf("---- %s shader ----------------------------------------------------\n", typeName);
        if (shaderDebugString.isEmpty()) {
            shaderDebugString = list_shaders(skslStrings, lengths, count, glsl);
        }
        SkDebugf(shaderDebugString.c_str());
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));
    *outInputs = program->fInputs;
    return shaderId;
}

void GrGLPrintShader(const GrGLContext& context, GrGLenum type, const char** skslStrings,
                     int* lengths, int count, const SkSL::Program::Settings& settings) {
    SkSL::String glsl;
    if (translate_to_glsl(context, type, skslStrings, lengths, count, settings, &glsl)) {
        SkDebugf(list_shaders(skslStrings, lengths, count, glsl).c_str());
    }
}
