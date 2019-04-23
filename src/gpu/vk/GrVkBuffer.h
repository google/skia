/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBuffer_DEFINED
#define GrVkBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkResource.h"

class GrVkGpu;

/**
 * This class serves as the base of GrVk*Buffer classes. It was written to avoid code
 * duplication in those classes.
 */
class GrVkBuffer : public SkNoncopyable {
public:
    virtual ~GrVkBuffer() {
        // either release or abandon should have been called by the owner of this object.
        SkASSERT(!fResource);
        delete [] (unsigned char*)fMapPtr;
    }

    VkBuffer                    buffer() const { return fResource->fBuffer; }
    const GrVkAlloc&            alloc() const { return fResource->fAlloc; }
    const GrVkRecycledResource* resource() const { return fResource; }
    size_t                      size() const { return fDesc.fSizeInBytes; }
    VkDeviceSize                offset() const { return fOffset;  }

    void addMemoryBarrier(const GrVkGpu* gpu,
                          VkAccessFlags srcAccessMask,
                          VkAccessFlags dstAccessMask,
                          VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask,
                          bool byRegion) const;

    enum Type {
        kVertex_Type,
        kIndex_Type,
        kUniform_Type,
        kTexel_Type,
        kCopyRead_Type,
        kCopyWrite_Type,
    };

protected:
    struct Desc {
        size_t      fSizeInBytes;
        Type        fType;         // vertex buffer, index buffer, etc.
        bool        fDynamic;
    };

    class Resource : public GrVkRecycledResource {
    public:
        Resource(VkBuffer buf, const GrVkAlloc& alloc, Type type)
            : INHERITED(), fBuffer(buf), fAlloc(alloc), fType(type) {}

#ifdef SK_TRACE_VK_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkBuffer: %d (%d refs)\n", fBuffer, this->getRefCnt());
        }
#endif
        VkBuffer           fBuffer;
        GrVkAlloc          fAlloc;
        Type               fType;

    private:
        void freeGPUData(GrVkGpu* gpu) const override;

        void onRecycle(GrVkGpu* gpu) const override { this->unref(gpu); }

        typedef GrVkRecycledResource INHERITED;
    };

    // convenience routine for raw buffer creation
    static const Resource* Create(const GrVkGpu* gpu,
                                  const Desc& descriptor);

    GrVkBuffer(const Desc& desc, const GrVkBuffer::Resource* resource)
        : fDesc(desc), fResource(resource), fOffset(0), fMapPtr(nullptr) {
    }

    void* vkMap(GrVkGpu* gpu) {
        this->internalMap(gpu, fDesc.fSizeInBytes);
        return fMapPtr;
    }
    void vkUnmap(GrVkGpu* gpu) { this->internalUnmap(gpu, this->size()); }

    // If the caller passes in a non null createdNewBuffer, this function will set the bool to true
    // if it creates a new VkBuffer to upload the data to.
    bool vkUpdateData(GrVkGpu* gpu, const void* src, size_t srcSizeInBytes,
                      bool* createdNewBuffer = nullptr);

    void vkAbandon();
    void vkRelease(const GrVkGpu* gpu);

private:
    virtual const Resource* createResource(GrVkGpu* gpu,
                                           const Desc& descriptor) {
        return Create(gpu, descriptor);
    }

    void internalMap(GrVkGpu* gpu, size_t size, bool* createdNewBuffer = nullptr);
    void internalUnmap(GrVkGpu* gpu, size_t size);
    void copyCpuDataToGpuBuffer(GrVkGpu* gpu, const void* srcData, size_t size);

    void validate() const;
    bool vkIsMapped() const;

    Desc                    fDesc;
    const Resource*         fResource;
    VkDeviceSize            fOffset;
    void*                   fMapPtr;

    typedef SkNoncopyable INHERITED;
};

#endif
