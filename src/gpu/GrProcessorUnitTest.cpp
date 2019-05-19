/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessorUnitTest.h"

#if GR_TEST_UTILS

std::unique_ptr<GrFragmentProcessor> GrProcessorUnitTest::MakeChildFP(GrProcessorTestData* data) {
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        fp = GrFragmentProcessorTestFactory::Make(data);
        SkASSERT(fp);
    } while (fp->numChildProcessors() != 0);
    return fp;
#else
    SK_ABORT("Should not be called if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS");
    return nullptr;
#endif
}
#endif
