/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuBuffer.h"

GrGpuBuffer::GrGpuBuffer(GrGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                         GrAccessPattern pattern)
        : GrGpuResource(gpu)
        , fMapPtr(nullptr)
        , fSizeInBytes(sizeInBytes)
        , fAccessPattern(pattern)
        , fIntendedType(type) {}

void* GrGpuBuffer::map() {
    if (this->wasDestroyed()) {
        return nullptr;
    }
    SkASSERT(!fHasWrittenToBuffer || fAccessPattern == kDynamic_GrAccessPattern);
    if (!fMapPtr) {
        this->onMap();
    }
    return fMapPtr;
}

void GrGpuBuffer::unmap() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fMapPtr);
    this->onUnmap();
    fMapPtr = nullptr;
#ifdef SK_DEBUG
    fHasWrittenToBuffer = true;
#endif
}

bool GrGpuBuffer::isMapped() const { return SkToBool(fMapPtr); }

bool GrGpuBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    SkASSERT(!fHasWrittenToBuffer || fAccessPattern == kDynamic_GrAccessPattern);
    SkASSERT(!this->isMapped());
    SkASSERT(srcSizeInBytes <= fSizeInBytes);
    if (this->intendedType() == GrGpuBufferType::kXferGpuToCpu) {
        return false;
    }
    bool result = this->onUpdateData(src, srcSizeInBytes);
#ifdef SK_DEBUG
    if (result) {
        fHasWrittenToBuffer = true;
    }
#endif
    return result;
}

void GrGpuBuffer::ComputeScratchKeyForDynamicBuffer(size_t size, GrGpuBufferType intendedType,
                                                 GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    GrScratchKey::Builder builder(key, kType, 1 + (sizeof(size_t) + 3) / 4);
    builder[0] = SkToU32(intendedType);
    builder[1] = (uint32_t)size;
    if (sizeof(size_t) > 4) {
        builder[2] = (uint32_t)((uint64_t)size >> 32);
    }
}

void GrGpuBuffer::computeScratchKey(GrScratchKey* key) const {
    if (SkIsPow2(fSizeInBytes) && kDynamic_GrAccessPattern == fAccessPattern) {
        ComputeScratchKeyForDynamicBuffer(fSizeInBytes, fIntendedType, key);
    }
}
