/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferedDrawTarget_DEFINED
#define GrBufferedDrawTarget_DEFINED

#include "GrDrawTarget.h"
#include "GrCommandBuilder.h"
#include "SkChunkAlloc.h"

/**
 * GrBufferedDrawTarget is an implementation of GrDrawTarget that queues up draws for eventual
 * playback into a GrGpu. In theory one draw buffer could playback into another. Similarly, it is
 * the caller's responsibility to ensure that all referenced textures, buffers, and render-targets
 * are associated in the GrGpu object that the buffer is played back into.
 */
class GrBufferedDrawTarget : public GrClipTarget {
public:
    /**
     * Creates a GrBufferedDrawTarget
     *
     * @param context    the context object that owns this draw buffer.
     */
    GrBufferedDrawTarget(GrContext* context);

    ~GrBufferedDrawTarget() override;

protected:
    void onDrawBatch(GrBatch*) override;

private:
    void onReset() override;
    void onFlush() override;

    SkAutoTDelete<GrCommandBuilder>     fCommands;
    uint32_t                            fDrawID;

    typedef GrClipTarget INHERITED;
};

#endif
