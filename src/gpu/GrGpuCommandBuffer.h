/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGpuCommandBuffer_DEFINED
#define GrGpuCommandBuffer_DEFINED

#include "GrColor.h"

class GrRenderTarget;

class GrGpuCommandBuffer {
public:
    enum LoadAndStoreOp {
        kLoadAndStore_LoadAndStoreOp,
        kLoadAndDiscard_LoadAndStoreOp,
        kClearAndStore_LoadAndStoreOp,
        kClearAndDiscard_LoadAndStoreOp,
        kDiscardAndStore_LoadAndStoreOp,
        kDiscardAndDiscard_LoadAndStoreOp,
    };

    GrGpuCommandBuffer() {}
    virtual ~GrGpuCommandBuffer() {}

    // Signals the end of recording to the command buffer and that it can now be submitted.
    virtual void end() = 0;

    // Sends the command buffer off to the GPU object to execute the commands built up in the
    // buffer. The gpu object is allowed to defer execution of the commands until it is flushed.
    virtual void submit() = 0;
};

#endif
