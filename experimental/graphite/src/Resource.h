/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Resource_DEFINED
#define skgpu_Resource_DEFINED

#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkTypes.h"

#include <atomic>

namespace skgpu {

class Gpu;

/**
 * Base class for objects that can be kept in the ResourceCache.
 */
class Resource {
public:
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    // Adds a usage ref to the resource. Named ref so we can easily manage usage refs with sk_sp.
    void ref() const {
        // Only the cache should be able to add the first usage ref to a resource.
        SkASSERT(this->hasUsageRef());
        // No barrier required.
        (void)fUsageRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // Removes a usage ref from the resource
    void unref() const {
        SkASSERT(this->hasUsageRef());
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == fUsageRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            this->notifyARefIsZero(LastRemovedRef::kUsageRef);
        }
    }

    // Adds a command buffer ref to the resource
    void refCommandBuffer() const {
        // No barrier required.
        (void)fCommandBufferRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // Removes a command buffer ref from the resource
    void unrefCommandBuffer() const {
        SkASSERT(this->hasCommandBufferRef());
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == fCommandBufferRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            this->notifyARefIsZero(LastRemovedRef::kCommandBufferRef);
        }
    }

    /**
     * Tests whether a object has been abandoned or released. All objects will be in this state
     * after their creating Context is destroyed or abandoned.
     *
     * @return true if the object has been released or abandoned,
     *         false otherwise.
     */
    bool wasDestroyed() const { return fGpu == nullptr; }

    int* accessCacheIndex()  const { return &fCacheArrayIndex; }

    uint32_t timestamp() const { return fTimestamp; }
    void setTimestamp(uint32_t ts) { fTimestamp = ts; }

protected:
    Resource(const Gpu*);
    virtual ~Resource();

    /** Overridden to free GPU resources in the backend API. */
    virtual void onFreeGpuData() = 0;

private:
    bool hasUsageRef() const {
        if (0 == fUsageRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of hasUsageRef() from running until previous
            // owners are all totally done calling unref().
            return false;
        }
        return true;
    }

    bool hasCommandBufferRef() const {
        if (0 == fCommandBufferRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of hasCommandBufferRef() from running
            // until previous owners are all totally done calling unrefCommandBuffer().
            return false;
        }
        return true;
    }

    // Privileged method that allows going from ref count = 0 to ref count = 1.
    void addInitialUsageRef() const {
        SkASSERT(!this->hasUsageRef());
        // No barrier required.
        (void)fUsageRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    void notifyARefIsZero(LastRemovedRef removedRef) const;

    /**
     * Frees the object in the underlying 3D API.
     */
    void freeGpuData();

    // This is not ref'ed but abandon() or release() will be called before the Gpu object is
    // destroyed. Those calls set will this to nullptr.
    const Gpu* fGpu;

    mutable std::atomic<int32_t> fUsageRefCnt;
    mutable std::atomic<int32_t> fCommandBufferRefCnt;

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache.
    mutable int fCacheArrayIndex;
    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache.
    uint32_t fTimestamp;
};

} // namespace skgpu

#endif // skgpu_Resource_DEFINED

