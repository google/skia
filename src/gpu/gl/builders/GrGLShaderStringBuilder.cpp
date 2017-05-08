/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderStringBuilder.h"
#include "SkAutoMalloc.h"
#include "SkSLCompiler.h"
#include "SkSLGLSLCodeGenerator.h"
#include "SkTraceEvent.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "ir/SkSLProgram.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(gpu->glInterface(), R, X)

// Print the source code for all shaders generated.
static const bool c_PrintShaders{false};

static void print_source_with_line_numbers(const SkString&);

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char** strings,
                                    int* lengths,
                                    int count,
                                    GrGpu::Stats* stats,
                                    const SkSL::Program::Settings& settings,
                                    SkSL::Program::Inputs* outInputs) {
    const GrGLInterface* gli = glCtx.interface();

    GrGLuint shaderId;
    GR_GL_CALL_RET(gli, shaderId, CreateShader(type));
    if (0 == shaderId) {
        return 0;
    }

    SkString sksl;
#ifdef SK_DEBUG
    sksl = GrGLSLPrettyPrint::PrettyPrintGLSL(strings, lengths, count, false);
#else
    for (int i = 0; i < count; i++) {
        sksl.append(strings[i], lengths[i]);
    }
#endif

    SkSL::String glsl;
    if (type == GR_GL_VERTEX_SHADER || type == GR_GL_FRAGMENT_SHADER) {
        SkSL::Compiler& compiler = *glCtx.compiler();
        std::unique_ptr<SkSL::Program> program;
        program = compiler.convertProgram(
                                        type == GR_GL_VERTEX_SHADER ? SkSL::Program::kVertex_Kind
                                                                    : SkSL::Program::kFragment_Kind,
                                        sksl,
                                        settings);
        if (!program || !compiler.toGLSL(*program, &glsl)) {
            SkDebugf("SKSL compilation error\n----------------------\n");
            SkDebugf("SKSL:\n");
            print_source_with_line_numbers(sksl);
            SkDebugf("\nErrors:\n%s\n", compiler.errorText().c_str());
            SkDEBUGFAIL("SKSL compilation failed!\n");
        }
        *outInputs = program->fInputs;
    } else {
        // TODO: geometry shader support in sksl.
        SkASSERT(type == GR_GL_GEOMETRY_SHADER);
        glsl = sksl;
    }

    const char* glslChars = glsl.c_str();
    GrGLint glslLength = (GrGLint) glsl.size();
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
                print_source_with_line_numbers(sksl);
                SkDebugf("GLSL:\n");
                print_source_with_line_numbers(glsl);
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
        print_source_with_line_numbers(sksl);
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));

    return shaderId;
}

static void print_source_with_line_numbers(const SkString& source) {
    SkTArray<SkString> lines;
    SkStrSplit(source.c_str(), "\n", kStrict_SkStrSplitMode, &lines);
    for (int line = 0; line < lines.count(); ++line) {
        // Print the shader one line at the time so it doesn't get truncated by the adb log.
        SkDebugf("%4i\t%s\n", line + 1, lines[line].c_str());
    }
}
