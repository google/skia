/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkFence_DEFINED
#define GrVkFence_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/GrVkTypes.h"

class GrVkGpu;

class GrVkFence : public SkRefCnt {
public:
    static sk_sp<GrVkFence> Make(GrVkGpu* gpu, bool isSignaled = false);

    ~GrVkFence();

    VkFence fence() { return fFence; }

    bool isSignaled();

    void wait();

    void reset();

private:
    GrVkFence(GrVkGpu* gpu, VkFence fence);

    GrVkGpu* const fGpu;
    const VkFence fFence;
};

#endif
