/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockRenderTask_DEFINED
#define GrMockRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrMockRenderTask : public GrRenderTask {
public:
    GrMockRenderTask() : GrRenderTask() {
        // Mock tasks are never "owned" by a drawmgr in the first place.
        this->setFlag(kDisowned_Flag);
    }

    void addTarget(GrSurfaceProxyView view) { fTargets.push_back(view.detachProxy()); }
    void addDependency(GrRenderTask* dep) { fDependencies.push_back(dep); }

    // Overrides.
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc&) const override { return; }
#endif
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override {}
    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect*) override { SkUNREACHABLE; }
    bool onIsUsed(GrSurfaceProxy*) const override { return false; }
    bool onExecute(GrOpFlushState*) override { return true; }

#if GR_TEST_UTILS
    const char* name() const final { return "Mock"; }
#endif
};

#endif
