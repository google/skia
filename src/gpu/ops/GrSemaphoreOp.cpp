/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSemaphoreOp.h"

#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrSemaphore.h"

class GrWaitSemaphoreOp final : public GrSemaphoreOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRecordingContext* context,
                                      GrRenderTargetProxy* proxy,
                                      const GrBackendSemaphore& sema,
                                      SemaphoreDoneProc doneProc,
                                      SemaphoreContext doneContext) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<GrWaitSemaphoreOp>(proxy, sema, doneProc, doneContext);
    }

    const char* name() const override { return "WaitSemaphore"; }

private:
    friend class GrOpMemoryPool; // for ctor

    explicit GrWaitSemaphoreOp(GrRenderTargetProxy* proxy, const GrBackendSemaphore& sema,
                               SemaphoreDoneProc doneProc, SemaphoreContext doneContext)
            : INHERITED(ClassID(), proxy, sema, doneProc, doneContext) {}

    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        state->gpu()->waitSemaphore(fSemaphore);
    }

    typedef GrSemaphoreOp INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrOp> GrSemaphoreOp::MakeWait(GrRecordingContext* context,
                                              GrRenderTargetProxy* proxy,
                                              const GrBackendSemaphore& sema,
                                              SemaphoreDoneProc doneProc,
                                              SemaphoreContext doneContext) {
    return GrWaitSemaphoreOp::Make(context, proxy, sema, doneProc, doneContext);
}

GrSemaphoreOp::~GrSemaphoreOp() {
    SkASSERT(fBackendSemaphore.isInitialized() != SkToBool(fSemaphore));
    if (!fSemaphore) {
        // If we never created the GrSemaphore object we need to manually clean up here
        fDoneProc(fDoneContext, fBackendSemaphore);
    }
}


void GrSemaphoreOp::onPrepare(GrOpFlushState* opFlushState) {
    if (fSemaphore) {
        return;  // we've already created the GrSemaphore object during a prior execution
    }

    auto resourceProvider = opFlushState->resourceProvider();

    fSemaphore = resourceProvider->wrapBackendSemaphore(
                        fBackendSemaphore,
                        GrResourceProvider::SemaphoreWrapType::kWillWait,
                        fDoneProc,
                        fDoneContext);
    fBackendSemaphore.reset();
}

