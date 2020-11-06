/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDDLTask_DEFINED
#define GrDDLTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrRenderTargetProxy;

/**
 * This render task isolates the DDL's tasks from the rest of the DAG. This means that
 * the DDL's tasks cannot be reordered by the topological sort and are always executed
 * as a single block.
 */
class GrDDLTask final : public GrRenderTask {
public:
    GrDDLTask(GrDrawingManager*,
              sk_sp<GrRenderTargetProxy> ddlTarget,
              sk_sp<const SkDeferredDisplayList>);

    ~GrDDLTask() override;

    // The render tasks w/in the DDL don't appear in the DAG so need explicit notification
    // when they can free their contents.
    bool requiresExplicitCleanup() const override { return true; }

    void endFlush(GrDrawingManager* drawingManager) override;

    void disown(GrDrawingManager* drawingManager) override;

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override;

    void handleInternalAllocationFailure() override {}

    void gatherProxyIntervals(GrResourceAllocator* alloc) const override;

    ExpectedOutcome onMakeClosed(const GrCaps& caps, SkIRect* targetUpdateBounds) override;

    void gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const override;

    void onPrePrepare(GrRecordingContext*) override {
        // This entry point is only called when a DDL is created. There should never
        // be DDLTasks w/in a DDL.
        SkASSERT(0);
    }

    void onPrepare(GrOpFlushState* flushState) override;

    bool onExecute(GrOpFlushState* flushState) override;

#if GR_TEST_UTILS
    const char* name() const final { return "DDL"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
    sk_sp<GrRenderTargetProxy>         fDDLTarget;

    typedef GrRenderTask INHERITED;
};

#endif
