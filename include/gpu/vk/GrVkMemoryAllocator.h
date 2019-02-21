/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkMemoryAllocator_DEFINED
#define GrVkMemoryAllocator_DEFINED

#include "SkRefCnt.h"
#include "GrTypes.h"
#include "GrVkTypes.h"

class GrVkMemoryAllocator : public SkRefCnt {
public:
    enum class AllocationPropertyFlags {
        kNone                = 0,
        // Allocation will be placed in its own VkDeviceMemory and not suballocated from some larger
        // block.
        kDedicatedAllocation = 0x1,
        // Says that the backing memory can only be accessed by the device. Additionally the device
        // may lazily allocate the memory. This cannot be used with buffers that will be host
        // visible. Setting this flag does not guarantee that we will allocate memory that respects
        // it, but we will try to prefer memory that can respect it.
        kLazyAllocation      = 0x2,
        // The allocation will be mapped immediately and stay mapped until it is destroyed. This
        // flag is only valid for buffers which are host visible (i.e. must have a usage other than
        // BufferUsage::kGpuOnly).
        kPersistentlyMapped  = 0x4,
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(AllocationPropertyFlags);

    enum class BufferUsage {
        // Buffers that will only be accessed from the device (large const buffers). Will always be
        // in device local memory.
        kGpuOnly,
        // Buffers that will be accessed on the host and copied to and from a GPU resource (transfer
        // buffers). Will always be mappable and coherent memory.
        kCpuOnly,
        // Buffers that typically will be updated multiple times by the host and read on the gpu
        // (e.g. uniform or vertex buffers). Will always be mappable memory, and will prefer to be
        // in device local memory.
        kCpuWritesGpuReads,
        // Buffers which are typically writted to by the GPU and then read on the host. Will always
        // be mappable memory, and will prefer coherent and cached memory.
        kGpuWritesCpuReads,
    };

    virtual bool allocateMemoryForImage(VkImage image, AllocationPropertyFlags flags,
                                        GrVkBackendMemory*) = 0;

    virtual bool allocateMemoryForBuffer(VkBuffer buffer, BufferUsage usage,
                                         AllocationPropertyFlags flags, GrVkBackendMemory*) = 0;

    // Fills out the passed in GrVkAlloc struct for the passed in GrVkBackendMemory.
    virtual void getAllocInfo(const GrVkBackendMemory&, GrVkAlloc*) const = 0;

    // Maps the entire allocation and returns a pointer to the start of the allocation. The
    // implementation may map more memory than just the allocation, but the returned pointer must
    // point at the start of the memory for the requested allocation.
    virtual void* mapMemory(const GrVkBackendMemory&) = 0;
    virtual void unmapMemory(const GrVkBackendMemory&) = 0;

    // The following two calls are used for managing non-coherent memory. The offset is relative to
    // the start of the allocation and not the underlying VkDeviceMemory. Additionaly the client
    // must make sure that the offset + size passed in is less that or equal to the allocation size.
    // It is the responsibility of the implementation to make sure all alignment requirements are
    // followed. The client should not have to deal with any sort of alignment issues.
    virtual void flushMappedMemory(const GrVkBackendMemory&, VkDeviceSize offset,
                                   VkDeviceSize size) = 0;
    virtual void invalidateMappedMemory(const GrVkBackendMemory&, VkDeviceSize offset,
                                        VkDeviceSize size)= 0;

    virtual void freeMemory(const GrVkBackendMemory&) = 0;

    // Returns the total amount of memory that is allocated and in use by an allocation for this
    // allocator.
    virtual uint64_t totalUsedMemory() const = 0;

    // Returns the total amount of memory that is allocated by this allocator.
    virtual uint64_t totalAllocatedMemory() const = 0;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrVkMemoryAllocator::AllocationPropertyFlags)

#endif
