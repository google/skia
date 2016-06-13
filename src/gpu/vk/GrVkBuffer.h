/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBuffer_DEFINED
#define GrVkBuffer_DEFINED

#include "GrVkResource.h"
#include "vk/GrVkDefines.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;

/**
 * This class serves as the base of GrVk*Buffer classes. It was written to avoid code
 * duplication in those classes.
 */
class GrVkBuffer : public SkNoncopyable {
public:
    ~GrVkBuffer() {
        // either release or abandon should have been called by the owner of this object.
        SkASSERT(!fResource);
    }

    VkBuffer buffer() const { return fResource->fBuffer; }
    const GrVkAlloc& alloc() const { return fResource->fAlloc; }
    const GrVkResource* resource() const { return fResource; }
    size_t         size() const { return fDesc.fSizeInBytes; }

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
        kCopyRead_Type,
        kCopyWrite_Type,
    };

protected:
    struct Desc {
        size_t      fSizeInBytes;
        Type        fType;         // vertex buffer, index buffer, etc.
        bool        fDynamic;
    };

    class Resource : public GrVkResource {
    public:
        Resource(VkBuffer buf, const GrVkAlloc& alloc, Type type)
            : INHERITED(), fBuffer(buf), fAlloc(alloc), fType(type) {}

        VkBuffer           fBuffer;
        GrVkAlloc          fAlloc;
        Type               fType;

    private:
        void freeGPUData(const GrVkGpu* gpu) const;

        typedef GrVkResource INHERITED;
    };

    // convenience routine for raw buffer creation
    static const Resource* Create(const GrVkGpu* gpu,
                                  const Desc& descriptor);

    GrVkBuffer(const Desc& desc, const GrVkBuffer::Resource* resource)
        : fDesc(desc), fResource(resource), fMapPtr(nullptr) {
    }

    void* vkMap(const GrVkGpu* gpu);
    void vkUnmap(const GrVkGpu* gpu);
    // If the caller passes in a non null createdNewBuffer, this function will set the bool to true
    // if it creates a new VkBuffer to upload the data to.
    bool vkUpdateData(const GrVkGpu* gpu, const void* src, size_t srcSizeInBytes,
                      bool* createdNewBuffer = nullptr);

    void vkAbandon();
    void vkRelease(const GrVkGpu* gpu);

private:
    void validate() const;
    bool vkIsMapped() const;

    Desc                    fDesc;
    const Resource*         fResource;
    void*                   fMapPtr;

    typedef SkNoncopyable INHERITED;
};

#endif
