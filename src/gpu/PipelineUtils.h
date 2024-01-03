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

/** Wrappers for the SkSL compiler with useful logging and error handling. */

namespace SkSL {

enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;

bool SkSLToGLSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind programKind,
                const SkSL::ProgramSettings& settings,
                std::string* glsl,
                SkSL::ProgramInterface*,
                ShaderErrorHandler* errorHandler);

bool SkSLToSPIRV(const SkSL::ShaderCaps* caps,
                 const std::string& sksl,
                 SkSL::ProgramKind,
                 const SkSL::ProgramSettings&,
                 std::string* spirv,
                 SkSL::ProgramInterface*,
                 ShaderErrorHandler*);

bool SkSLToWGSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind,
                const SkSL::ProgramSettings&,
                std::string* wgsl,
                SkSL::ProgramInterface*,
                ShaderErrorHandler*);

bool SkSLToMSL(const SkSL::ShaderCaps* caps,
               const std::string& sksl,
               SkSL::ProgramKind kind,
               const SkSL::ProgramSettings& settings,
               std::string* msl,
               SkSL::ProgramInterface* outInterface,
               ShaderErrorHandler* errorHandler);

bool SkSLToHLSL(const SkSL::ShaderCaps* caps,
                const std::string& sksl,
                SkSL::ProgramKind kind,
                const SkSL::ProgramSettings& settings,
                std::string* hlsl,
                SkSL::ProgramInterface* outInterface,
                ShaderErrorHandler* errorHandler);

} // namespace skgpu

#endif // skgpu_PipelineUtils_DEFINED
