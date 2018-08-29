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
    static sk_sp<GrGLSemaphore> Make(const GrGLGpu* gpu, bool isOwned) {
        return sk_sp<GrGLSemaphore>(new GrGLSemaphore(gpu, isOwned));
    }

    static sk_sp<GrGLSemaphore> MakeWrapped(const GrGLGpu* gpu,
                                            GrGLsync sync,
                                            GrWrapOwnership ownership) {
        auto sema = sk_sp<GrGLSemaphore>(new GrGLSemaphore(gpu,
                                                           kBorrow_GrWrapOwnership != ownership));
        sema->setSync(sync);
        return sema;
    }

    ~GrGLSemaphore() override;

    GrGLsync sync() const { return fSync; }
    void setSync(const GrGLsync& sync) { fSync = sync; }

    GrBackendSemaphore backendSemaphore() const override {
        GrBackendSemaphore backendSemaphore;
        backendSemaphore.initGL(fSync);
        return backendSemaphore;
    }

private:
    GrGLSemaphore(const GrGLGpu* gpu, bool isOwned);


    GrGLsync fSync;
    bool     fIsOwned;

    typedef GrSemaphore INHERITED;
};

#endif
