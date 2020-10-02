/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUnrefDDLTask_DEFINED
#define GrUnrefDDLTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrRenderTargetProxy;

/** When a DDL is played back, the drawing manager refs the DDL and adds one
 *  of these to the DAG to release it after the flush. Thus the user is free
 *  to unref the DDL at their leisure without messing us up.
 */
class GrUnrefDDLTask final : public GrRenderTask {
public:
    GrUnrefDDLTask(GrDrawingManager*,
                   sk_sp<GrRenderTargetProxy> ddlTarget,
                   sk_sp<const SkDeferredDisplayList>,
                   int xOffset, int yOffset);

    ~GrUnrefDDLTask() override;

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

//    void onPrePrepare(GrRecordingContext*) override {
//    }

    void onPrepare(GrOpFlushState* flushState) override;

    bool onExecute(GrOpFlushState* flushState) override;

#if GR_TEST_UTILS
    const char* name() const final { return "UnrefDDL"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
    sk_sp<GrRenderTargetProxy>         fDDLTarget;
    int fXOffset;
    int fYOffset;

    typedef GrRenderTask INHERITED;
};

#endif
