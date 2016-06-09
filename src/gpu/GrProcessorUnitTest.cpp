/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorUnitTest.h"
#include "GrFragmentProcessor.h"

sk_sp<GrFragmentProcessor> GrProcessorUnitTest::MakeChildFP(GrProcessorTestData* data) {
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    sk_sp<GrFragmentProcessor> fp;
    do {
        fp = GrProcessorTestFactory<GrFragmentProcessor>::Make(data);
        SkASSERT(fp);
    } while (fp->numChildProcessors() != 0);
    return fp;
#else
    SkFAIL("Should not be called if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS");
    return nullptr;
#endif
}
