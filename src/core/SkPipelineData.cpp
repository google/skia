/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

void SkPipelineDataGatherer::reset() {
#ifdef SK_GRAPHITE_ENABLED
    fTextureDataBlock.reset();
    fBlendInfo = BlendInfo();
    fUniformManager.reset();
#endif
}

#ifdef SK_DEBUG
void SkPipelineDataGatherer::checkReset() {
#ifdef SK_GRAPHITE_ENABLED
    SkASSERT(fTextureDataBlock.empty());
    SkASSERT(fBlendInfo == BlendInfo());
    SkDEBUGCODE(fUniformManager.checkReset());
#endif
}
#endif // SK_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
SkUniformDataBlock* SkUniformDataBlock::Make(const SkUniformDataBlock& other,
                                             SkArenaAlloc* arena) {
    static constexpr size_t kUniformAlignment = alignof(void*);
    char* mem = static_cast<char*>(arena->makeBytesAlignedTo(other.size(), kUniformAlignment));
    memcpy(mem, other.data(), other.size());

    return arena->make([&](void* ptr) {
        return new (ptr) SkUniformDataBlock(SkSpan<const char>(mem, other.size()));
    });
}

uint32_t SkUniformDataBlock::hash() const {
    return SkOpts::hash_fn(fData.data(), fData.size(), 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SK_GRAPHITE_ENABLED
static constexpr int kSkFilterModeCount = static_cast<int>(SkFilterMode::kLast) + 1;

std::unique_ptr<SkTextureDataBlock> SkTextureDataBlock::Make(const SkTextureDataBlock& other,
                                                             SkArenaAlloc* /* arena */) {
    return std::make_unique<SkTextureDataBlock>(other);
}

bool SkTextureDataBlock::TextureInfo::operator==(const TextureInfo& other) const {
    return fProxy == other.fProxy &&
           fSamplingOptions == other.fSamplingOptions &&
           fTileModes[0] == other.fTileModes[0] &&
           fTileModes[1] == other.fTileModes[1];
}

uint32_t SkTextureDataBlock::TextureInfo::samplerKey() const {
    static_assert(kSkTileModeCount <= 4 && kSkFilterModeCount <= 2);
    return (static_cast<int>(fTileModes[0])           << 0) |
           (static_cast<int>(fTileModes[1])           << 2) |
           (static_cast<int>(fSamplingOptions.filter) << 4) |
           (static_cast<int>(fSamplingOptions.mipmap) << 5);
}

bool SkTextureDataBlock::operator==(const SkTextureDataBlock& other) const {
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

uint32_t SkTextureDataBlock::hash() const {
    uint32_t hash = 0;

    for (auto& d : fTextureData) {
        uint32_t samplerKey = d.samplerKey();
        hash = SkOpts::hash_fn(&samplerKey, sizeof(samplerKey), hash);

        // Because the lifetime of the TextureDataCache is for just one Recording and the
        // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
        // for the hash here. This is a bit sloppy though in that it would be nice if proxies backed
        // by the same scratch texture hashed the same (it is tough to see how we could do that
        // at DrawPass creation time though).
        hash = SkOpts::hash_fn(d.fProxy.get(), sizeof(skgpu::graphite::TextureProxy*), hash);
    }

    return hash;
}

#endif
