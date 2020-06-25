/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUnrefDDLTask_DEFINED
#define GrUnrefDDLTask_DEFINED

#ifndef SK_DDL_IS_UNIQUE_POINTER

#include "src/gpu/GrRenderTask.h"

/** When a DDL is played back, the drawing manager refs the DDL and adds one
 *  of these to the DAG to release it after the flush. Thus the user is free
 *  to unref the DDL at their leisure without messing us up.
 */
class GrUnrefDDLTask final : public GrRenderTask {
public:
    GrUnrefDDLTask(sk_sp<const SkDeferredDisplayList> ddl)
            : GrRenderTask()
            , fDDL(std::move(ddl)) {}

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override { return false; }
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator* alloc) const override {
        // We don't have any proxies, but the resource allocator will still bark
        // if a task doesn't claim any op indices, so we oblige it.
        alloc->incOps();
    }

    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override {
        fDDL.reset();
        return true;
    }

#ifdef SK_DEBUG
    const char* name() const final { return "UnrefDDL"; }
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
};

#endif  // !SK_DDL_IS_UNIQUE_POINTER
#endif
