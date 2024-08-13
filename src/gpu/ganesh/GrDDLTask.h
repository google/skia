/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDDLTask_DEFINED
#define GrDDLTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrRenderTask.h"

class GrDeferredDisplayList;
class GrDrawingManager;
class GrOpFlushState;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrResourceAllocator;
class GrSurfaceProxy;
class SkString;
struct SkIRect;

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
              sk_sp<const GrDeferredDisplayList>);

    ~GrDDLTask() override;

    // The render tasks w/in the DDL don't appear in the DAG so need explicit notification
    // when they can free their contents.
    bool requiresExplicitCleanup() const override { return true; }

    void endFlush(GrDrawingManager*) override;

    void disown(GrDrawingManager*) override;

private:
    bool onIsUsed(GrSurfaceProxy*) const override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;

    void onPrePrepare(GrRecordingContext*) override {
        // This entry point is only called when a DDL is snapped off of a recorder.
        // Since DDL tasks should never recursively appear within a DDL this should never
        // be called.
        SkASSERT(0);
    }

    void onPrepare(GrOpFlushState*) override;

    bool onExecute(GrOpFlushState*) override;

#if defined(GPU_TEST_UTILS)
    void dump(const SkString& label,
              SkString indent,
              bool printDependencies,
              bool close) const final;
    const char* name() const final { return "DDL"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override {}
#endif

    sk_sp<const GrDeferredDisplayList> fDDL;
    sk_sp<GrRenderTargetProxy>         fDDLTarget;

    typedef GrRenderTask INHERITED;
};

#endif
