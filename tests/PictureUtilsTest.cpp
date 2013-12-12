/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "picture_utils.h"
#include "SkString.h"

static void test_filepath_creation(skiatest::Reporter* reporter) {
    SkString result;
    SkString filename("test");
    SkString dir("test/path");
    sk_tools::make_filepath(&result, dir, filename);
    REPORTER_ASSERT(reporter, result.equals("test/path/test"));
}

static void test_get_basename(skiatest::Reporter* reporter) {
    SkString result;
    SkString path("/path/basename");
    sk_tools::get_basename(&result, path);
    REPORTER_ASSERT(reporter, result.equals("basename"));

    result.reset();
    path.set("/path/dir/");
    sk_tools::get_basename(&result, path);
    REPORTER_ASSERT(reporter, result.equals("dir"));

    result.reset();
    path.set("path");
    sk_tools::get_basename(&result, path);
    REPORTER_ASSERT(reporter, result.equals("path"));

#if defined(SK_BUILD_FOR_WIN)
    result.reset();
    path.set("path\\winbasename");
    sk_tools::get_basename(&result, path);
    REPORTER_ASSERT(reporter, result.equals("winbasename"));

    result.reset();
    path.set("path\\windir\\");
    sk_tools::get_basename(&result, path);
    REPORTER_ASSERT(reporter, result.equals("windir"));
#endif
}

DEF_TEST(PictureUtils, reporter) {
    test_filepath_creation(reporter);
    test_get_basename(reporter);
}
