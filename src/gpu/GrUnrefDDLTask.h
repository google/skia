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

    GrUnrefDDLTask(GrDrawingManager* drawingMgr,
                   sk_sp<const SkDeferredDisplayList> ddl, int xOffset, int yOffset)
            : GrRenderTask()
            , fDDL(std::move(ddl))
            , fXOffset(xOffset)
            , fYOffset(yOffset)
            , fNumTargets(0) {
        for (auto& task : fDDL->priv().renderTasks()) {
            for (int i = 0; i < task->numTargets1(); ++i) {
                this->addTarget(drawingMgr, task->target(i));
            }
        }
    }

    ~GrUnrefDDLTask() override {
    }

    // The render tasks w/in the DDL don't appear in the DAG so need explicit notification
    // when they can free their contents.
    bool requiresExplicitCleanup() const override { return true; }

    void endFlush(GrDrawingManager* drawingManager) override {
        SkDebugf("GrUnrefDDLTask: ending w/ %d child tasks\n", fDDL->priv().numRenderTasks());

        for (auto& task : fDDL->priv().renderTasks()) {
            task->endFlush(drawingManager);
        }

        GrRenderTask::endFlush(drawingManager);
    }

    void disown(GrDrawingManager* drawingManager) override {
        for (auto& task : fDDL->priv().renderTasks()) {
            task->disown(drawingManager);
        }

        INHERITED::disown(drawingManager);
    }

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        for (auto& task : fDDL->priv().renderTasks()) {
            if (task->onIsUsed(proxy)) {
                return true;
            }
        }

        return false;
    }

    void handleInternalAllocationFailure() override {}

    void gatherProxyIntervals(GrResourceAllocator* alloc) const override {
        // We don't have any proxies, but the resource allocator will still bark
        // if a task doesn't claim any op indices, so we oblige it.
        alloc->incOps();

        for (auto& task : fDDL->priv().renderTasks()) {
            task->gatherProxyIntervals(alloc);
        }
    }

    ExpectedOutcome onMakeClosed(const GrCaps& caps, SkIRect* targetUpdateBounds) override {
        for (auto& task : fDDL->priv().renderTasks()) {
            SkASSERT(task->isClosed());
        }

        return ExpectedOutcome::kTargetUnchanged;
    }

//    void onPrePrepare(GrRecordingContext*) override {
//    }

    void onPrepare(GrOpFlushState* flushState) override {
        for (auto& task : fDDL->priv().renderTasks()) {
            task->prepare(flushState);
        }
    }

    bool onExecute(GrOpFlushState* flushState) override {
        bool anyCommandsIssued = false;
        for (auto& task : fDDL->priv().renderTasks()) {
            if (task->execute1(flushState)) {
                anyCommandsIssued = true;
            }
        }
        return anyCommandsIssued;
    }

#ifdef SK_DEBUG
    const char* name() const final { return "UnrefDDL"; }
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    sk_sp<const SkDeferredDisplayList> fDDL;
    int fXOffset;
    int fYOffset;
    int fNumTargets;

    typedef GrRenderTask INHERITED;
};

#endif
