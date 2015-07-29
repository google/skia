
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGpuFenceSync_DEFINED
#define SkGpuFenceSync_DEFINED

#include "SkTypes.h"

typedef void* SkPlatformGpuFence;

/*
 * This class provides an interface to interact with fence syncs. A fence sync is an object that the
 * client can insert into the GPU command stream, and then at any future time, wait until all
 * commands that were issued before the fence have completed.
 */
class SkGpuFenceSync {
public:
    virtual SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const = 0;
    virtual bool flushAndWaitFence(SkPlatformGpuFence) const = 0;
    virtual void deleteFence(SkPlatformGpuFence) const = 0;

    virtual ~SkGpuFenceSync() {}
};

#endif
