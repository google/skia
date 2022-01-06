/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Buffer_DEFINED
#define skgpu_Buffer_DEFINED

#include "experimental/graphite/src/Resource.h"
#include "experimental/graphite/src/ResourceTypes.h"

namespace skgpu {

class Buffer : public Resource {
public:
    size_t size() const { return fSize; }

    void* map();
    void unmap();

    bool isMapped() const { return fMapPtr; }

protected:
    Buffer(const Gpu* gpu, size_t size, BufferType type, PrioritizeGpuReads prioritizeGpuReads)
        : Resource(gpu), fSize(size), fType(type), fPrioritizeGpuReads(prioritizeGpuReads) {}

    void* fMapPtr = nullptr;

private:
    virtual void onMap() = 0;
    virtual void onUnmap() = 0;

    // TODO: Remove these getters once we start using fType and fPrioritizeGpuReads in key
    // generation. For now this silences compiler unused member warnings.
    BufferType bufferType() const { return fType; }
    PrioritizeGpuReads prioritizeGpuReads() const { return fPrioritizeGpuReads; }

    size_t             fSize;
    BufferType         fType;
    PrioritizeGpuReads fPrioritizeGpuReads;
};

} // namespace skgpu

#endif // skgpu_Buffer_DEFINED

