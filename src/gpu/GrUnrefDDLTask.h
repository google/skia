/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUnrefDDLTask_DEFINED
#define GrUnrefDDLTask_DEFINED

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

    // We actually do the unreffing in dtor instead of onExecute, so that we maintain the invariant
    // that DDLs are always the last owners of their render tasks (because those tasks depend on
    // memory owned by the DDL.) If we had this in onExecute, the tasks would still be alive in
    // the drawing manager although it would already have executed them.
    ~GrUnrefDDLTask() override {
        fDDL.reset();
    }

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

    bool onExecute(GrOpFlushState*) override { return true; }

#ifdef SK_DEBUG
    const char* name() const final { return "UnrefDDL"; }
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
};

#endif
