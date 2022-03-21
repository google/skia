/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

SkPipelineData::SkPipelineData(sk_sp<SkUniformData> initial)
        : fUniformDataBlock(std::move(initial)) {
}

void SkPipelineData::add(sk_sp<SkUniformData> uniforms) {
    SkASSERT(uniforms && uniforms->count());
    fUniformDataBlock.add(std::move(uniforms));
}

size_t SkUniformDataBlock::totalUniformSize() const {
    size_t total = 0;

    // TODO: It seems like we need to worry about alignment between the separate sets of uniforms
    for (auto& u : fUniformData) {
        total += u->dataSize();
    }

    return total;
}

int SkUniformDataBlock::numUniforms() const {
    int total = 0;

    for (auto& u : fUniformData) {
        total += u->count();
    }

    return total;
}

bool SkUniformDataBlock::operator==(const SkUniformDataBlock& other) const {
    if (fUniformData.size() != other.fUniformData.size()) {
        return false;
    }

    for (size_t i = 0; i < fUniformData.size(); ++i) {
        if (*fUniformData[i] != *other.fUniformData[i]) {
            return false;
        }
    }

    return true;
}

uint32_t SkUniformDataBlock::hash() const {
    uint32_t hash = 0;

    for (auto& u : fUniformData) {
        hash = SkOpts::hash_fn(u->data(), u->dataSize(), hash);
    }

    return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SK_GRAPHITE_ENABLED
static constexpr int kSkFilterModeCount = static_cast<int>(SkFilterMode::kLast) + 1;

bool SkPipelineData::TextureDataBlock::TextureInfo::operator==(const TextureInfo& other) const {
    return fProxy == other.fProxy &&
           fSamplingOptions == other.fSamplingOptions &&
           fTileModes[0] == other.fTileModes[0] &&
           fTileModes[1] == other.fTileModes[1];
}

uint32_t SkPipelineData::TextureDataBlock::TextureInfo::samplerKey() const {
    static_assert(kSkTileModeCount <= 4 && kSkFilterModeCount <= 2);
    return (static_cast<int>(fTileModes[0])           << 0) |
           (static_cast<int>(fTileModes[1])           << 2) |
           (static_cast<int>(fSamplingOptions.filter) << 4) |
           (static_cast<int>(fSamplingOptions.mipmap) << 5);
}

bool SkPipelineData::TextureDataBlock::operator==(const TextureDataBlock& other) const {
    if (fTextureData.size() != other.fTextureData.size()) {
        return false;
    }

    for (size_t i = 0; i < fTextureData.size(); ++i) {
        if (fTextureData[i] != other.fTextureData[i]) {
            return false;
        }
    }

    return true;
}

uint32_t SkPipelineData::TextureDataBlock::hash() const {
    uint32_t hash = 0;

    for (auto& d : fTextureData) {
        uint32_t samplerKey = d.samplerKey();
        hash = SkOpts::hash_fn(&samplerKey, sizeof(samplerKey), hash);

        // Because the lifetime of the TextureDataCache is for just one Recording and the
        // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
        // for the hash here. This is a bit sloppy though in that it would be nice if proxies backed
        // by the same scratch texture hashed the same (it is tough to see how we could do that
        // at DrawPass creation time though).
        hash = SkOpts::hash_fn(d.fProxy.get(), sizeof(skgpu::TextureProxy*), hash);
    }

    return hash;
}

#endif
