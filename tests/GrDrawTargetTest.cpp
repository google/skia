
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU

#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpu.h"

DEF_GPUTEST_FOR_ALL_CONTEXTS(GrDrawTargetPrint, reporter, context) {
    // This used to assert.
    SkString result = context->caps()->dump();
    SkASSERT(!result.isEmpty());
    SkString shaderResult = context->caps()->shaderCaps()->dump();
    SkASSERT(!shaderResult.isEmpty());
}

#endif
