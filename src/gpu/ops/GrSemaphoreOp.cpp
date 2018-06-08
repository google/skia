/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSemaphoreOp.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrOpFlushState.h"

class GrSignalSemaphoreOp final : public GrSemaphoreOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrContext* context,
                                      sk_sp<GrSemaphore> semaphore,
                                      GrRenderTargetProxy* proxy,
                                      bool forceFlush) {
        // $$
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

#if 1
        return pool->allocate<GrSignalSemaphoreOp>(std::move(semaphore),
                                                   proxy,
                                                   forceFlush);
#else
        char* mem = (char*) pool->allocate(sizeof(GrSignalSemaphoreOp));
        return std::unique_ptr<GrOp>(new (mem) GrSignalSemaphoreOp(std::move(semaphore),
                                                                   proxy,
                                                                   forceFlush));
#endif
    }

    const char* name() const override { return "SignalSemaphore"; }

private:
    friend class GrOpMemoryPool; // for ctor

    explicit GrSignalSemaphoreOp(sk_sp<GrSemaphore> semaphore, GrRenderTargetProxy* proxy,
                                 bool forceFlush)
            : INHERITED(ClassID(), std::move(semaphore), proxy), fForceFlush(forceFlush) {}

    void onExecute(GrOpFlushState* state) override {
        state->gpu()->insertSemaphore(fSemaphore, fForceFlush);
    }

    bool fForceFlush;

    typedef GrSemaphoreOp INHERITED;
};

class GrWaitSemaphoreOp final : public GrSemaphoreOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrContext* context,
                                      sk_sp<GrSemaphore> semaphore,
                                      GrRenderTargetProxy* proxy) {
        // $$
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
#if 0
        char* mem = (char*) pool->allocate(sizeof(GrWaitSemaphoreOp));
        return std::unique_ptr<GrOp>(new (mem) GrWaitSemaphoreOp(std::move(semaphore),
                                                                 proxy));
#else
        return pool->allocate<GrWaitSemaphoreOp>(std::move(semaphore), proxy);
#endif
    }

    const char* name() const override { return "WaitSemaphore"; }

private:
    friend class GrOpMemoryPool; // for ctor

    explicit GrWaitSemaphoreOp(sk_sp<GrSemaphore> semaphore, GrRenderTargetProxy* proxy)
            : INHERITED(ClassID(), std::move(semaphore), proxy) {}

    void onExecute(GrOpFlushState* state) override {
        state->gpu()->waitSemaphore(fSemaphore);
    }

    typedef GrSemaphoreOp INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrOp> GrSemaphoreOp::MakeSignal(GrContext* context,
                                                sk_sp<GrSemaphore> semaphore,
                                                GrRenderTargetProxy* proxy,
                                                bool forceFlush) {
    return GrSignalSemaphoreOp::Make(context, std::move(semaphore), proxy, forceFlush);
}

std::unique_ptr<GrOp> GrSemaphoreOp::MakeWait(GrContext* context,
                                              sk_sp<GrSemaphore> semaphore,
                                              GrRenderTargetProxy* proxy) {
    return GrWaitSemaphoreOp::Make(context, std::move(semaphore), proxy);
}


