/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDebugMarkerOp.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"

std::unique_ptr<GrOp> GrDebugMarkerOp::Make(GrContext* context,
                                            GrRenderTargetProxy* proxy,
                                            const SkString& str) {
    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrDebugMarkerOp>(proxy, str);
}

void GrDebugMarkerOp::onExecute(GrOpFlushState* state) {
    //SkDebugf("%s\n", fStr.c_str());
    if (state->caps().gpuTracingSupport()) {
        state->commandBuffer()->insertEventMarker(fStr.c_str());
    }
}
