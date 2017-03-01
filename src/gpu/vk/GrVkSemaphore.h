/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSemaphore_DEFINED
#define GrVkSemaphore_DEFINED

#include "GrSemaphore.h"
#include "GrVkResource.h"

#include "vk/GrVkTypes.h"

class GrVkGpu;

class GrVkSemaphore : public GrSemaphore {
public:
    static sk_sp<GrVkSemaphore> Make(const GrVkGpu* gpu);

    ~GrVkSemaphore() override;

    class Resource : public GrVkResource {
    public:
        Resource(VkSemaphore semaphore) : INHERITED(), fSemaphore(semaphore) {}

        ~Resource() override {}

        VkSemaphore semaphore() const { return fSemaphore; }

#ifdef SK_TRACE_VK_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkSemaphore: %d (%d refs)\n", fSemaphore, this->getRefCnt());
        }
#endif
    private:
        void freeGPUData(const GrVkGpu* gpu) const override;

        VkSemaphore fSemaphore;

        typedef GrVkResource INHERITED;
    };

    const Resource* getResource() const { return fResource; }

private:
    GrVkSemaphore(const GrVkGpu* gpu, VkSemaphore semaphore);

    const Resource* fResource;

    typedef GrSemaphore INHERITED;
};

#endif
