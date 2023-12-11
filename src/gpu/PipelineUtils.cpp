/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/PipelineUtils.h"

namespace skgpu {

static bool sksl_to_backend(SkSL::Compiler* compiler,
                            bool (SkSL::Compiler::*toBackend)(SkSL::Program&, std::string*),
                            const char* backendLabel,
                            const std::string& sksl,
                            SkSL::ProgramKind programKind,
                            const SkSL::ProgramSettings& settings,
                            std::string* output,
                            SkSL::Program::Interface* outInterface,
                            ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<SkSL::Program> program = compiler->convertProgram(programKind,
                                                                      src,
                                                                      settings);
    if (!program || !(compiler->*toBackend)(*program, output)) {
        errorHandler->compileError(
                src.c_str(), compiler->errorText().c_str(), /*shaderWasCached=*/false);
        return false;
    }

#if defined(SK_PRINT_SKSL_SHADERS)
    const bool kPrintSkSL = true;
#else
    const bool kPrintSkSL = false;
#endif
#if defined(SK_PRINT_NATIVE_SHADERS)
    const bool printBackendSL = (backendLabel != nullptr);
#else
    const bool printBackendSL = false;
#endif

    if (kPrintSkSL || printBackendSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        if (kPrintSkSL) {
            SkDebugf("SkSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
        }
        if (printBackendSL) {
            SkDebugf("%s:\n", backendLabel);
            SkShaderUtils::PrintLineByLine(*output);
        }
    }

    if (outInterface) {
        *outInterface = program->fInterface;
    }
    return true;
}

bool SkSLToGLSL(SkSL::Compiler* compiler,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* glsl,
                SkSL::Program::Interface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(compiler, &SkSL::Compiler::toGLSL, "GLSL",
                           sksl, programKind, settings, glsl, outInterface, errorHandler);
}

bool SkSLToSPIRV(SkSL::Compiler* compiler,
                 const std::string& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::Program::Interface* outInterface,
                 ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(compiler, &SkSL::Compiler::toSPIRV, /*backendLabel=*/nullptr,
                           sksl, programKind, settings, spirv, outInterface, errorHandler);
}

bool SkSLToWGSL(SkSL::Compiler* compiler,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* wgsl,
                SkSL::Program::Interface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(compiler, &SkSL::Compiler::toWGSL, "WGSL",
                           sksl, programKind, settings, wgsl, outInterface, errorHandler);
}

bool SkSLToMSL(SkSL::Compiler* compiler,
               const std::string& sksl,
               SkSL::ProgramKind programKind,
               const SkSL::ProgramSettings& settings,
               std::string* msl,
               SkSL::Program::Interface* outInterface,
               ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(compiler, &SkSL::Compiler::toMetal, "MSL",
                           sksl, programKind, settings, msl, outInterface, errorHandler);
}

bool SkSLToHLSL(SkSL::Compiler* compiler,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* hlsl,
                SkSL::Program::Interface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(compiler, &SkSL::Compiler::toHLSL, "HLSL",
                           sksl, programKind, settings, hlsl, outInterface, errorHandler);
}

} // namespace skgpu
