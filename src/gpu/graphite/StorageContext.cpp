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
    fVertexData.clear();
    fRunningLCM = 1;
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

void StorageContext::recordAlignment(size_t stride, size_t align) {
    SkASSERT(stride > 0 && align > 0);
    SkASSERT(!fGradientsFinalized);
    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
    fRunningLCM = BufferAligner::LcmAlignment(fRunningLCM, align32);
}

uint32_t StorageContext::appendVertices(const void* data,
                                        size_t count,
                                        size_t stride,
                                        size_t align) {
    SkASSERT(data && count > 0 && stride > 0 && align > 0);

    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
    SkASSERT(fRunningLCM % align32 == 0);

    uint32_t requiredBytes =
            BufferAligner::ValidateCountAndStride(count, stride, /*headroom=*/0, align32);
    if (requiredBytes == 0) {
        return 0;
    }

    uint32_t alignedVertOffset = SkAlignNonPow2(static_cast<uint32_t>(fVertexData.size()), align32);
    if (alignedVertOffset > static_cast<uint32_t>(fVertexData.size())) {
        int padBytes = alignedVertOffset - fVertexData.size();
        memset(fVertexData.append(padBytes), 0, padBytes);
    }

    char* dst = fVertexData.append(requiredBytes);
    memcpy(dst, data, requiredBytes);

    uint32_t totalOffset = SkTo<uint32_t>(fGradientCache.fGradientDataSize) + alignedVertOffset;
    return totalOffset;
}

void StorageContext::finalizePrecachedStorageData() {
    fGradientCache.fGradientDataSize =
            SkAlignNonPow2<size_t>(fGradientCache.fGradientDataSize, fRunningLCM);
    SkDEBUGCODE(fGradientsFinalized = true;)
}

BindBufferInfo StorageContext::finalize(DrawBufferManager* bufferMgr) {
    SkASSERT(fGradientsFinalized);
    size_t totalBytes = fGradientCache.fGradientDataSize + fVertexData.size_bytes();

    BindBufferInfo result;
    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] = bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            if (!fGradientCache.isEmpty()) {
                writer.write(fGradientCache.fGradientData.data(),
                             fGradientCache.fGradientData.size_bytes());
                if (fGradientCache.fGradientDataSize > fGradientCache.fGradientData.size_bytes()) {
                    writer.zeroBytes(fGradientCache.fGradientDataSize -
                                     fGradientCache.fGradientData.size_bytes());
                }
            }
            if (!fVertexData.empty()) {
                writer.write(fVertexData.data(), fVertexData.size_bytes());
            }

            result = bufferInfo;
        }
    }

    SkDEBUGCODE(fGradientsFinalized = false;)
    return result;
}

}  // namespace skgpu::graphite
