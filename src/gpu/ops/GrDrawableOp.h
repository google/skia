/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawableOp_DEFINED
#define GrDrawableOp_DEFINED

#include "GrOp.h"

#include "GrSemaphore.h"
#include "SkMatrix.h"

class SkDrawable;

class GrDrawableOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawableOp> Make(SkDrawable* drawable,
                                              const SkMatrix& matrix,
                                              sk_sp<GrSemaphore> semaphoreToWaitOn,
                                              sk_sp<GrSemaphore> semaphoreToSignal) {
        return std::unique_ptr<GrDrawableOp>(new GrDrawableOp(drawable, matrix,
                                                              std::move(semaphoreToWaitOn),
                                                              std::move(semaphoreToSignal)));
    }

    const char* name() const override { return "Drawable"; }

    SkString dumpInfo() const override {
        return INHERITED::dumpInfo();
    }

private:
    GrDrawableOp(SkDrawable*, const SkMatrix&,
                 sk_sp<GrSemaphore> semaphoreToWaitOn,
                 sk_sp<GrSemaphore> semaphoreToSignal);

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override;

    sk_sp<SkDrawable> fDrawable;
    SkMatrix fMatrix;
    sk_sp<GrSemaphore> fSemaphoreToWaitOn;
    sk_sp<GrSemaphore> fSemaphoreToSignal;

    typedef GrOp INHERITED;
};

#endif

