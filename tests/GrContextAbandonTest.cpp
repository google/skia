/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/FenceSync.h"

struct GrContextOptions;

using namespace sk_gpu_test;

DEF_GANESH_TEST(GrContext_abandonContext, reporter, options, CtsEnforcement::kApiLevel_T) {
    for (int testType = 0; testType < 6; ++testType) {
        for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
            GrContextFactory testFactory(options);
            auto ctxType = static_cast<skgpu::ContextType>(i);
            ContextInfo info = testFactory.getContextInfo(ctxType);
            if (auto context = info.directContext()) {
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
