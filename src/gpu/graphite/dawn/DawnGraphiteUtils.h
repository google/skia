/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypesPriv_DEFINED
#define skgpu_graphite_DawnTypesPriv_DEFINED

#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/core/SkEnumBitMask.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/codegen/SkSLNativeShader.h"
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
                       SkSL::NativeShader* wgsl,
                       SkSL::ProgramInterface* outInterface,
                       ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps,
                         &SkSL::ToWGSL,
                         "WGSL",
                         sksl,
                         programKind,
                         settings,
                         wgsl,
                         outInterface,
                         errorHandler);
}

}  // namespace skgpu

namespace skgpu::graphite {

class DawnSharedContext;
enum class TextureFormat : uint8_t;

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const SkSL::NativeShader& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler*);

#if !defined(__EMSCRIPTEN__)

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor&, const wgpu::YCbCrVkDescriptor&);

ImmutableSamplerInfo DawnDescriptorToImmutableSamplerInfo(const wgpu::YCbCrVkDescriptor&);
wgpu::YCbCrVkDescriptor DawnDescriptorFromImmutableSamplerInfo(ImmutableSamplerInfo);

#endif // !defined(__EMSCRIPTEN__)

TextureFormat DawnFormatToTextureFormat(wgpu::TextureFormat);
wgpu::TextureFormat TextureFormatToDawnFormat(TextureFormat);

// Helper bit mask for the columns of the "Texture Format Capabilities" tables in
// https://gpuweb.github.io/gpuweb/#texture-format-caps
enum class DawnFormatFlag {
    None      = 0x0,
    // Corresponds to "float" in GPUTextureSampleType column; "unfilterable-float", "uint" and
    // "sint" are readable but not filterable and can be inferred from the format's type.
    Filter    = 0x1,
    Render    = 0x2,  // Support for wgpu::TextureUsage::RenderAttachment
    Blend     = 0x4,  // Corresponds to https://gpuweb.github.io/gpuweb/#blendable
    MSAA      = 0x8,  // Supports MSAA (4x only)
    Resolve   = 0x10, // Supports being a resolve target
    WriteOnly = 0x20, // Support for wgpu::TextureUsage::StorageBinding as "write-only"
    ReadOnly  = 0x40, // Support for wgpu::TextureUsage::StorageBinding as "read-only"
    ReadWrite = 0x80, // Support for wgpu::TextureUsage::StorageBinding as "read-write"
};
SK_MAKE_BITMASK_OPS(DawnFormatFlag)

SkEnumBitMask<DawnFormatFlag> DawnTextureFormatSupport(wgpu::Device, wgpu::TextureFormat);

namespace BackendTextures {

WGPUTexture GetDawnTexturePtr(const BackendTexture&);
WGPUTextureView GetDawnTextureViewPtr(const BackendTexture&);

}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnTypesPriv_DEFINED
