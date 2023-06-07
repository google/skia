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
static constexpr bool gPrintSKSL  = true;
#else
static constexpr bool gPrintSKSL  = false;
#endif
#ifdef SK_PRINT_NATIVE_SHADERS
static const bool gPrintBackendSL = true;
#else
static const bool gPrintBackendSL = false;
#endif

// SkSL->SPIR-V is only needed by Dawn + Vulkan backends, but seeing as this is a small wrapper
// function, it's fine to be here even if not all backends end up using it.
bool SkSLToSPIRV(SkSL::Compiler*,
                 const std::string& sksl,
                 SkSL::ProgramKind,
                 const SkSL::ProgramSettings&,
                 std::string* spirv,
                 SkSL::Program::Interface*,
                 ShaderErrorHandler*);

} // namespace skgpu

#endif // skgpu_PipelineUtils_DEFINED
