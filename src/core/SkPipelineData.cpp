/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"

#ifdef SK_GRAPHITE_ENABLED
SkPipelineDataGatherer::SkPipelineDataGatherer(skgpu::graphite::Layout layout)
        : fUniformManager(layout)
        , fSnippetRequirementFlags(SnippetRequirementFlags::kNone) {
}
#endif

void SkPipelineDataGatherer::reset() {
#ifdef SK_GRAPHITE_ENABLED
    fTextureDataBlock.reset();
    fUniformManager.reset();
#endif
    fSnippetRequirementFlags = SnippetRequirementFlags::kNone;
}

#ifdef SK_DEBUG
void SkPipelineDataGatherer::checkReset() {
#ifdef SK_GRAPHITE_ENABLED
    SkASSERT(fTextureDataBlock.empty());
    SkDEBUGCODE(fUniformManager.checkReset());
#endif
    SkASSERT(fSnippetRequirementFlags == SnippetRequirementFlags::kNone);
}
#endif // SK_DEBUG

void SkPipelineDataGatherer::addFlags(SnippetRequirementFlags flags) {
    fSnippetRequirementFlags |= flags;
}

bool SkPipelineDataGatherer::needsLocalCoords() const {
    return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
}

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

SkTextureDataBlock* SkTextureDataBlock::Make(const SkTextureDataBlock& other,
                                             SkArenaAlloc* arena) {
    return arena->make([&](void *ptr) {
        return new (ptr) SkTextureDataBlock(other);
    });
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
        uint32_t samplerKey = std::get<1>(d).asKey();
        hash = SkOpts::hash_fn(&samplerKey, sizeof(samplerKey), hash);

        // Because the lifetime of the TextureDataCache is for just one Recording and the
        // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
        // for the hash here.
        uintptr_t proxy = reinterpret_cast<uintptr_t>(std::get<0>(d).get());
        hash = SkOpts::hash_fn(&proxy, sizeof(proxy), hash);
    }

    return hash;
}

#endif
