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

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler*);

struct DawnTextureSpec {
    DawnTextureSpec() = default;
    DawnTextureSpec(const DawnTextureInfo& info)
            : fFormat(info.fFormat)
            , fViewFormat(info.fViewFormat)
            , fUsage(info.fUsage)
            , fAspect(info.fAspect)
#if !defined(__EMSCRIPTEN__)
            , fYcbcrVkDescriptor(info.fYcbcrVkDescriptor)
#endif
            , fSlice(info.fSlice) {
    }

    bool operator==(const DawnTextureSpec& that) const;

    bool isCompatible(const DawnTextureSpec& that) const;

    wgpu::TextureFormat getViewFormat() const {
        return fViewFormat != wgpu::TextureFormat::Undefined ? fViewFormat : fFormat;
    }

    SkString toString() const;

    wgpu::TextureFormat fFormat = wgpu::TextureFormat::Undefined;
    // `fViewFormat` is always single plane format or plane view format for a multiplanar
    // wgpu::Texture.
    wgpu::TextureFormat fViewFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureUsage fUsage = wgpu::TextureUsage::None;
    wgpu::TextureAspect fAspect = wgpu::TextureAspect::All;
#if !defined(__EMSCRIPTEN__)
    wgpu::YCbCrVkDescriptor fYcbcrVkDescriptor = {};
#endif
    uint32_t fSlice = 0;
};

#if !defined(__EMSCRIPTEN__)

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor&, const wgpu::YCbCrVkDescriptor&);

ImmutableSamplerInfo DawnDescriptorToImmutableSamplerInfo(const wgpu::YCbCrVkDescriptor&);
wgpu::YCbCrVkDescriptor DawnDescriptorFromImmutableSamplerInfo(ImmutableSamplerInfo);

#endif // !defined(__EMSCRIPTEN__)

size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format);

SkTextureCompressionType DawnFormatToCompressionType(wgpu::TextureFormat format);

uint32_t DawnFormatChannels(wgpu::TextureFormat format);

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat);
bool DawnFormatIsDepth(wgpu::TextureFormat);
bool DawnFormatIsStencil(wgpu::TextureFormat);

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             Mipmapped mipmapped);

DawnTextureInfo DawnTextureInfoFromWGPUTexture(WGPUTexture texture);

namespace TextureInfos {
DawnTextureSpec GetDawnTextureSpec(const TextureInfo& dawnInfo);

wgpu::TextureFormat GetDawnViewFormat(const TextureInfo& dawnInfo);
wgpu::TextureAspect GetDawnAspect(const TextureInfo& dawnInfo);
}  // namespace TextureInfos

namespace BackendTextures {
WGPUTexture GetDawnTexturePtr(const BackendTexture&);
WGPUTextureView GetDawnTextureViewPtr(const BackendTexture&);
}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnTypesPriv_DEFINED
