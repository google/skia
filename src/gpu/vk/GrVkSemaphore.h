/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSemaphore_DEFINED
#define GrVkSemaphore_DEFINED

#include "GrSemaphore.h"

#include "GrResourceProvider.h"
#include "GrVkResource.h"

#include "vk/GrVkTypes.h"

class GrBackendSemaphore;
class GrVkGpu;

class GrVkSemaphore : public GrSemaphore {
public:
    static sk_sp<GrVkSemaphore> Make(const GrVkGpu* gpu, bool isOwned);

    using WrapType = GrResourceProvider::SemaphoreWrapType;

    static sk_sp<GrVkSemaphore> MakeWrapped(const GrVkGpu* gpu,
                                            VkSemaphore semaphore,
                                            WrapType wrapType,
                                            GrWrapOwnership);

    ~GrVkSemaphore() override;

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

        static void AcquireMutex() { gMutex.acquire(); }
        static void ReleaseMutex() { gMutex.release(); }

        bool shouldSignal() const {
            return !fHasBeenSubmittedToQueueForSignal;
        }
        bool shouldWait() const {
            return !fHasBeenSubmittedToQueueForWait;
        }

        void markAsSignaled() {
            gMutex.assertHeld();
            fHasBeenSubmittedToQueueForSignal = true;
        }
        void markAsWaited() {
            gMutex.assertHeld();
            fHasBeenSubmittedToQueueForWait = true;
        }

#ifdef SK_TRACE_VK_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkSemaphore: %d (%d refs)\n", fSemaphore, this->getRefCnt());
        }
#endif
    private:
        void freeGPUData(const GrVkGpu* gpu) const override;

        static SkMutex gMutex;
        VkSemaphore fSemaphore;
        bool        fHasBeenSubmittedToQueueForSignal;
        bool        fHasBeenSubmittedToQueueForWait;
        bool        fIsOwned;

        typedef GrVkResource INHERITED;
    };

    Resource* getResource() { return fResource; }

private:
    GrVkSemaphore(const GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal, bool prohibitWait,
                  bool isOwned);

    void setBackendSemaphore(GrBackendSemaphore*) const override;

    Resource* fResource;

    typedef GrSemaphore INHERITED;
};

#endif
