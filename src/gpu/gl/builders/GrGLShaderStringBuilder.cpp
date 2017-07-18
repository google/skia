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

static void print_source_lines_with_numbers(const char* source,
                                            std::function<void(const char*)> println) {
    SkTArray<SkString> lines;
    SkStrSplit(source, "\n", kStrict_SkStrSplitMode, &lines);
    for (int i = 0; i < lines.count(); ++i) {
        SkString& line = lines[i];
        line.prependf("%4i\t", i + 1);
        println(line.c_str());
    }
}

// Prints shaders one line at the time. This ensures they don't get truncated by the adb log.
static void print_shaders_line_by_line(const char** skslStrings, int* lengths,
                                     int count, const SkSL::String& glsl,
                                     std::function<void(const char*)> println = [](const char* ln) {
                                         SkDebugf("%s\n", ln);
                                     }) {
    SkString sksl = GrSKSLPrettyPrint::PrettyPrint(skslStrings, lengths, count, false);
    println("SKSL:");
    print_source_lines_with_numbers(sksl.c_str(), println);
    if (!glsl.isEmpty()) {
        println("GLSL:");
        print_source_lines_with_numbers(glsl.c_str(), println);
    }
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
    SkSL::Compiler* compiler = context.compiler();
    std::unique_ptr<SkSL::Program> program;
    SkSL::Program::Kind programKind;
    switch (type) {
        case GR_GL_VERTEX_SHADER:   programKind = SkSL::Program::kVertex_Kind;   break;
        case GR_GL_FRAGMENT_SHADER: programKind = SkSL::Program::kFragment_Kind; break;
        case GR_GL_GEOMETRY_SHADER: programKind = SkSL::Program::kGeometry_Kind; break;
    }
    program = compiler->convertProgram(programKind, sksl, settings);
    if (!program || !compiler->toGLSL(*program, glsl)) {
        SkDebugf("SKSL compilation error\n----------------------\n");
        print_shaders_line_by_line(skslStrings, lengths, count, *glsl);
        SkDebugf("\nErrors:\n%s\n", compiler->errorText().c_str());
        SkDEBUGFAIL("SKSL compilation failed!\n");
        return nullptr;
    }
    return program;
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

    // Trace event for shader preceding driver compilation
    bool traceShader;
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), &traceShader);
    if (traceShader) {
        SkString shaderDebugString;
        print_shaders_line_by_line(skslStrings, lengths, count, glsl, [&](const char* ln) {
            shaderDebugString.append(ln);
            shaderDebugString.append("\n");
        });
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
            SkDebugf("GLSL compilation error\n----------------------\n");
            print_shaders_line_by_line(skslStrings, lengths, count, glsl);
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
        print_shaders_line_by_line(skslStrings, lengths, count, glsl);
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
        print_shaders_line_by_line(skslStrings, lengths, count, glsl);
    }
}
