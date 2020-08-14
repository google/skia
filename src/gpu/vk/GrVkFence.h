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
#include "src/gpu/vk/GrVkManagedResource.h"

class GrVkGpu;

class GrVkFence : public GrVkRecycledResource {
public:
    static GrVkFence* Create(GrVkGpu* gpu);
    ~GrVkFence() override = default;

    VkFence fence() const { return fFence; }

    bool isSignaled(GrVkGpu* gpu);

    void wait(GrVkGpu* gpu);

    void reset(GrVkGpu* gpu);

private:
#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFence: %d (%d refs)\n", fFence, this->getRefCnt());
    }
#endif
    void freeGPUData() const override;
    void onRecycle() const override;

    GrVkFence(GrVkGpu* gpu, VkFence fence) : GrVkRecycledResource(gpu), fFence(fence) {}

    const VkFence fFence;

    typedef GrVkRecycledResource INHERITED;
};

#endif
