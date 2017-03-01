/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSemaphore_DEFINED
#define GrGLSemaphore_DEFINED

#include "GrSemaphore.h"

#include "GrGLGpu.h"

class GrGLSemaphore : public GrSemaphore {
public:
    static sk_sp<GrGLSemaphore> Make(const GrGLGpu* gpu) {
        return sk_sp<GrGLSemaphore>(new GrGLSemaphore(gpu));
    }

    ~GrGLSemaphore() override {
        if (fGpu) {
            static_cast<const GrGLGpu*>(fGpu)->deleteSync(fSync);
        }
    }

    GrGLsync sync() const { return fSync; }
    void setSync(const GrGLsync& sync) { fSync = sync; }

private:
    GrGLSemaphore(const GrGLGpu* gpu) : INHERITED(gpu), fSync(0) {}

    GrGLsync fSync;

    typedef GrSemaphore INHERITED;
};

#endif
