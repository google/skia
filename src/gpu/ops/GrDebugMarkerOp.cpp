/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDebugMarkerOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrRecordingContextPriv.h"

std::unique_ptr<GrOp> GrDebugMarkerOp::Make(GrRecordingContext* context,
                                            GrRenderTargetProxy* proxy,
                                            const SkString& str) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrDebugMarkerOp>(proxy, str);
}

void GrDebugMarkerOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    //SkDebugf("%s\n", fStr.c_str());
    if (state->caps().gpuTracingSupport()) {
        state->opsRenderPass()->insertEventMarker(fStr.c_str());
    }
}
