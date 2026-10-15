/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGPUContext_DEFINED
#define SkGPUContext_DEFINED

#include "include/private/SkAPI.h"

#include <chrono>
#include <cstddef>
#include <optional>

class SkTraceMemoryDump;

/**
 * Internal virtual base class representing a GPU context (Graphite or Ganesh) owned by SkContext.
 *
 * This lightweight interface is always defined so that core files (such as SkContext) can compile
 * in modular builds where Ganesh and/or Graphite may be compiled out. It cannot be instantiated
 * directly unless a GPU backend (Graphite's Context or Ganesh's adapter) subclasses it.
 */
class SK_API SkGPUContext {
public:
    SkGPUContext(const SkGPUContext&) = delete;
    SkGPUContext(SkGPUContext&&) = delete;
    SkGPUContext& operator=(const SkGPUContext&) = delete;
    SkGPUContext& operator=(SkGPUContext&&) = delete;

    virtual ~SkGPUContext() = default;

    /**
     * Submits outstanding GPU work to the underlying backend API. Returns true if the submission
     * succeeded.
     */
    virtual bool submit() = 0;

    /**
     * Checks whether any asynchronous GPU work is complete and invokes associated callbacks.
     */
    virtual void checkAsyncWorkCompletion() = 0;

    /**
     * Frees GPU resources created and held by the context that are not currently in use to reduce
     * GPU memory pressure.
     */
    virtual void freeGpuResources() = 0;

    /**
     * Purges unlocked GPU resources that haven't been used in the past 'msNotUsed' milliseconds or
     * are otherwise marked for deletion, regardless of whether the context is under budget.
     * Optionally accepts 'microsMaxPurgingDur' (supported by Graphite) to limit the time spent
     * purging resources.
     */
    virtual void performDeferredCleanup(
            std::chrono::milliseconds msNotUsed,
            std::optional<std::chrono::microseconds> microsMaxPurgingDur = std::nullopt) = 0;

    /**
     * Returns the number of bytes of the context's GPU memory cache budget currently in use.
     */
    virtual size_t currentBudgetedBytes() const = 0;

    /**
     * Returns the number of bytes in the context's GPU resource cache that are currently purgeable.
     */
    virtual size_t currentPurgeableBytes() const = 0;

    /**
     * Returns the maximum size of the context's GPU memory cache budget in bytes.
     */
    virtual size_t maxBudgetedBytes() const = 0;

    /**
     * Sets the maximum size of the context's GPU memory cache budget in bytes. If the new budget is
     * lower than the current usage, the cache will attempt to purge resources to get under budget.
     */
    virtual void setMaxBudgetedBytes(size_t bytes) = 0;

    /**
     * Enumerates all cached GPU resources owned by the context and dumps their memory statistics to
     * traceMemoryDump.
     */
    virtual void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const = 0;

    /**
     * Returns true if the underlying backend GPU context has entered an unrecoverable, lost state.
     */
    virtual bool isDeviceLost() const = 0;

    /**
     * Returns the maximum texture dimension supported by the underlying GPU backend.
     */
    virtual int maxTextureSize() const = 0;

    /**
     * Returns true if this GPU context supports protected content.
     */
    virtual bool supportsProtectedContent() const = 0;

protected:
    SkGPUContext() = default;
};

#endif  // SkGPUContext_DEFINED
