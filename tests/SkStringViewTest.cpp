/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkStringView.h"
#include "tests/Test.h"

#include <string>
#include <string_view>

DEF_TEST(SkStringViewStartsAndEnds, r) {
    std::string_view empty("");
    REPORTER_ASSERT(r, empty.empty());
    REPORTER_ASSERT(r, !skstd::starts_with(empty, 'x'));
    REPORTER_ASSERT(r, !skstd::ends_with(empty, 'x'));
    REPORTER_ASSERT(r, !skstd::starts_with(empty, "x"));
    REPORTER_ASSERT(r, !skstd::ends_with(empty, "x"));
    REPORTER_ASSERT(r, skstd::starts_with(empty, ""));
    REPORTER_ASSERT(r, skstd::ends_with(empty, ""));

    std::string_view xyz("xyz");
    REPORTER_ASSERT(r, !xyz.empty());
    REPORTER_ASSERT(r, xyz.front() == 'x');
    REPORTER_ASSERT(r, xyz.back() == 'z');
    REPORTER_ASSERT(r, xyz.length() == 3);

    REPORTER_ASSERT(r, skstd::starts_with(xyz, 'x'));
    REPORTER_ASSERT(r, !skstd::starts_with(xyz, 'y'));
    REPORTER_ASSERT(r, skstd::ends_with(xyz, 'z'));
    REPORTER_ASSERT(r, !skstd::ends_with(xyz, 'y'));

    REPORTER_ASSERT(r, skstd::starts_with(xyz, ""));
    REPORTER_ASSERT(r, skstd::ends_with(xyz, ""));
    REPORTER_ASSERT(r, skstd::starts_with(xyz, "x"));
    REPORTER_ASSERT(r, skstd::ends_with(xyz, "z"));
    REPORTER_ASSERT(r, !skstd::starts_with(xyz, "xa"));
    REPORTER_ASSERT(r, !skstd::ends_with(xyz, "az"));
    REPORTER_ASSERT(r, skstd::starts_with(xyz, "xy"));
    REPORTER_ASSERT(r, skstd::ends_with(xyz, "yz"));
    REPORTER_ASSERT(r, skstd::starts_with(xyz, "xyz"));
    REPORTER_ASSERT(r, skstd::ends_with(xyz, "xyz"));
    REPORTER_ASSERT(r, !skstd::starts_with(xyz, "wxyz"));
    REPORTER_ASSERT(r, !skstd::ends_with(xyz, "wxyz"));

    xyz.swap(empty);
    REPORTER_ASSERT(r, xyz == "");
    REPORTER_ASSERT(r, empty == "xyz");
}

DEF_TEST(SkStringViewContains, r) {
    REPORTER_ASSERT(r, skstd::contains("ttttest1tttest2tttest3", "test"));
    REPORTER_ASSERT(r, skstd::contains("ttttest1tttest2tttest3", "test3"));
    REPORTER_ASSERT(r, !skstd::contains("ttttest1tttest2tttest3", "test4"));
    REPORTER_ASSERT(r, skstd::contains("", ""));
    REPORTER_ASSERT(r, !skstd::contains("", "a"));
    REPORTER_ASSERT(r, skstd::contains("abcabcd", "abcd"));
    REPORTER_ASSERT(r, skstd::contains("abc", ""));
    REPORTER_ASSERT(r, skstd::contains("abc", "a"));
    REPORTER_ASSERT(r, skstd::contains("abc", "b"));
    REPORTER_ASSERT(r, skstd::contains("abc", "c"));
    REPORTER_ASSERT(r, skstd::contains("abc", "ab"));
    REPORTER_ASSERT(r, skstd::contains("abc", "bc"));
    REPORTER_ASSERT(r, !skstd::contains("abc", "ac"));
    REPORTER_ASSERT(r, !skstd::contains("abc", "cb"));
    REPORTER_ASSERT(r, !skstd::contains("abc", "abcd"));
}
