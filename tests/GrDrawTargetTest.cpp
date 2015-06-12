
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
#include "GrContextFactory.h"
#include "GrGpu.h"

static void test_print(skiatest::Reporter*, const GrCaps* caps) {
    // This used to assert.
    SkString result = caps->dump();
    SkASSERT(!result.isEmpty());
    SkString shaderResult = caps->shaderCaps()->dump();
    SkASSERT(!shaderResult.isEmpty());
}

DEF_GPUTEST(GrDrawTarget, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);

        GrContext* grContext = factory->get(glType);
        if (NULL == grContext) {
            continue;
        }

        test_print(reporter, grContext->caps());
    }
}

#endif
