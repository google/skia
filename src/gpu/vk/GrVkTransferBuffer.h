/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkTransferBuffer_DEFINED
#define GrVkTransferBuffer_DEFINED

#include "GrBuffer.h"
#include "GrVkBuffer.h"

class GrVkGpu;

class GrVkTransferBuffer : public GrBuffer, public GrVkBuffer {

public:
    static GrVkTransferBuffer* Create(GrVkGpu* gpu, size_t size, GrVkBuffer::Type type);

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
            this->GrBuffer::fMapPtr = this->vkMap(this->getVkGpu());
        }
    }

    void onUnmap() override {
        if (!this->wasDestroyed()) {
            this->vkUnmap(this->getVkGpu());
        }
    }

    bool onUpdateData(const void* src, size_t srcSizeInBytes) override {
        SkFAIL("Not implemented for transfer buffers.");
        return false;
    }

    GrVkGpu* getVkGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrVkGpu*>(this->getGpu());
    }

    typedef GrBuffer INHERITED;
};

#endif
