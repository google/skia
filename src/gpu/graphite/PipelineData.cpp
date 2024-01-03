/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PipelineData.h"

#include "src/core/SkChecksum.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

PipelineDataGatherer::PipelineDataGatherer(Layout layout) : fUniformManager(layout) {}

void PipelineDataGatherer::resetWithNewLayout(Layout layout) {
    fUniformManager.resetWithNewLayout(layout);
    fTextureDataBlock.reset();
}

#ifdef SK_DEBUG
void PipelineDataGatherer::checkReset() {
    SkASSERT(fTextureDataBlock.empty());
    SkASSERT(fUniformManager.isReset());
}

void PipelineDataGatherer::setExpectedUniforms(SkSpan<const Uniform> expectedUniforms) {
    fUniformManager.setExpectedUniforms(expectedUniforms);
}
#endif // SK_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
UniformDataBlock* UniformDataBlock::Make(const UniformDataBlock& other, SkArenaAlloc* arena) {
    static constexpr size_t kUniformAlignment = alignof(void*);
    char* mem = static_cast<char*>(arena->makeBytesAlignedTo(other.size(), kUniformAlignment));
    memcpy(mem, other.data(), other.size());

    return arena->make([&](void* ptr) {
        return new (ptr) UniformDataBlock(SkSpan<const char>(mem, other.size()));
    });
}

uint32_t UniformDataBlock::hash() const {
    return SkChecksum::Hash32(fData.data(), fData.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TextureDataBlock* TextureDataBlock::Make(const TextureDataBlock& other,
                                             SkArenaAlloc* arena) {
    return arena->make([&](void *ptr) {
        return new (ptr) TextureDataBlock(other);
    });
}

bool TextureDataBlock::operator==(const TextureDataBlock& other) const {
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

uint32_t TextureDataBlock::hash() const {
    uint32_t hash = 0;

    for (auto& d : fTextureData) {
        SamplerDesc samplerKey = std::get<1>(d);
        hash = SkChecksum::Hash32(&samplerKey, sizeof(samplerKey), hash);

        // Because the lifetime of the TextureDataCache is for just one Recording and the
        // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
        // for the hash here.
        uintptr_t proxy = reinterpret_cast<uintptr_t>(std::get<0>(d).get());
        hash = SkChecksum::Hash32(&proxy, sizeof(proxy), hash);
    }

    return hash;
}

#ifdef SK_DEBUG
UniformExpectationsValidator::UniformExpectationsValidator(PipelineDataGatherer *gatherer,
                                                           SkSpan<const Uniform> expectedUniforms)
        : fGatherer(gatherer) {
    fGatherer->setExpectedUniforms(expectedUniforms);
}
#endif

} // namespace skgpu::graphite
