/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkTransferBuffer_DEFINED
#define GrVkTransferBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/vk/GrVkBuffer.h"

class GrVkGpu;

class GrVkTransferBuffer : public GrGpuBuffer, public GrVkBuffer {
public:
    static sk_sp<GrVkTransferBuffer> Make(GrVkGpu* gpu, size_t size, GrVkBuffer::Type type);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkTransferBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                       const GrVkBuffer::Resource* resource);
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

    void onMap() override {
        this->GrGpuBuffer::fMapPtr = this->vkMap(this->getVkGpu());
        if (this->GrGpuBuffer::fMapPtr &&
            this->intendedType() == GrGpuBufferType::kXferGpuToCpu) {
            const GrVkAlloc& alloc = this->alloc();
            GrVkMemory::InvalidateMappedAlloc(this->getVkGpu(), alloc, 0, alloc.fSize);
        }
    }

    void onUnmap() override { this->vkUnmap(this->getVkGpu()); }

    bool onUpdateData(const void* src, size_t srcSizeInBytes) override {
        SK_ABORT("Not implemented for transfer buffers.");
    }

    GrVkGpu* getVkGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrVkGpu*>(this->getGpu());
    }

    typedef GrGpuBuffer INHERITED;
};

#endif
