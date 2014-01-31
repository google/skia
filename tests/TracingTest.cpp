/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTraceEvent.h"
#include "Test.h"

DEF_TEST(Tracing, reporter) {
  TRACE_EVENT0("skia.testing", "just a test");
}
