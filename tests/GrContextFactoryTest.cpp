/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrCaps.h"
#include "Test.h"

using namespace sk_gpu_test;

DEF_GPUTEST(GrContextFactory_NVPRContextOptionHasPathRenderingSupport, reporter, /*factory*/) {
    // Test that if NVPR is requested, the context always has path rendering
    // or the context creation fails.
    GrContextFactory testFactory;
    // Test that if NVPR is possible, caps are in sync.
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        GrContext* context = testFactory.get(ctxType,
                                           GrContextFactory::ContextOverrides::kRequireNVPRSupport);
        if (!context) {
            continue;
        }
        REPORTER_ASSERT(
            reporter,
            context->caps()->shaderCaps()->pathRenderingSupport());
    }
}

DEF_GPUTEST(GrContextFactory_NoPathRenderingIfNVPRDisabled, reporter, /*factory*/) {
    // Test that if NVPR is explicitly disabled, the context has no path rendering support.

    GrContextFactory testFactory;
    for (int i = 0; i <= GrContextFactory::kLastContextType; ++i) {
        GrContextFactory::ContextType ctxType = (GrContextFactory::ContextType)i;
        GrContext* context =
            testFactory.get(ctxType, GrContextFactory::ContextOverrides::kDisableNVPR);
        if (context) {
            REPORTER_ASSERT(
                reporter,
                !context->caps()->shaderCaps()->pathRenderingSupport());
        }
    }
}

DEF_GPUTEST(GrContextFactory_RequiredSRGBSupport, reporter, /*factory*/) {
    // Test that if sRGB support is requested, the context always has that capability
    // or the context creation fails. Also test that if the creation fails, a context
    // created without that flag would not have had sRGB support.
    GrContextFactory testFactory;
    // Test that if sRGB is requested, caps are in sync.
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
        GrContext* context =
            testFactory.get(ctxType, GrContextFactory::ContextOverrides::kRequireSRGBSupport);

        if (context) {
            REPORTER_ASSERT(reporter, context->caps()->srgbSupport());
        } else {
            context = testFactory.get(ctxType);
            if (context) {
                REPORTER_ASSERT(reporter, !context->caps()->srgbSupport());
            }
        }
    }
}

DEF_GPUTEST(GrContextFactory_abandon, reporter, /*factory*/) {
    GrContextFactory testFactory;
    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
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

DEF_GPUTEST(GrContextFactory_sharedContexts, reporter, /*factory*/) {
    GrContextFactory testFactory;

    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
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
        if (!info3.grContext()) {
            // Vulkan NexusPlayer bot fails here. Sigh.
            continue;
        }

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

#endif
