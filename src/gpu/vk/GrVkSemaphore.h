/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSemaphore_DEFINED
#define GrVkSemaphore_DEFINED

#include "src/gpu/GrSemaphore.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/vk/GrVkResource.h"

class GrBackendSemaphore;
class GrVkGpu;

class GrVkSemaphore : public GrSemaphore {
public:
    static sk_sp<GrVkSemaphore> Make(GrVkGpu* gpu, bool isOwned);

    using WrapType = GrResourceProvider::SemaphoreWrapType;

    static sk_sp<GrVkSemaphore> MakeWrapped(GrVkGpu* gpu,
                                            VkSemaphore semaphore,
                                            WrapType wrapType,
                                            GrWrapOwnership);

    GrBackendSemaphore backendSemaphore() const override;

    class Resource : public GrVkResource {
    public:
        Resource(VkSemaphore semaphore, bool prohibitSignal, bool prohibitWait, bool isOwned)
                : INHERITED()
                , fSemaphore(semaphore)
                , fHasBeenSubmittedToQueueForSignal(prohibitSignal)
                , fHasBeenSubmittedToQueueForWait(prohibitWait)
                , fIsOwned(isOwned) {}

        ~Resource() override {}

        VkSemaphore semaphore() const { return fSemaphore; }

        bool shouldSignal() const {
            return !fHasBeenSubmittedToQueueForSignal;
        }
        bool shouldWait() const {
            return !fHasBeenSubmittedToQueueForWait;
        }

        void markAsSignaled() {
            fHasBeenSubmittedToQueueForSignal = true;
        }
        void markAsWaited() {
            fHasBeenSubmittedToQueueForWait = true;
        }

#ifdef SK_TRACE_VK_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkSemaphore: %d (%d refs)\n", fSemaphore, this->getRefCnt());
        }
#endif
    private:
        void freeGPUData(GrVkGpu* gpu) const override;

        VkSemaphore fSemaphore;
        bool        fHasBeenSubmittedToQueueForSignal;
        bool        fHasBeenSubmittedToQueueForWait;
        bool        fIsOwned;

        typedef GrVkResource INHERITED;
    };

    Resource* getResource() { return fResource; }

private:
    GrVkSemaphore(GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal, bool prohibitWait,
                  bool isOwned);

    void onRelease() override;
    void onAbandon() override;

    Resource* fResource;

    typedef GrSemaphore INHERITED;
};

#endif
