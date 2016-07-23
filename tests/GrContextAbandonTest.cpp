/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "Test.h"

using namespace sk_gpu_test;

DEF_GPUTEST(GrContext_abandonContext, reporter, /*factory*/) {
    for (int testType = 0; testType < 6; ++testType) {
        for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
            GrContextFactory testFactory;
            GrContextFactory::ContextType ctxType = (GrContextFactory::ContextType) i;
            ContextInfo info = testFactory.getContextInfo(ctxType);
            if (GrContext* context = info.grContext()) {
                switch (testType) {
                    case 0:
                        context->abandonContext();
                        break;
                    case 1:
                        context->releaseResourcesAndAbandonContext();
                        break;
                    case 2:
                        context->abandonContext();
                        context->abandonContext();
                        break;
                    case 3:
                        context->abandonContext();
                        context->releaseResourcesAndAbandonContext();
                        break;
                    case 4:
                        context->releaseResourcesAndAbandonContext();
                        context->abandonContext();
                        break;
                    case 5:
                        context->releaseResourcesAndAbandonContext();
                        context->releaseResourcesAndAbandonContext();
                        break;
                }
            }
        }
    }
}

#endif
