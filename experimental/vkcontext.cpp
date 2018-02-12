/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <cstdio>
#include "vk/VkTestContext.h"
#include "GrContext.h"
#include "GrContextOptions.h"
int main() {
#ifdef SK_VULKAN
    std::unique_ptr<sk_gpu_test::TestContext> testCtx(
            sk_gpu_test::CreatePlatformVkTestContext(nullptr));
    fprintf(stderr, "TestContext: %p\n", testCtx.get());
    SkASSERT(testCtx);
    testCtx->makeCurrent();
    GrContextOptions grContextOptions;
    grContextOptions.fAllowPathMaskCaching = true;
    grContextOptions.fSuppressPathRendering = true;
    grContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    sk_sp<GrContext> grContext = testCtx->makeGrContext(grContextOptions);
    fprintf(stderr, "GrContext: %p\n", grContext.get());
#endif
    return 0;
}
