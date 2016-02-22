/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkTransferBuffer_DEFINED
#define GrVkTransferBuffer_DEFINED

#include "GrTransferBuffer.h"
#include "GrVkBuffer.h"
#include "vk/GrVkInterface.h"

class GrVkGpu;

class GrVkTransferBuffer : public GrTransferBuffer, public GrVkBuffer {

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

    void* onMap() override {
        if (!this->wasDestroyed()) {
            return this->vkMap(this->getVkGpu());
        } else {
            return nullptr;
        }
    }

    void onUnmap() override {
        if (!this->wasDestroyed()) {
            this->vkUnmap(this->getVkGpu());
        }
    }

    GrVkGpu* getVkGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrVkGpu*>(this->getGpu());
    }

    typedef GrTransferBuffer INHERITED;
};

#endif
