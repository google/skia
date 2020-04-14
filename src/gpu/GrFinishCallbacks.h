/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFinishCallbacks_DEFINED
#define GrFinishCallbacks_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

#include <list>

class GrGpu;

class GrFinishCallbacks {
public:
    GrFinishCallbacks(GrGpu* gpu);
   ~GrFinishCallbacks();
    void call(bool doDelete);
    void add(GrGpuFinishedProc finishedProc, GrGpuFinishedContext finishedContext);
    void check();
    bool empty() const { return !fCallbacks.empty(); }
private:
    struct FinishCallback {
        GrGpuFinishedProc     fCallback;
        GrGpuFinishedContext  fContext;
        GrFence               fFence;
    };

    GrGpu*                           fGpu;
    std::list<FinishCallback>        fCallbacks;
};

#endif
