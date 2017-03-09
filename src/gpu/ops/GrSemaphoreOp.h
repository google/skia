/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphoreOp_DEFINED
#define GrSemaphoreOp_DEFINED

#include "GrOp.h"

#include "GrSemaphore.h"
#include "SkRefCnt.h"

class GrSemaphoreOp : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrSemaphoreOp> MakeSignal(sk_sp<GrSemaphore> semaphore);

    static std::unique_ptr<GrSemaphoreOp> MakeWait(sk_sp<GrSemaphore> semaphore);

    SkString dumpInfo() const override {
        return INHERITED::dumpInfo();
    }

protected:
    GrSemaphoreOp(uint32_t classId, sk_sp<GrSemaphore> semaphore)
        : INHERITED(classId), fSemaphore(semaphore) {}

    sk_sp<GrSemaphore> fSemaphore;

private:
    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}

    typedef GrOp INHERITED;
};

#endif
