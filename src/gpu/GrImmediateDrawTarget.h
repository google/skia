/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImmediateDrawTarget_DEFINED
#define GrImmediateDrawTarget_DEFINED

#include "GrDrawTarget.h"

#include "GrBatchFlushState.h"

/**
 * A debug GrDrawTarget which immediately flushes every command it receives
 */
class GrImmediateDrawTarget : public GrClipTarget {
public:
    /**
     * Creates a GrImmediateDrawTarget
     *
     * @param context    the context object that owns this draw buffer.
     */
    GrImmediateDrawTarget(GrContext* context);

    ~GrImmediateDrawTarget() override;

protected:
    void onDrawBatch(GrBatch*) override;

private:
    void onReset() override;
    void onFlush() override;

    uint32_t fDrawID;

    typedef GrClipTarget INHERITED;
};

#endif
