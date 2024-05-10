/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
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
                         wgpu::Sampler sampler)
        : Sampler(sharedContext)
        , fSampler(std::move(sampler)) {}

static inline wgpu::AddressMode tile_mode_to_dawn_address_mode(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return wgpu::AddressMode::ClampToEdge;
        case SkTileMode::kRepeat:
            return wgpu::AddressMode::Repeat;
        case SkTileMode::kMirror:
            return wgpu::AddressMode::MirrorRepeat;
        case SkTileMode::kDecal:
            // Dawn doesn't support this mode.
            return wgpu::AddressMode::ClampToEdge;
    }
    SkUNREACHABLE;
}

sk_sp<DawnSampler> DawnSampler::Make(const DawnSharedContext* sharedContext,
                                     const SkSamplingOptions& samplingOptions,
                                     SkTileMode xTileMode,
                                     SkTileMode yTileMode) {
    wgpu::SamplerDescriptor desc;
    desc.addressModeU  = tile_mode_to_dawn_address_mode(xTileMode);
    desc.addressModeV  = tile_mode_to_dawn_address_mode(yTileMode);
    desc.magFilter     = filter_mode_to_dawn_filter_mode(samplingOptions.filter);
    desc.minFilter     = desc.magFilter;
    desc.mipmapFilter  = mipmap_mode_to_dawn_filter_mode(samplingOptions.mipmap);
    desc.lodMinClamp   = 0.0f;
    if (samplingOptions.mipmap == SkMipmapMode::kNone) {
        // Disabling mipmap by clamping max lod to first level only.
        desc.lodMaxClamp = 0.0f;
    } else {
        desc.lodMaxClamp = FLT_MAX;
    }
    desc.maxAnisotropy = 1;
    desc.compare       = wgpu::CompareFunction::Undefined;

    std::string label;
    if (sharedContext->caps()->setBackendLabels()) {
        static const char* tileModeLabels[] = {"Clamp", "Repeat", "Mirror", "Decal"};
        static const char* minMagFilterLabels[] = {"Nearest", "Linear"};
        static const char* mipFilterLabels[] = {"MipNone", "MipNearest", "MipLinear"};
        label.append("X").append(tileModeLabels[static_cast<int>(xTileMode)]);
        label.append("Y").append(tileModeLabels[static_cast<int>(yTileMode)]);
        label.append(minMagFilterLabels[static_cast<int>(samplingOptions.filter)]);
        label.append(mipFilterLabels[static_cast<int>(samplingOptions.mipmap)]);
        desc.label = label.c_str();
    }

    auto sampler = sharedContext->device().CreateSampler(&desc);
    if (!sampler) {
        return {};
    }
    return sk_sp<DawnSampler>(new DawnSampler(sharedContext, std::move(sampler)));
}

void DawnSampler::freeGpuData() {
    fSampler = nullptr;
}

} // namespace skgpu::graphite

