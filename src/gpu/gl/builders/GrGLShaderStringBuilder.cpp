/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkAutoMalloc.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/builders/GrGLShaderStringBuilder.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

// Print the source code for all shaders generated.
static const bool gPrintSKSL = false;
static const bool gPrintGLSL = false;

void print_shader_banner(SkSL::Program::Kind programKind) {
    const char* typeName = "Unknown";
    switch (programKind) {
        case SkSL::Program::kVertex_Kind:   typeName = "Vertex";   break;
        case SkSL::Program::kGeometry_Kind: typeName = "Geometry"; break;
        case SkSL::Program::kFragment_Kind: typeName = "Fragment"; break;
        default: break;
    }
    SkDebugf("---- %s shader ----------------------------------------------------\n", typeName);
}

std::unique_ptr<SkSL::Program> GrSkSLtoGLSL(const GrGLContext& context,
                                            SkSL::Program::Kind programKind,
                                            const SkSL::String& sksl,
                                            const SkSL::Program::Settings& settings,
                                            SkSL::String* glsl) {
    SkSL::Compiler* compiler = context.compiler();
    std::unique_ptr<SkSL::Program> program;
    program = compiler->convertProgram(programKind, sksl, settings);
    if (!program || !compiler->toGLSL(*program, glsl)) {
        SkDebugf("SKSL compilation error\n----------------------\n");
        GrShaderUtils::PrintLineByLine("SKSL:", sksl);
        SkDebugf("\nErrors:\n%s\n", compiler->errorText().c_str());
        SkDEBUGFAIL("SKSL compilation failed!\n");
        return nullptr;
    }

    if (gPrintSKSL || gPrintGLSL) {
        print_shader_banner(programKind);
        if (gPrintSKSL) {
            GrShaderUtils::PrintLineByLine("SKSL:", sksl);
        }
        if (gPrintGLSL) {
            GrShaderUtils::PrintLineByLine("GLSL:", *glsl);
        }
    }

    return program;
}

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const SkSL::String& glsl,
                                    GrGpu::Stats* stats,
                                    bool assertOnFailure) {
    const GrGLInterface* gli = glCtx.interface();

    // Specify GLSL source to the driver.
    GrGLuint shaderId;
    GR_GL_CALL_RET(gli, shaderId, CreateShader(type));
    if (0 == shaderId) {
        return 0;
    }
    const GrGLchar* source = glsl.c_str();
    GrGLint sourceLength = glsl.size();
    GR_GL_CALL(gli, ShaderSource(shaderId, 1, &source, &sourceLength));

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
            GrShaderUtils::PrintLineByLine("GLSL:", glsl);
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
            // In Chrome we may have failed due to context-loss. So we should just continue along
            // wihthout asserting until the GrContext gets abandoned.
            if (assertOnFailure && kChromium_GrGLDriver != glCtx.driver()) {
                SkDEBUGFAIL("GLSL compilation failed!");
            }
            GR_GL_CALL(gli, DeleteShader(shaderId));
            return 0;
        }
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));
    return shaderId;
}

void GrGLPrintShader(const GrGLContext& context, SkSL::Program::Kind programKind,
                     const SkSL::String& sksl, const SkSL::Program::Settings& settings) {
    print_shader_banner(programKind);
    GrShaderUtils::PrintLineByLine("SKSL:", sksl);
    SkSL::String glsl;
    if (GrSkSLtoGLSL(context, programKind, sksl, settings, &glsl)) {
        GrShaderUtils::PrintLineByLine("GLSL:", glsl);
    }
}
