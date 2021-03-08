/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDDLTask_DEFINED
#define GrDDLTask_DEFINED

#include "include/core/SkPoint.h"
#include "src/gpu/GrRenderTask.h"

class GrRenderTargetProxy;

/**
 * This render task isolates the DDL's tasks from the rest of the DAG. This means that
 * the DDL's tasks cannot be reordered by the topological sort and are always executed
 * as a single block.
 * It almost entirely just forwards calls down to the DDL's render tasks.
 */
class GrDDLTask final : public GrRenderTask {
public:
    GrDDLTask(GrDrawingManager*,
              sk_sp<GrRenderTargetProxy> ddlTarget,
              sk_sp<const SkDeferredDisplayList>,
              SkIPoint offset);

    ~GrDDLTask() override;

    // The render tasks w/in the DDL don't appear in the DAG so need explicit notification
    // when they can free their contents.
    bool requiresExplicitCleanup() const override { return true; }

    void endFlush(GrDrawingManager*) override;

    void disown(GrDrawingManager*) override;

private:
    bool onIsUsed(GrSurfaceProxy*) const override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect* targetUpdateBounds) override;

    void gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const override;

    void onPrePrepare(GrRecordingContext*) override {
        // This entry point is only called when a DDL is snapped off of a recorder.
        // Since DDL tasks should never recursively appear within a DDL this should never
        // be called.
        SkASSERT(0);
    }

    void onPrepare(GrOpFlushState*) override;

    bool onExecute(GrOpFlushState*) override;

#if GR_TEST_UTILS
    void dump(const SkString& label,
              SkString indent,
              bool printDependencies,
              bool close) const final;
    const char* name() const final { return "DDL"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
    sk_sp<GrRenderTargetProxy>         fDDLTarget;
    SkIPoint                           fOffset;

    typedef GrRenderTask INHERITED;
};

#endif
