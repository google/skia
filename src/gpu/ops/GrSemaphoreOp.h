/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphoreOp_DEFINED
#define GrSemaphoreOp_DEFINED

#include "GrOp.h"

#include "GrRenderTargetProxy.h"
#include "GrSemaphore.h"
#include "SkRefCnt.h"

class GrRecordingContext;

class GrSemaphoreOp : public GrOp {
public:
    static std::unique_ptr<GrOp> MakeWait(GrRecordingContext*,
                                          GrRenderTargetProxy*,
                                          const GrBackendSemaphore&);

protected:
    GrSemaphoreOp(uint32_t classId, GrRenderTargetProxy* proxy, const GrBackendSemaphore& sema) {
            : INHERITED(classId)
            , fSemaphore(sema) {
        this->makeFullScreen(proxy);
    }

private:
    void onPrepare(GrOpFlushState*) override {}

    GrBackendSemaphore fSemaphore;

    typedef GrOp INHERITED;
};

#endif
