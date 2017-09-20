/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBuffer.h"
#include "GrGpu.h"
#include "GrCaps.h"

GrBuffer* GrBuffer::CreateCPUBacked(GrGpu* gpu, size_t sizeInBytes, GrBufferType intendedType,
                                    const void* data) {
    SkASSERT(GrBufferTypeIsVertexOrIndex(intendedType));
    void* cpuData;
    if (gpu->caps()->mustClearUploadedBufferData()) {
        cpuData = sk_calloc_throw(sizeInBytes);
    } else {
        cpuData = sk_malloc_flags(sizeInBytes, SK_MALLOC_THROW);
    }
    if (data) {
        memcpy(cpuData, data, sizeInBytes);
    }
    return new GrBuffer(gpu, sizeInBytes, intendedType, cpuData);
}

GrBuffer::GrBuffer(GrGpu* gpu, size_t sizeInBytes, GrBufferType type, void* cpuData)
    : INHERITED(gpu)
    , fMapPtr(nullptr)
    , fSizeInBytes(sizeInBytes)
    , fAccessPattern(kDynamic_GrAccessPattern)
    , fCPUData(cpuData)
    , fIntendedType(type) {
    this->registerWithCache(SkBudgeted::kNo);
}

GrBuffer::GrBuffer(GrGpu* gpu, size_t sizeInBytes, GrBufferType type, GrAccessPattern pattern)
    : INHERITED(gpu)
    , fMapPtr(nullptr)
    , fSizeInBytes(sizeInBytes)
    , fAccessPattern(pattern)
    , fCPUData(nullptr)
    , fIntendedType(type) {
    // Subclass registers with cache.
}

void GrBuffer::ComputeScratchKeyForDynamicVBO(size_t size, GrBufferType intendedType,
                                              GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    GrScratchKey::Builder builder(key, kType, 1 + (sizeof(size_t) + 3) / 4);
    // TODO: There's not always reason to cache a buffer by type. In some (all?) APIs it's just
    // a chunk of memory we can use/reuse for any type of data. We really only need to
    // differentiate between the "read" types (e.g. kGpuToCpu_BufferType) and "draw" types.
    builder[0] = intendedType;
    builder[1] = (uint32_t)size;
    if (sizeof(size_t) > 4) {
        builder[2] = (uint32_t)((uint64_t)size >> 32);
    }
}

bool GrBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    SkASSERT(this->isCPUBacked());
    memcpy(fCPUData, src, srcSizeInBytes);
    return true;
}

void GrBuffer::computeScratchKey(GrScratchKey* key) const {
    if (!this->isCPUBacked() && SkIsPow2(fSizeInBytes) &&
        kDynamic_GrAccessPattern == fAccessPattern) {
        ComputeScratchKeyForDynamicVBO(fSizeInBytes, fIntendedType, key);
    }
}

// We need to tell TSAN that onMap() makes new allocations.
// If we don't, and the buffer returned overlaps with memory we've used before,
// it may see this as a use-after-free or possibly a data race.
#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer)
    extern "C" void AnnotateNewMemory(const char*, int, void*, long);
#else
    static void AnnotateNewMemory(const char*, int, void*, long) {}
#endif

void* GrBuffer::map() {
    if (!fMapPtr) {
        this->onMap();
        AnnotateNewMemory(__FILE__, __LINE__, fMapPtr, (long)this->sizeInBytes());
    }
    return fMapPtr;
}
