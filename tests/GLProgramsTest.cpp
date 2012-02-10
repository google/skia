
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "GrContext.h"
#include "gl/GrGpuGLShaders.h"

static void GLProgramsTest(skiatest::Reporter* reporter, GrContext* context) {
    GrGpuGLShaders* shadersGpu = (GrGpuGLShaders*) context->getGpu();
    REPORTER_ASSERT(reporter, shadersGpu->programUnitTest());
}


#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GLPrograms", GLProgramsTestClass, GLProgramsTest)

