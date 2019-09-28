/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphoreOp_DEFINED
#define GrSemaphoreOp_DEFINED

#include "src/gpu/ops/GrOp.h"

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSemaphore.h"

class GrRecordingContext;

class GrSemaphoreOp : public GrOp {
public:
    static std::unique_ptr<GrOp> MakeWait(GrRecordingContext*,
                                          sk_sp<GrSemaphore>,
                                          GrRenderTargetProxy*);

protected:
    GrSemaphoreOp(uint32_t classId, sk_sp<GrSemaphore> semaphore, GrRenderTargetProxy* proxy)
            : INHERITED(classId)
            , fSemaphore(std::move(semaphore)) {
        this->makeFullScreen(proxy);
    }

    sk_sp<GrSemaphore> fSemaphore;

private:
    void onPrepare(GrOpFlushState*) override {}

    typedef GrOp INHERITED;
};

#endif
