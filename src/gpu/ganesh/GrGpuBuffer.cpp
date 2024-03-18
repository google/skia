/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrGpuBuffer.h"

#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpu.h"

#include <cstdint>

GrGpuBuffer::GrGpuBuffer(GrGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                         GrAccessPattern pattern,
                         std::string_view label)
        : GrGpuResource(gpu, label)
        , fMapPtr(nullptr)
        , fSizeInBytes(sizeInBytes)
        , fAccessPattern(pattern)
        , fIntendedType(type) {}

void* GrGpuBuffer::map() {
    if (this->wasDestroyed()) {
        return nullptr;
    }
    if (!fMapPtr) {
        this->onMap(this->mapType());
    }
    return fMapPtr;
}

void GrGpuBuffer::unmap() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fMapPtr);
    this->onUnmap(this->mapType());
    fMapPtr = nullptr;
}

bool GrGpuBuffer::isMapped() const { return SkToBool(fMapPtr); }

bool GrGpuBuffer::clearToZero() {
    SkASSERT(!this->isMapped());

    if (this->wasDestroyed()) {
        return false;
    }

    if (this->intendedType()  == GrGpuBufferType::kXferGpuToCpu) {
        return false;
    }

    return this->onClearToZero();
}

bool GrGpuBuffer::updateData(const void* src, size_t offset, size_t size, bool preserve) {
    SkASSERT(!this->isMapped());
    SkASSERT(size > 0 && offset + size <= fSizeInBytes);
    SkASSERT(src);

    if (this->wasDestroyed()) {
        return false;
    }

    if (preserve) {
        size_t a = this->getGpu()->caps()->bufferUpdateDataPreserveAlignment();
        if (SkAlignTo(offset, a) != offset || SkAlignTo(size, a) != size) {
            return false;
        }
    }

    if (this->intendedType() == GrGpuBufferType::kXferGpuToCpu) {
        return false;
    }

    return this->onUpdateData(src, offset, size, preserve);
}

void GrGpuBuffer::ComputeScratchKeyForDynamicBuffer(size_t size,
                                                    GrGpuBufferType intendedType,
                                                    skgpu::ScratchKey* key) {
    static const skgpu::ScratchKey::ResourceType kType = skgpu::ScratchKey::GenerateResourceType();
    skgpu::ScratchKey::Builder builder(key, kType, 1 + (sizeof(size_t) + 3) / 4);
    builder[0] = SkToU32(intendedType);
    builder[1] = (uint32_t)size;
    if (sizeof(size_t) > 4) {
        builder[2] = (uint32_t)((uint64_t)size >> 32);
    }
}

void GrGpuBuffer::computeScratchKey(skgpu::ScratchKey* key) const {
    if (kDynamic_GrAccessPattern == fAccessPattern) {
        ComputeScratchKeyForDynamicBuffer(fSizeInBytes, fIntendedType, key);
    }
}
