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

    void addTarget(sk_sp<GrSurfaceProxy> proxy) { fTargets.push_back(std::move(proxy)); }
    void addDependency(GrRenderTask* dep) { fDependencies.push_back(dep); }
    void addUsed(sk_sp<GrSurfaceProxy> proxy) { fUsed.push_back(std::move(proxy)); }

    // Overrides.
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override { return; }
#endif
    void gatherProxyIntervals(GrResourceAllocator*) const override {}
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect*) override { SkUNREACHABLE; }
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        for (const auto& entry : fUsed) {
            if (entry.get() == proxy) {
                return true;
            }
        }
        return false;
    }
    bool onExecute(GrOpFlushState*) override { return true; }

#if GR_TEST_UTILS
    const char* name() const final { return "Mock"; }
#endif

private:
    SkTArray<sk_sp<GrSurfaceProxy>> fUsed;
};

#endif
