/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSemaphore_DEFINED
#define GrGLSemaphore_DEFINED

#include "GrBackendSemaphore.h"
#include "GrSemaphore.h"
#include "GrTypesPriv.h"

class GrGLGpu;

class GrGLSemaphore : public GrSemaphore {
public:
    static sk_sp<GrGLSemaphore> Make(GrGLGpu* gpu,
                                     GrResourceProvider::SemaphoreDoneProc doneProc,
                                     GrResourceProvider::SemaphoreContext doneContext) {
        return sk_sp<GrGLSemaphore>(new GrGLSemaphore(gpu, doneProc, doneContext));
    }

    static sk_sp<GrGLSemaphore> MakeWrapped(GrGLGpu* gpu,
                                            GrGLsync sync,
                                            GrResourceProvider::SemaphoreDoneProc doneProc,
                                            GrResourceProvider::SemaphoreContext doneContext) {
        auto sema = sk_sp<GrGLSemaphore>(new GrGLSemaphore(gpu, doneProc, doneContext));
        sema->setSync(sync);
        return sema;
    }

    GrGLsync sync() const { return fSync; }
    void setSync(const GrGLsync& sync) { fSync = sync; }

    GrBackendSemaphore backendSemaphore() const override {
        GrBackendSemaphore backendSemaphore;
        backendSemaphore.initGL(fSync);
        return backendSemaphore;
    }

private:
    GrGLSemaphore(GrGLGpu* gpu,
                  GrResourceProvider::SemaphoreDoneProc doneProc,
                  GrResourceProvider::SemaphoreContext doneContext);

    void onRelease() override;
    void onAbandon() override;

    GrGLsync fSync;
    GrResourceProvider::SemaphoreDoneProc fDoneProc;
    GrResourceProvider::SemaphoreContext fDoneContext;

    typedef GrSemaphore INHERITED;
};

#endif
