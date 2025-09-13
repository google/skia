/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include <cfloat>

namespace skgpu::graphite {

namespace {

wgpu::FilterMode filter_mode_to_dawn_filter_mode(SkFilterMode mode) {
    switch (mode) {
        case SkFilterMode::kNearest:
            return wgpu::FilterMode::Nearest;
        case SkFilterMode::kLinear:
            return wgpu::FilterMode::Linear;
    }
    SkUNREACHABLE;
}

wgpu::MipmapFilterMode mipmap_mode_to_dawn_filter_mode(SkMipmapMode mode) {
    switch (mode) {
        case SkMipmapMode::kNone:
            // Dawn doesn't have none filter mode.
            return wgpu::MipmapFilterMode::Nearest;
        case SkMipmapMode::kNearest:
            return wgpu::MipmapFilterMode::Nearest;
        case SkMipmapMode::kLinear:
            return wgpu::MipmapFilterMode::Linear;
    }
    SkUNREACHABLE;
}
}

DawnSampler::DawnSampler(const DawnSharedContext* sharedContext,
                         SamplerDesc samplerDesc,
                         wgpu::Sampler sampler)
        : Sampler(sharedContext)
        , fSampler(std::move(sampler))
        , fSamplerDesc(samplerDesc) {}

static inline std::pair<wgpu::AddressMode, wgpu::AddressMode>
tile_modes_to_dawn_address_modes(const SamplerDesc& samplerDesc) {
    auto to_dawn_mode = [](SkTileMode tm) -> wgpu::AddressMode {
        switch (tm) {
            case SkTileMode::kClamp:
                return wgpu::AddressMode::ClampToEdge;
            case SkTileMode::kRepeat:
                return wgpu::AddressMode::Repeat;
            case SkTileMode::kMirror:
                return wgpu::AddressMode::MirrorRepeat;
            case SkTileMode::kDecal:
                // Dawn doesn't support kDecal; considered an error if we reach this point.
                SkASSERT(false);
                return {};
        }
        SkUNREACHABLE;
    };

    return {to_dawn_mode(samplerDesc.tileModeX()), to_dawn_mode(samplerDesc.tileModeY())};
}

sk_sp<DawnSampler> DawnSampler::Make(const DawnSharedContext* sharedContext,
                                     SamplerDesc samplerDesc) {
    wgpu::SamplerDescriptor desc;
    const SkSamplingOptions& samplingOptions = samplerDesc.samplingOptions();
    std::tie(desc.addressModeU, desc.addressModeV) = tile_modes_to_dawn_address_modes(samplerDesc);
    desc.magFilter    = filter_mode_to_dawn_filter_mode(samplingOptions.filter);
    desc.minFilter    = desc.magFilter;
    desc.mipmapFilter = mipmap_mode_to_dawn_filter_mode(samplingOptions.mipmap);
    desc.lodMinClamp  = 0.0f;
    if (samplingOptions.mipmap == SkMipmapMode::kNone) {
        // Disabling mipmap by clamping max lod to first level only.
        desc.lodMaxClamp = 0.0f;
    } else {
        desc.lodMaxClamp = FLT_MAX;
    }
    desc.maxAnisotropy = 1;
    desc.compare       = wgpu::CompareFunction::Undefined;

#if !defined(__EMSCRIPTEN__)
    wgpu::YCbCrVkDescriptor ycbcrDescriptor;
    if (samplerDesc.isImmutable()) {
        ycbcrDescriptor =
                DawnDescriptorFromImmutableSamplerInfo(samplerDesc.immutableSamplerInfo());
        desc.nextInChain = &ycbcrDescriptor;
    }
#endif

    std::string label;
    if (sharedContext->caps()->setBackendLabels()) {
        static const char* tileModeLabels[] = {"Clamp", "Repeat", "Mirror", "Decal"};
        static const char* minMagFilterLabels[] = {"Nearest", "Linear"};
        static const char* mipFilterLabels[] = {"MipNone", "MipNearest", "MipLinear"};
        label.append("X").append(tileModeLabels[static_cast<int>(samplerDesc.tileModeX())]);
        label.append("Y").append(tileModeLabels[static_cast<int>(samplerDesc.tileModeY())]);
        label.append(minMagFilterLabels[static_cast<int>(samplingOptions.filter)]);
        label.append(mipFilterLabels[static_cast<int>(samplingOptions.mipmap)]);
#if !defined(__EMSCRIPTEN__)
        if (DawnDescriptorIsValid(ycbcrDescriptor)) {
            label.append("YCbCr");

            if (DawnDescriptorUsesExternalFormat(ycbcrDescriptor)) {
                label.append("ExternalFormat");
                label.append(std::to_string(ycbcrDescriptor.externalFormat));
            } else {
                label.append("KnownFormat").append(std::to_string(ycbcrDescriptor.vkFormat));
            }

            label.append("Model").append(std::to_string(ycbcrDescriptor.vkYCbCrModel));
            label.append("Range").append(std::to_string(ycbcrDescriptor.vkYCbCrRange));

            label.append("ComponentSwizzleRGBA");
            label.append(std::to_string(ycbcrDescriptor.vkComponentSwizzleRed));
            label.append(std::to_string(ycbcrDescriptor.vkComponentSwizzleGreen));
            label.append(std::to_string(ycbcrDescriptor.vkComponentSwizzleBlue));
            label.append(std::to_string(ycbcrDescriptor.vkComponentSwizzleAlpha));

            label.append("ChromaOffset");
            label.append("X").append(std::to_string(ycbcrDescriptor.vkXChromaOffset));
            label.append("Y").append(std::to_string(ycbcrDescriptor.vkYChromaOffset));

            static const char* chromaFilterLabels[] = {
                    "WGPU_Undefined", "WGPU_Nearest", "WGPU_Linear"};
            label.append("ChromaFilter");
            label.append(chromaFilterLabels[static_cast<int>(ycbcrDescriptor.vkChromaFilter)]);

            label.append("ForceExplicitReconstruct");
            label.append(std::to_string(ycbcrDescriptor.forceExplicitReconstruction));
        }
#endif
        desc.label = label.c_str();
    }

    auto sampler = sharedContext->device().CreateSampler(&desc);
    if (!sampler) {
        return {};
    }
    return sk_sp<DawnSampler>(new DawnSampler(sharedContext, samplerDesc, std::move(sampler)));
}

void DawnSampler::freeGpuData() {
    fSampler = nullptr;
}

} // namespace skgpu::graphite
