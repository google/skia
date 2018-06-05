/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDebugMarkerOp.h"

#include "GrContext.h"
#include "GrContextPriv.h"

std::unique_ptr<GrOp> GrDebugMarkerOp::Make(GrContext* context,
                                            GrRenderTargetProxy* proxy,
                                            const SkString& str) {
    // $$
    GrMemoryPool* pool = context->contextPriv().opMemoryPool();

    char* mem = (char*) pool->allocate(sizeof(GrDebugMarkerOp));
    return std::unique_ptr<GrOp>(new (mem) GrDebugMarkerOp(proxy, str));
}
