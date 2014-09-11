/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkError.h"
#include "SkPath.h"
#include "SkRect.h"
#include "Test.h"

typedef struct {
    skiatest::Reporter *fReporter;
    unsigned int *fIntPointer;
} ErrorContext;

#define CHECK(errcode)                                                        \
  REPORTER_ASSERT( reporter, (err = SkGetLastError()) == errcode);            \
  if (err != kNoError_SkError)                                                \
  {                                                                           \
     SkClearLastError();                                                      \
  }

static void cb(SkError err, void *context) {
    ErrorContext *context_ptr = static_cast<ErrorContext *>(context);
    REPORTER_ASSERT( context_ptr->fReporter, (*(context_ptr->fIntPointer) == 0xdeadbeef) );
}

DEF_TEST(Error, reporter) {
    // Some previous user of this thread may have left an error laying around.
    SkClearLastError();

    SkError err;

    unsigned int test_value = 0xdeadbeef;
    ErrorContext context;
    context.fReporter = reporter;
    context.fIntPointer = &test_value;

    SkSetErrorCallback(cb, &context);

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

    // should trigger *our* callback.
    path.addRoundRect(r, -10, -10);
    CHECK(kInvalidArgument_SkError);
    CHECK(kNoError_SkError);
}
