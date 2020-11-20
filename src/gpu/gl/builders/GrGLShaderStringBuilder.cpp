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
                                            SkSL::String* glsl,
                                            GrContextOptions::ShaderErrorHandler* errorHandler) {
    SkSL::Compiler* compiler = context.compiler();
    std::unique_ptr<SkSL::Program> program;
#ifdef SK_DEBUG
    SkSL::String src = GrShaderUtils::PrettyPrint(sksl);
#else
    const SkSL::String& src = sksl;
#endif
    program = compiler->convertProgram(programKind, src, settings);
    if (!program || !compiler->toGLSL(*program, glsl)) {
        errorHandler->compileError(src.c_str(), compiler->errorText().c_str());
        return nullptr;
    }

    if (gPrintSKSL || gPrintGLSL) {
        print_shader_banner(programKind);
        if (gPrintSKSL) {
            SkDebugf("SKSL:\n");
            GrShaderUtils::PrintLineByLine(GrShaderUtils::PrettyPrint(sksl));
        }
        if (gPrintGLSL) {
            SkDebugf("GLSL:\n");
            GrShaderUtils::PrintLineByLine(GrShaderUtils::PrettyPrint(*glsl));
        }
    }

    return program;
}

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const SkSL::String& glsl,
                                    GrGpu::Stats* stats,
                                    GrContextOptions::ShaderErrorHandler* errorHandler) {
    const GrGLInterface* gli = glCtx.glInterface();

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

    bool checkCompiled = !glCtx.caps()->skipErrorChecks();

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
            }
            errorHandler->compileError(glsl.c_str(), infoLen > 0 ? (const char*)log.get() : "");
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
