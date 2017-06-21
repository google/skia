/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSemaphoreOp.h"

#include "GrGpu.h"
#include "GrOpFlushState.h"

class GrSignalSemaphoreOp final : public GrSemaphoreOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrSignalSemaphoreOp> Make(sk_sp<GrSemaphore> semaphore,
                                                     GrRenderTargetProxy* proxy,
                                                     bool forceFlush) {
        return std::unique_ptr<GrSignalSemaphoreOp>(new GrSignalSemaphoreOp(std::move(semaphore),
                                                                            proxy,
                                                                            forceFlush));
    }

    const char* name() const override { return "SignalSemaphore"; }

private:
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

    static std::unique_ptr<GrWaitSemaphoreOp> Make(sk_sp<GrSemaphore> semaphore,
                                                   GrRenderTargetProxy* proxy) {
        return std::unique_ptr<GrWaitSemaphoreOp>(new GrWaitSemaphoreOp(std::move(semaphore),
                                                                        proxy));
    }

    const char* name() const override { return "WaitSemaphore"; }

private:
    explicit GrWaitSemaphoreOp(sk_sp<GrSemaphore> semaphore, GrRenderTargetProxy* proxy)
            : INHERITED(ClassID(), std::move(semaphore), proxy) {}

    void onExecute(GrOpFlushState* state) override {
        state->gpu()->waitSemaphore(fSemaphore);
    }

    typedef GrSemaphoreOp INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrSemaphoreOp> GrSemaphoreOp::MakeSignal(sk_sp<GrSemaphore> semaphore,
                                                         GrRenderTargetProxy* proxy,
                                                         bool forceFlush) {
    return GrSignalSemaphoreOp::Make(std::move(semaphore), proxy, forceFlush);
}

std::unique_ptr<GrSemaphoreOp> GrSemaphoreOp::MakeWait(sk_sp<GrSemaphore> semaphore,
                                                       GrRenderTargetProxy* proxy) {
    return GrWaitSemaphoreOp::Make(std::move(semaphore), proxy);
}


