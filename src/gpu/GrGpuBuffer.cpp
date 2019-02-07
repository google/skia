/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuBuffer.h"
#include "GrCaps.h"
#include "GrGpu.h"

GrGpuBuffer::GrGpuBuffer(GrGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                         GrAccessPattern pattern)
        : GrGpuResource(gpu)
        , fMapPtr(nullptr)
        , fSizeInBytes(sizeInBytes)
        , fAccessPattern(pattern)
        , fIntendedType(type) {}

void GrGpuBuffer::ComputeScratchKeyForDynamicVBO(size_t size, GrGpuBufferType intendedType,
                                                 GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    GrScratchKey::Builder builder(key, kType, 1 + (sizeof(size_t) + 3) / 4);
    // TODO: There's not always reason to cache a buffer by type. In some (all?) APIs it's just
    // a chunk of memory we can use/reuse for any type of data. We really only need to
    // differentiate between the "read" types (e.g. kGpuToCpu_BufferType) and "draw" types.
    builder[0] = SkToU32(intendedType);
    builder[1] = (uint32_t)size;
    if (sizeof(size_t) > 4) {
        builder[2] = (uint32_t)((uint64_t)size >> 32);
    }
}

void GrGpuBuffer::computeScratchKey(GrScratchKey* key) const {
    if (SkIsPow2(fSizeInBytes) && kDynamic_GrAccessPattern == fAccessPattern) {
        ComputeScratchKeyForDynamicVBO(fSizeInBytes, fIntendedType, key);
    }
}
