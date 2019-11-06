/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/gpu/gl/GrGLExtensions.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "tests/Test.h"

const GrGLubyte* simpleGetString(GrGLenum name) {
    return (const GrGLubyte*)(name == GR_GL_VERSION ? "3.0" : "");
}

void simpleGetIntegerv(GrGLenum name, GrGLint* params) {
    if (name == GR_GL_NUM_EXTENSIONS) {
        *params = 2;
    } else {
        *params = 0;
    }
}

const GrGLubyte* simpleGetStringi(GrGLenum name, GrGLuint index) {
    if (name != GR_GL_EXTENSIONS || index >= 2)
        return (const GrGLubyte*)"";
    return (const GrGLubyte*)(index == 0 ? "test_extension_1" : "test_extension_2");
}

DEF_TEST(GrGLExtensionsTest_remove, reporter) {
    GrGLExtensions ext;
    ext.init(kGL_GrGLStandard,
             &simpleGetString,
             &simpleGetStringi,
             &simpleGetIntegerv,
             nullptr,
             nullptr);

    REPORTER_ASSERT(reporter, ext.isInitialized());
    REPORTER_ASSERT(reporter, ext.has("test_extension_1"));
    REPORTER_ASSERT(reporter, ext.has("test_extension_2"));
    REPORTER_ASSERT(reporter, ext.remove("test_extension_2"));
    REPORTER_ASSERT(reporter, !ext.has("test_extension_2"));
    REPORTER_ASSERT(reporter, ext.remove("test_extension_1"));
    REPORTER_ASSERT(reporter, !ext.has("test_extension_1"));
}
