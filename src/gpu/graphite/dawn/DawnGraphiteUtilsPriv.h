/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED

#include "include/core/SkImageInfo.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

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

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler*);

#if !defined(__EMSCRIPTEN__)

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor&, const wgpu::YCbCrVkDescriptor&);

ImmutableSamplerInfo DawnDescriptorToImmutableSamplerInfo(const wgpu::YCbCrVkDescriptor&);
wgpu::YCbCrVkDescriptor DawnDescriptorFromImmutableSamplerInfo(ImmutableSamplerInfo);

#endif // !defined(__EMSCRIPTEN__)

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED
