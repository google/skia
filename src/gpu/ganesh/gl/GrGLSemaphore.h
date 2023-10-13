/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSemaphore_DEFINED
#define GrGLSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrSemaphore.h"

#include <memory>

class GrGLGpu;

class GrGLSemaphore : public GrSemaphore {
public:
    static std::unique_ptr<GrGLSemaphore> Make(GrGLGpu* gpu, bool isOwned) {
        return std::unique_ptr<GrGLSemaphore>(new GrGLSemaphore(gpu, isOwned));
    }

    ~GrGLSemaphore() override;

    GrGLsync sync() const { return fSync; }
    void setSync(const GrGLsync& sync) { fSync = sync; }

    GrBackendSemaphore backendSemaphore() const override {
        SK_ABORT("Unsupported");
    }

private:
    GrGLSemaphore(GrGLGpu* gpu, bool isOwned);

    void setIsOwned() override {
        fIsOwned = true;
    }

    GrGLGpu* fGpu;
    GrGLsync fSync;
    bool     fIsOwned;

    using INHERITED = GrSemaphore;
};

#endif
