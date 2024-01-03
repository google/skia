/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/PipelineUtils.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLHLSLCodeGenerator.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/utils/SkShaderUtils.h"

#include <string>

namespace skgpu {

static bool sksl_to_backend(const SkSL::ShaderCaps* caps,
                            bool (*toBackend)(SkSL::Program&,
                                              const SkSL::ShaderCaps*,
                                              std::string*),
                            const char* backendLabel,
                            const std::string& sksl,
                            SkSL::ProgramKind programKind,
                            const SkSL::ProgramSettings& settings,
                            std::string* output,
                            SkSL::ProgramInterface* outInterface,
                            ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    SkSL::Compiler compiler;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(programKind, src, settings);
    if (!program || !(*toBackend)(*program, caps, output)) {
        errorHandler->compileError(src.c_str(),
                                   compiler.errorText().c_str(),
                                   /*shaderWasCached=*/false);
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

bool SkSLToGLSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* glsl,
                SkSL::ProgramInterface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(caps, &SkSL::ToGLSL, "GLSL",
                           sksl, programKind, settings, glsl, outInterface, errorHandler);
}

bool SkSLToSPIRV(const SkSL::ShaderCaps* caps,
                 const std::string& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::ProgramInterface* outInterface,
                 ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(caps, &SkSL::ToSPIRV, /*backendLabel=*/nullptr,
                           sksl, programKind, settings, spirv, outInterface, errorHandler);
}

bool SkSLToWGSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* wgsl,
                SkSL::ProgramInterface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(caps, &SkSL::ToWGSL, "WGSL",
                           sksl, programKind, settings, wgsl, outInterface, errorHandler);
}

bool SkSLToMSL(const SkSL::ShaderCaps* caps,
               const std::string& sksl,
               SkSL::ProgramKind programKind,
               const SkSL::ProgramSettings& settings,
               std::string* msl,
               SkSL::ProgramInterface* outInterface,
               ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(caps, &SkSL::ToMetal, "MSL",
                           sksl, programKind, settings, msl, outInterface, errorHandler);
}

bool SkSLToHLSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* hlsl,
                SkSL::ProgramInterface* outInterface,
                ShaderErrorHandler* errorHandler) {
    return sksl_to_backend(caps, &SkSL::ToHLSL, "HLSL",
                           sksl, programKind, settings, hlsl, outInterface, errorHandler);
}

}  // namespace skgpu
