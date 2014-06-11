/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"

#include "SkString.h"
#include "Test.h"

DEF_TEST(PictureUtils, reporter) {
    SkString result;
    SkString filename("test");
    SkString dir("test/path");
    sk_tools::make_filepath(&result, dir, filename);
    REPORTER_ASSERT(reporter, result.equals("test/path/test"));
}
