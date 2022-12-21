/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnUtilsPriv_DEFINED
#define skgpu_graphite_DawnUtilsPriv_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/private/SkSLProgramKind.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "webgpu/webgpu_cpp.h"

namespace SkSL {
class Compiler;
struct ProgramSettings;
}

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {
class DawnSharedContext;

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat);
bool DawnFormatIsDepth(wgpu::TextureFormat);
bool DawnFormatIsStencil(wgpu::TextureFormat);

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

bool SkSLToSPIRV(SkSL::Compiler*,
                 const std::string& sksl,
                 SkSL::ProgramKind kind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::Program::Inputs* outInputs,
                 ShaderErrorHandler* errorHandler);

wgpu::ShaderModule DawnCompileSPIRVShaderModule(const DawnSharedContext* sharedContext,
                                                const std::string& spirv,
                                                ShaderErrorHandler* errorHandler);
} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnUtilsPriv_DEFINED
