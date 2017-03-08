/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphoreOp_DEFINED
#define GrSemaphoreOp_DEFINED

#include "GrOp.h"

#include "GrGpu.h"
#include "GrSemaphore.h"
#include "SkRefCnt.h"

class GrSemaphoreOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrSemaphoreOp> MakeInsert(sk_sp<GrSemaphore> semaphore) {
        return std::unique_ptr<GrSemaphoreOp>(new GrSemaphoreOp(semaphore, Use::kInsert));
    }

    static std::unique_ptr<GrSemaphoreOp> MakeWait(sk_sp<GrSemaphore> semaphore) {
        return std::unique_ptr<GrSemaphoreOp>(new GrSemaphoreOp(semaphore, Use::kWait));
    }

    const char* name() const override { return "Semaphore"; }

    SkString dumpInfo() const override {
        return INHERITED::dumpInfo();
    }

private:
    enum class Use {
        kInsert,
        kWait,
    };

    GrSemaphoreOp(sk_sp<GrSemaphore> semaphore, Use use)
        : INHERITED(ClassID()), fSemaphore(semaphore), fUse(use) {}

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        if (Use::kInsert == fUse) {
            state->gpu()->insertSemaphore(fSemaphore);
        } else {
            SkASSERT(Use::kWait == fUse);
            state->gpu()->waitSemaphore(fSemaphore);
        }
    }

    sk_sp<GrSemaphore> fSemaphore;
    Use                fUse;

    typedef GrOp INHERITED;
};

#endif
