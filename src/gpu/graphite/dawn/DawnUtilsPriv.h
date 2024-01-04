/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DawnUtilsPriv_DEFINED
#define skgpu_DawnUtilsPriv_DEFINED

#include "src/gpu/PipelineUtils.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace SkSL {

enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;

inline bool SkSLToWGSL(const SkSL::ShaderCaps* caps,
                       const std::string& sksl,
                       SkSL::ProgramKind programKind,
                       const SkSL::ProgramSettings& settings,
                       std::string* wgsl,
                       SkSL::ProgramInterface* outInterface,
                       ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps, &SkSL::ToWGSL, "WGSL",
                         sksl, programKind, settings, wgsl, outInterface, errorHandler);
}

namespace graphite {

class DawnSharedContext;

size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format);

uint32_t DawnFormatChannels(wgpu::TextureFormat format);

}  // namespace graphite
}  // namespace skgpu

#endif // skgpu_DawnUtilsPriv_DEFINED
