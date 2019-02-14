/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphoreOp_DEFINED
#define GrSemaphoreOp_DEFINED


#include "GrBackendSemaphore.h"
#include "GrOp.h"
#include "GrRenderTargetProxy.h"
#include "SkRefCnt.h"

class GrRecordingContext;
class GrSemaphore;

class GrSemaphoreOp : public GrOp {
public:
    ~GrSemaphoreOp() override;

    using SemaphoreContext = void*;
    using SemaphoreDoneProc = void (*)(SemaphoreContext, const GrBackendSemaphore&);

    static std::unique_ptr<GrOp> MakeWait(GrRecordingContext*,
                                          GrRenderTargetProxy*,
                                          const GrBackendSemaphore&,
                                          SemaphoreDoneProc doneProc,
                                          SemaphoreContext doneContext);

protected:
    GrSemaphoreOp(uint32_t classId, GrRenderTargetProxy* proxy, const GrBackendSemaphore& sema,
                  SemaphoreDoneProc doneProc, SemaphoreContext doneContext)
            : INHERITED(classId)
            , fBackendSemaphore(sema)
            , fDoneProc(doneProc)
            , fDoneContext(doneContext) {
        this->makeFullScreen(proxy);
    }

    void onPrepare(GrOpFlushState*) override;

    sk_sp<GrSemaphore> fSemaphore;
    GrBackendSemaphore fBackendSemaphore;
    SemaphoreDoneProc fDoneProc;
    SemaphoreContext fDoneContext;

private:
    typedef GrOp INHERITED;
};

#endif
