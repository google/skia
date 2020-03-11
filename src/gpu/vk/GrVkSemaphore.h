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
#include "src/gpu/vk/GrVkManagedResource.h"

class GrBackendSemaphore;
class GrVkGpu;

class GrVkSemaphore : public GrSemaphore {
public:
    static std::unique_ptr<GrVkSemaphore> Make(GrVkGpu* gpu, bool isOwned);

    using WrapType = GrResourceProvider::SemaphoreWrapType;

    static std::unique_ptr<GrVkSemaphore> MakeWrapped(GrVkGpu* gpu,
                                                      VkSemaphore semaphore,
                                                      WrapType wrapType,
                                                      GrWrapOwnership);

    ~GrVkSemaphore() override;

    GrBackendSemaphore backendSemaphore() const override;

    class Resource : public GrVkManagedResource {
    public:
        Resource(const GrVkGpu* gpu, VkSemaphore semaphore,
                 bool prohibitSignal, bool prohibitWait, bool isOwned)
                : INHERITED(gpu)
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

        void setIsOwned() {
            fIsOwned = true;
        }

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkSemaphore: %d (%d refs)\n", fSemaphore, this->getRefCnt());
        }
#endif
    private:
        void freeGPUData() const override;

        VkSemaphore fSemaphore;
        bool        fHasBeenSubmittedToQueueForSignal;
        bool        fHasBeenSubmittedToQueueForWait;
        bool        fIsOwned;

        typedef GrVkManagedResource INHERITED;
    };

    Resource* getResource() { return fResource; }

private:
    GrVkSemaphore(GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal, bool prohibitWait,
                  bool isOwned);

    void setIsOwned() override {
        fResource->setIsOwned();
    }

    Resource* fResource;

    typedef GrSemaphore INHERITED;
};

#endif
