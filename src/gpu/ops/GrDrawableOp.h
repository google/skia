/*
 * Copyright 2018 Google Inc.
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

    static std::unique_ptr<GrDrawableOp> Make(GrContext*, SkDrawable* drawable,
                                              const SkMatrix& matrix);

    const char* name() const override { return "Drawable"; }

    SkString dumpInfo() const override {
        return INHERITED::dumpInfo();
    }

private:
    friend class GrOpMemoryPool; // for ctor

    GrDrawableOp(SkDrawable*, const SkMatrix&);

    CombineResult onCombineIfPossible(GrOp* that, const GrCaps& caps) override {
        return CombineResult::kCannotCombine;
    }
    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override;

    sk_sp<SkDrawable> fDrawable;
    SkMatrix fMatrix;

    typedef GrOp INHERITED;
};

#endif

