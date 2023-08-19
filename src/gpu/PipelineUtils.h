/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PipelineUtils_DEFINED
#define skgpu_PipelineUtils_DEFINED

#include "include/gpu/ShaderErrorHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/utils/SkShaderUtils.h"


// This file houses utilities to be shared across pipelines of different backend types.
namespace skgpu {

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static constexpr bool gPrintSkSL  = true;
#else
static constexpr bool gPrintSkSL  = false;
#endif
#ifdef SK_PRINT_NATIVE_SHADERS
static const bool gPrintBackendSL = true;
#else
static const bool gPrintBackendSL = false;
#endif

bool SkSLToGLSL(SkSL::Compiler*,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* glsl,
                SkSL::Program::Interface*,
                ShaderErrorHandler* errorHandler);

bool SkSLToSPIRV(SkSL::Compiler*,
                 const std::string& sksl,
                 SkSL::ProgramKind,
                 const SkSL::ProgramSettings&,
                 std::string* spirv,
                 SkSL::Program::Interface*,
                 ShaderErrorHandler*);

bool SkSLToWGSL(SkSL::Compiler*,
                const std::string& sksl,
                SkSL::ProgramKind,
                const SkSL::ProgramSettings&,
                std::string* wgsl,
                SkSL::Program::Interface*,
                ShaderErrorHandler*);

bool SkSLToMSL(SkSL::Compiler*,
               const std::string& sksl,
               SkSL::ProgramKind kind,
               const SkSL::ProgramSettings& settings,
               std::string* msl,
               SkSL::Program::Interface* outInterface,
               ShaderErrorHandler* errorHandler);

bool SkSLToHLSL(SkSL::Compiler*,
               const std::string& sksl,
               SkSL::ProgramKind kind,
               const SkSL::ProgramSettings& settings,
               std::string* hlsl,
               SkSL::Program::Interface* outInterface,
               ShaderErrorHandler* errorHandler);

} // namespace skgpu

#endif // skgpu_PipelineUtils_DEFINED
