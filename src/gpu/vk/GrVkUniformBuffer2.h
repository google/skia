/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformBuffer2_DEFINED
#define GrVkUniformBuffer2_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/vk/GrVkBuffer.h"

class GrVkDescriptorSet;
class GrVkGpu;

class GrVkUniformBuffer2 : public GrGpuBuffer, public GrVkBuffer {
private:
    class Resource;

public:
    static sk_sp<GrVkUniformBuffer2> Make(GrVkGpu* gpu, size_t size, bool willSubAlloc = true);
    static const Resource* CreateResource(GrVkGpu* gpu, size_t size, bool willSubAlloc = true);

    void release(GrVkGpu* gpu) { this->vkRelease(gpu); }

    const VkDescriptorSet* descriptorSet() const {
        const Resource* resource = static_cast<const Resource*>(this->resource());
        return resource->descriptorSet();
    }

    void flushBufferWrites() override;

protected:
    void onAbandon() override {
        if (!this->wasDestroyed()) {
            this->vkRelease(this->getVkGpu());
        }
        this->GrGpuBuffer::onAbandon();
    }
    void onRelease() override {
        if (!this->wasDestroyed()) {
            this->vkRelease(this->getVkGpu());
        }

        this->GrGpuBuffer::onRelease();
    }

private:
    class Resource : public GrVkBuffer::Resource {
    public:
        Resource(GrVkGpu* gpu, VkBuffer buf, const GrVkAlloc& alloc,
                 const GrVkDescriptorSet* descSet)
            : INHERITED(gpu, buf, alloc, kUniform_Type)
            , fDescriptorSet(descSet) {}

        void freeGPUData() const override;
        void onRecycle() const override;

        const VkDescriptorSet* descriptorSet() const;

        typedef GrVkBuffer::Resource INHERITED;

    private:
        const GrVkDescriptorSet* fDescriptorSet;
    };

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
        if (!this->wasDestroyed()) {
            return this->vkUpdateData(this->getVkGpu(), src, srcSizeInBytes);
        } else {
            return false;
        }
    }

    GrVkGpu* getVkGpu() const;

    const GrVkBuffer::Resource* createResource(GrVkGpu* gpu,
                                               const GrVkBuffer::Desc& descriptor) override;

    GrVkUniformBuffer2(GrVkGpu* gpu,
                       const GrVkBuffer::Desc& desc,
                       const GrVkUniformBuffer2::Resource* resource,
                       void* mappedPtr);

    typedef GrVkBuffer INHERITED;
};

#endif
