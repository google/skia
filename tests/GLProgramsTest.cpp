
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "Test.h"
#include "GrContext.h"
#include "gl/GrGpuGL.h"

static void GLProgramsTest(skiatest::Reporter* reporter, GrContext* context) {
    GrGpuGL* shadersGpu = static_cast<GrGpuGL*>(context->getGpu());
    REPORTER_ASSERT(reporter, shadersGpu->programUnitTest());
}


#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GLPrograms", GLProgramsTestClass, GLProgramsTest)

#endif
