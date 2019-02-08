/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDebugMarkerOp.h"

#include "GrCaps.h"
#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"

std::unique_ptr<GrOp> GrDebugMarkerOp::Make(GrRecordingContext* context,
                                            GrRenderTargetProxy* proxy,
                                            const SkString& str) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrDebugMarkerOp>(proxy, str);
}

void GrDebugMarkerOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    //SkDebugf("%s\n", fStr.c_str());
    if (state->caps().gpuTracingSupport()) {
        state->commandBuffer()->insertEventMarker(fStr.c_str());
    }
}
