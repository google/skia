/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkExecutor.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

using namespace sk_gpu_test;

DEF_GPUTEST(GrContextFactory_abandon, reporter, options) {
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory testFactory(options);
        GrContextFactory::ContextType ctxType = (GrContextFactory::ContextType) i;
        ContextInfo info1 = testFactory.getContextInfo(ctxType);
        if (!info1.grContext()) {
            continue;
        }
        REPORTER_ASSERT(reporter, info1.testContext());
         // Ref for comparison. The API does not explicitly say that this stays alive.
        info1.grContext()->ref();
        testFactory.abandonContexts();

        // Test that we get different context after abandon.
        ContextInfo info2 = testFactory.getContextInfo(ctxType);
        REPORTER_ASSERT(reporter, info2.grContext());
        REPORTER_ASSERT(reporter, info2.testContext());

        REPORTER_ASSERT(reporter, info1.grContext() != info2.grContext());
        // The GL context should also change, but it also could get the same address.

        info1.grContext()->unref();
    }
}

DEF_GPUTEST(GrContextFactory_sharedContexts, reporter, options) {
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory testFactory(options);
        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        ContextInfo info1 = testFactory.getContextInfo(ctxType);
        if (!info1.grContext()) {
            continue;
        }

        // Ref for passing in. The API does not explicitly say that this stays alive.
        info1.grContext()->ref();
        testFactory.abandonContexts();

        // Test that creating a context in a share group with an abandoned context fails.
        ContextInfo info2 = testFactory.getSharedContextInfo(info1.grContext());
        REPORTER_ASSERT(reporter, !info2.grContext());
        info1.grContext()->unref();

        // Create a new base context
        ContextInfo info3 = testFactory.getContextInfo(ctxType);

        // Creating a context in a share group may fail, but should never crash.
        ContextInfo info4 = testFactory.getSharedContextInfo(info3.grContext());
        if (!info4.grContext()) {
            continue;
        }
        REPORTER_ASSERT(reporter, info3.grContext() != info4.grContext());
        REPORTER_ASSERT(reporter, info3.testContext() != info4.testContext());

        // Passing a different index should create a new (unique) context.
        ContextInfo info5 = testFactory.getSharedContextInfo(info3.grContext(), 1);
        REPORTER_ASSERT(reporter, info5.grContext());
        REPORTER_ASSERT(reporter, info5.testContext());
        REPORTER_ASSERT(reporter, info5.grContext() != info4.grContext());
        REPORTER_ASSERT(reporter, info5.testContext() != info4.testContext());
    }
}

DEF_GPUTEST(GrContextFactory_executorAndTaskGroup, reporter, options) {
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        // Verify that contexts have a task group iff we supply an executor with context options
        GrContextOptions contextOptions = options;
        contextOptions.fExecutor = nullptr;
        GrContextFactory serialFactory(contextOptions);

        std::unique_ptr<SkExecutor> threadPool = SkExecutor::MakeFIFOThreadPool(1);
        contextOptions.fExecutor = threadPool.get();
        GrContextFactory threadedFactory(contextOptions);

        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        ContextInfo serialInfo = serialFactory.getContextInfo(ctxType);
        if (GrContext* serialContext = serialInfo.grContext()) {
            REPORTER_ASSERT(reporter, nullptr == serialContext->priv().getTaskGroup());
        }

        ContextInfo threadedInfo = threadedFactory.getContextInfo(ctxType);
        if (GrContext* threadedContext = threadedInfo.grContext()) {
            REPORTER_ASSERT(reporter, nullptr != threadedContext->priv().getTaskGroup());
        }
    }
}

#ifdef SK_ENABLE_DUMP_GPU
DEF_GPUTEST_FOR_ALL_CONTEXTS(GrContextDump, reporter, ctxInfo) {
    // Ensure that GrContext::dump doesn't assert (which is possible, if the JSON code is wrong)
    SkString result = ctxInfo.grContext()->priv().dump();
    REPORTER_ASSERT(reporter, !result.isEmpty());
}
#endif
