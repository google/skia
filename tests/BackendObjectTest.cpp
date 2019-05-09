/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendObject.h"
#include "tests/Test.h"

#include <utility>

void test1(GrContext* context, skiatest::Reporter* reporter) {

    REPORTER_ASSERT(reporter, true);

}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    test1(context, reporter);
}
