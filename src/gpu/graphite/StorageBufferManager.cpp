/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageBufferManager.h"

#include "src/gpu/graphite/BufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageBufferManager::~StorageBufferManager() { this->reset(); }

void StorageBufferManager::GradientCache::reset() {
    for (auto [k, _] : fOffsetCache) {
        k->unref();
    }
    fOffsetCache.reset();
    fStorage.clear();
}

void StorageBufferManager::reset() {
    fGradientCache.reset();
    fBufferInfo.reset();
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        int numStops, const SkGradientBaseShader* shader) {
    SkASSERT(!this->isFinalized());
    if (numStops > GradientCache::kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingOffset = fGradientCache.fOffsetCache.find(shader);
    if (existingOffset) {
        return {nullptr, *existingOffset};
    }

    int floatCount = numStops * 5;
    int currentFloatCount = fGradientCache.fStorage.size();
    if (GradientCache::kMaxStorageFloats - floatCount < currentFloatCount) {
        return {nullptr, -1};
    }

    float* startPtr = fGradientCache.fStorage.append(floatCount);

    // Since StorageBufferManager is single threaded, adding a ref and then storing in the
    // map should be fine.
    shader->ref();
    fGradientCache.fOffsetCache.set(shader, currentFloatCount);

    return {startPtr, currentFloatCount};
}

bool StorageBufferManager::finalize(DrawBufferManager* bufferMgr) {
    SkASSERT(!this->isFinalized());
    size_t totalBytes = fGradientCache.fStorage.size_bytes();
    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, sizeof(float));
        if (writer) {
            if (!fGradientCache.fStorage.empty()) {
                writer.write(fGradientCache.fStorage.data(), fGradientCache.fStorage.size_bytes());
            }
            this->reset();
            fBufferInfo = bufferInfo;
            return true;
        }
        return false;
    } else {
        fBufferInfo = BindBufferInfo();
    }
    return true;
}

}  // namespace skgpu::graphite
