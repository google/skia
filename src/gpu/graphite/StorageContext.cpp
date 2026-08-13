/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageContext.h"

#include "include/private/SkAlign.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageContext::StorageContext() = default;

StorageContext::~StorageContext() {
    this->resetCache();
}

void StorageContext::GradientCache::reset() {
    fLocalGradientOffsetCache.foreach([](const SkGradientBaseShader* shader, int*) {
        shader->unref();
    });
    fLocalGradientOffsetCache.reset();
    fGradientData.clear();
    fGradientDataSize = 0;
}

void StorageContext::resetCache() {
    fGradientCache.reset();
    SkDEBUGCODE(fGradientsFinalized = false;)
}

std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    SkASSERT(!fGradientsFinalized);
    if (numStops > GradientCache::kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = fGradientCache.fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    int floatOffset = fGradientCache.fGradientData.size();
    int floatCount = numStops * 5;
    if (GradientCache::kMaxStorageFloats - floatCount < floatOffset) {
        return {nullptr, -1};
    }

    fGradientCache.fGradientData.resize(floatOffset + floatCount);
    float* dstData = fGradientCache.fGradientData.data() + floatOffset;

    shader->ref();
    fGradientCache.fLocalGradientOffsetCache.set(shader, floatOffset);
    fGradientCache.fGradientDataSize = fGradientCache.fGradientData.size_bytes();

    return {dstData, floatOffset};
}

void StorageContext::finalizePrecachedStorageData() {
    fGradientCache.fGradientDataSize =
            SkAlignTo<size_t>(fGradientCache.fGradientDataSize, kStructAlignment);
    SkDEBUGCODE(fGradientsFinalized = true;)
}

BindBufferInfo StorageContext::finalize(DrawBufferManager* bufferMgr) {
    SkASSERT(fGradientsFinalized);
    size_t totalBytes = fGradientCache.fGradientDataSize;

    BindBufferInfo result;
    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] = bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            writer.write(fGradientCache.fGradientData.data(),
                         fGradientCache.fGradientData.size_bytes());
            if (totalBytes > fGradientCache.fGradientData.size_bytes()) {
                writer.zeroBytes(totalBytes - fGradientCache.fGradientData.size_bytes());
            }

            result = bufferInfo;
        }
    }

    SkDEBUGCODE(fGradientsFinalized = false;)
    return result;
}

} // namespace skgpu::graphite
