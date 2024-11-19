/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFinishCallbacks_DEFINED
#define GrGLFinishCallbacks_DEFINED

#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "src/gpu/RefCntedCallback.h"

#include <list>

class GrGLGpu;

/**
 * Maintains a list of callbacks to be called when work on the GPU is complete.
 */

class GrGLFinishCallbacks {
public:
    GrGLFinishCallbacks(GrGLGpu* gpu);
   ~GrGLFinishCallbacks();

    /**
     * Call all the callbacks in the list. This will block until all work is done.
     *
     * @param doDelete        delete the contained fence object.
     */
    void callAll(bool doDelete);

    /**
     * Add a new callback to the list.
     *
     * @param finishedProc    The function to call when GPU work is complete.
     * @param finishedContext The context object to pass back to the callback.
     * @param timerQuery      A timer query to get the result from when finished.
     */
    void add(skgpu::AutoCallback, GrGLint timerQeury = 0);

    /**
     * Check if any GPU work is complete, and call the associated callbacks.
     * This call is non-blocking.
     */
    void check();

    /**
     * Returns true if the callback list is empty.
     */
    bool empty() const { return fCallbacks.empty(); }

private:
    struct FinishCallback {
        skgpu::AutoCallback fCallback;
        GrGLsync            fSync;
        GrGLint             fTimerQuery;
    };

    GrGLGpu*                  fGpu;
    std::list<FinishCallback> fCallbacks;
};

#endif  // GrGLFinishCallbacks
