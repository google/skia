/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/FenceSync.h"

#include <memory>

using namespace sk_gpu_test;

DEF_GANESH_TEST(GrContextFactory_abandon, reporter, options, CtsEnforcement::kNever) {
    for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
        GrContextFactory testFactory(options);
        skgpu::ContextType ctxType = static_cast<skgpu::ContextType>(i);
        ContextInfo info1 = testFactory.getContextInfo(ctxType);
        if (!info1.directContext()) {
            continue;
        }
        REPORTER_ASSERT(reporter, info1.testContext());
         // Ref for comparison. The API does not explicitly say that this stays alive.
        info1.directContext()->ref();
        testFactory.abandonContexts();

        // Test that we get different context after abandon.
        ContextInfo info2 = testFactory.getContextInfo(ctxType);
        REPORTER_ASSERT(reporter, info2.directContext());
        REPORTER_ASSERT(reporter, info2.testContext());

        REPORTER_ASSERT(reporter, info1.directContext() != info2.directContext());
        // The GL context should also change, but it also could get the same address.

        info1.directContext()->unref();
    }
}

DEF_GANESH_TEST(GrContextFactory_sharedContexts, reporter, options, CtsEnforcement::kApiLevel_T) {
    for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
        GrContextFactory testFactory(options);
        skgpu::ContextType ctxType = static_cast<skgpu::ContextType>(i);
        ContextInfo info1 = testFactory.getContextInfo(ctxType);
        if (!info1.directContext()) {
            continue;
        }

        // Ref for passing in. The API does not explicitly say that this stays alive.
        info1.directContext()->ref();
        testFactory.abandonContexts();

        // Test that creating a context in a share group with an abandoned context fails.
        ContextInfo info2 = testFactory.getSharedContextInfo(info1.directContext());
        REPORTER_ASSERT(reporter, !info2.directContext());
        info1.directContext()->unref();

        // Create a new base context
        ContextInfo info3 = testFactory.getContextInfo(ctxType);

        // Creating a context in a share group may fail, but should never crash.
        ContextInfo info4 = testFactory.getSharedContextInfo(info3.directContext());
        if (!info4.directContext()) {
            continue;
        }
        REPORTER_ASSERT(reporter, info3.directContext() != info4.directContext());
        REPORTER_ASSERT(reporter, info3.testContext() != info4.testContext());

        // Passing a different index should create a new (unique) context.
        ContextInfo info5 = testFactory.getSharedContextInfo(info3.directContext(), 1);
        REPORTER_ASSERT(reporter, info5.directContext());
        REPORTER_ASSERT(reporter, info5.testContext());
        REPORTER_ASSERT(reporter, info5.directContext() != info4.directContext());
        REPORTER_ASSERT(reporter, info5.testContext() != info4.testContext());
    }
}

DEF_GANESH_TEST(GrContextFactory_executorAndTaskGroup, reporter, options, CtsEnforcement::kNever) {
    for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
        // Verify that contexts have a task group iff we supply an executor with context options
        GrContextOptions contextOptions = options;
        contextOptions.fExecutor = nullptr;
        GrContextFactory serialFactory(contextOptions);

        std::unique_ptr<SkExecutor> threadPool = SkExecutor::MakeFIFOThreadPool(1);
        contextOptions.fExecutor = threadPool.get();
        GrContextFactory threadedFactory(contextOptions);

        skgpu::ContextType ctxType = static_cast<skgpu::ContextType>(i);
        ContextInfo serialInfo = serialFactory.getContextInfo(ctxType);
        if (auto serialContext = serialInfo.directContext()) {
            REPORTER_ASSERT(reporter, nullptr == serialContext->priv().getTaskGroup());
        }

        ContextInfo threadedInfo = threadedFactory.getContextInfo(ctxType);
        if (auto threadedContext = threadedInfo.directContext()) {
            REPORTER_ASSERT(reporter, nullptr != threadedContext->priv().getTaskGroup());
        }
    }
}

#ifdef SK_ENABLE_DUMP_GPU
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(GrContextDump, reporter, ctxInfo, CtsEnforcement::kNever) {
    // Ensure that GrDirectContext::dump doesn't assert (which is possible, if the JSON code
    // is wrong)
    SkString result = ctxInfo.directContext()->dump();
    REPORTER_ASSERT(reporter, !result.isEmpty());
}
#endif
