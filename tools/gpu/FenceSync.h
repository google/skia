/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FenceSync_DEFINED
#define FenceSync_DEFINED

#include "SkTypes.h"

namespace sk_gpu_test {

using PlatformFence = uint64_t;
static constexpr PlatformFence kInvalidFence = 0;

/*
 * This class provides an interface to interact with fence syncs. A fence sync is an object that the
 * client can insert into the GPU command stream, and then at any future time, wait until all
 * commands that were issued before the fence have completed.
 */
class FenceSync {
public:
    virtual PlatformFence SK_WARN_UNUSED_RESULT insertFence() const = 0;
    virtual bool waitFence(PlatformFence) const = 0;
    virtual void deleteFence(PlatformFence) const = 0;

    virtual ~FenceSync() {}
};

}  // namespace sk_gpu_test

#endif
