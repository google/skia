/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypesPriv_DEFINED
#define skgpu_graphite_DawnTypesPriv_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

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

}  // namespace skgpu

namespace skgpu::graphite {

class DawnSharedContext;
enum class TextureFormat : uint8_t;

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

SkTextureCompressionType DawnFormatToCompressionType(wgpu::TextureFormat format);

TextureFormat DawnFormatToTextureFormat(wgpu::TextureFormat);
wgpu::TextureFormat TextureFormatToDawnFormat(TextureFormat);

namespace BackendTextures {

WGPUTexture GetDawnTexturePtr(const BackendTexture&);
WGPUTextureView GetDawnTextureViewPtr(const BackendTexture&);

}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnTypesPriv_DEFINED
