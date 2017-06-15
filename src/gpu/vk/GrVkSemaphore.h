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

class GrBackendSemaphore;
class GrVkGpu;

class GrVkSemaphore : public GrSemaphore {
public:
    static sk_sp<GrVkSemaphore> Make(const GrVkGpu* gpu, bool isOwned);

    static sk_sp<GrVkSemaphore> MakeWrapped(const GrVkGpu* gpu,
                                            VkSemaphore semaphore,
                                            GrWrapOwnership);

    ~GrVkSemaphore() override;

    class Resource : public GrVkResource {
    public:
        Resource(VkSemaphore semaphore, bool isOwned)
                : INHERITED(), fSemaphore(semaphore), fIsOwned(isOwned) {}

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
        bool        fIsOwned;

        typedef GrVkResource INHERITED;
    };

    const Resource* getResource() const { return fResource; }

private:
    GrVkSemaphore(const GrVkGpu* gpu, VkSemaphore semaphore, bool isOwned);

    void setBackendSemaphore(GrBackendSemaphore*) const override;

    const Resource* fResource;

    typedef GrSemaphore INHERITED;
};

#endif
