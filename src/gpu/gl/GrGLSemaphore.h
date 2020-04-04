/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSemaphore_DEFINED
#define GrGLSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSemaphore.h"

class GrGLGpu;

class GrGLSemaphore : public GrSemaphore {
public:
    static std::unique_ptr<GrGLSemaphore> Make(GrGLGpu* gpu, bool isOwned) {
        return std::unique_ptr<GrGLSemaphore>(new GrGLSemaphore(gpu, isOwned));
    }

    static std::unique_ptr<GrGLSemaphore> MakeWrapped(GrGLGpu* gpu,
                                                      GrGLsync sync,
                                                      GrWrapOwnership ownership) {
        auto sema = std::unique_ptr<GrGLSemaphore>(
                new GrGLSemaphore(gpu, kBorrow_GrWrapOwnership != ownership));
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
    GrGLSemaphore(GrGLGpu* gpu, bool isOwned);

    void setIsOwned() override {
        fIsOwned = true;
    }

    GrGLGpu* fGpu;
    GrGLsync fSync;
    bool     fIsOwned;

    typedef GrSemaphore INHERITED;
};

#endif
