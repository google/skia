/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkTransferBuffer_DEFINED
#define GrVkTransferBuffer_DEFINED

#include "GrGpuBuffer.h"
#include "GrVkBuffer.h"
#include "vk/GrVkTypes.h"

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
        if (!this->wasDestroyed()) {
            this->GrGpuBuffer::fMapPtr = this->vkMap(this->getVkGpu());
        }
    }

    void onUnmap() override {
        if (!this->wasDestroyed()) {
            this->vkUnmap(this->getVkGpu());
        }
    }

    bool onUpdateData(const void* src, size_t srcSizeInBytes) override {
        SK_ABORT("Not implemented for transfer buffers.");
        return false;
    }

    GrVkGpu* getVkGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrVkGpu*>(this->getGpu());
    }

    typedef GrGpuBuffer INHERITED;
};

#endif
