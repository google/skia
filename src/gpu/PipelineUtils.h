/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PipelineUtils_DEFINED
#define skgpu_PipelineUtils_DEFINED

#include <cstdint>
#include <string>

namespace SkSL {

enum class ProgramKind : int8_t;
struct Program;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;

/** Wrapper for the SkSL compiler with useful logging and error handling. */
bool SkSLToBackend(const SkSL::ShaderCaps* caps,
                   bool (*toBackend)(SkSL::Program&, const SkSL::ShaderCaps*, std::string*),
                   const char* backendLabel,
                   const std::string& sksl,
                   SkSL::ProgramKind programKind,
                   const SkSL::ProgramSettings& settings,
                   std::string* output,
                   SkSL::ProgramInterface* outInterface,
                   ShaderErrorHandler* errorHandler);

}  // namespace skgpu

#endif // skgpu_PipelineUtils_DEFINED
