
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkError.h"
#include "SkPath.h"
#include "SkRect.h"

#define CHECK(errcode)                                                        \
  REPORTER_ASSERT( reporter, (err = SkGetLastError()) == errcode);            \
  if (err != kNoError_SkError)                                                \
  {                                                                           \
     SkDebugf("Last error string: %s\n", SkGetLastErrorString());             \
     SkClearLastError();                                                      \
  }

static void cb(SkError err, void *context) {
    int *context_ptr = static_cast<int *>(context);
    SkDebugf("CB (0x%x): %s\n", *context_ptr, SkGetLastErrorString());
}

static void ErrorTest(skiatest::Reporter* reporter) {
    SkError err;

    CHECK(kNoError_SkError);

    SkRect r = SkRect::MakeWH(50, 100);
    CHECK(kNoError_SkError);

    SkPath path;
    path.addRect(r);
    CHECK(kNoError_SkError);

    path.addRoundRect(r, 10, 10);
    CHECK(kNoError_SkError);

    // should trigger the default error callback, which just prints to the screen.
    path.addRoundRect(r, -10, -10);
    CHECK(kInvalidArgument_SkError);
    CHECK(kNoError_SkError);

    int test_value = 0xdeadbeef;
    SkSetErrorCallback(cb, &test_value);

    // should trigger *our* callback.
    path.addRoundRect(r, -10, -10);
    CHECK(kInvalidArgument_SkError);
    CHECK(kNoError_SkError);

    // Should trigger the default one again.
    SkSetErrorCallback(NULL, NULL);
    path.addRoundRect(r, -10, -10);
    CHECK(kInvalidArgument_SkError);
    CHECK(kNoError_SkError);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Error", ErrorTestClass, ErrorTest)
