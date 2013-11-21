
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrGpu.h"
#include "GrDrawTargetCaps.h"
#include "Test.h"

static void test_print(skiatest::Reporter*, const GrDrawTargetCaps* caps) {
    // This used to assert.
    SkString result = caps->dump();
    SkASSERT(!result.isEmpty());
}

static void TestGrDrawTarget(skiatest::Reporter* reporter, GrContextFactory* factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);

        GrContext* grContext = factory->get(glType);
        if (NULL == grContext) {
            continue;
        }

        test_print(reporter, grContext->getGpu()->caps());
    }
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GrDrawTarget", TestGrDrawTargetClass, TestGrDrawTarget)

#endif
