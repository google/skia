/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkIDChangeListener_DEFINED
#define SkIDChangeListener_DEFINED

#include "include/core/SkTypes.h"

#include <atomic>

/**
 * Used to be notified when a gen/unique ID is invalidated, typically to preemptively purge
 * associated items from a cache that are no longer reachable. The listener can
 * be marked for deregistration if the cached item is remove before the listener is
 * triggered. This prevents unbounded listener growth when cache items are routinely
 * removed before the gen ID/unique ID is invalidated.
 */
class SkIDChangeListener : public SkRefCnt {
public:
    SkIDChangeListener() = default;

    virtual ~SkIDChangeListener() = default;

    virtual void changed() = 0;

    /**
     * Mark the listener is no longer needed. It should be removed and changed() should not be
     * called.
     */
    void markShouldDeregister() { fShouldDeregister.store(true, std::memory_order_relaxed); }

    /** Indicates whether markShouldDeregister was called. */
    bool shouldDeregister() { return fShouldDeregister.load(std::memory_order_acquire); }

private:
    std::atomic<bool> fShouldDeregister = false;
};

#endif
