/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorUnitTest.h"
#include "GrFragmentProcessor.h"

const GrFragmentProcessor* GrProcessorUnitTest::CreateChildFP(GrProcessorTestData* data) {
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    SkAutoTUnref<const GrFragmentProcessor> fp;
    do {
        fp.reset(GrProcessorTestFactory<GrFragmentProcessor>::Create(data));
        SkASSERT(fp);
    } while (fp->numChildProcessors() != 0);
    return SkRef(fp.get());
#else
    SkFAIL("Should not be called if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS");
    return nullptr;
#endif
}
