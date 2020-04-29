/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include <absl/container/btree_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/substitute.h>

DEF_TEST(AbseilTest, reporter) {
    // Tests that Abseil can be compiled, linked and run. Can be removed once Abseil is in use
    // elsewhere.
    const void* nullVoid = nullptr;
    const std::string kStringToHash = absl::Substitute("$0 $1 $2 $3", "absl", 0, false, nullVoid);
    REPORTER_ASSERT(reporter, kStringToHash == "absl 0 false NULL");

    const size_t hashValue = absl::Hash<std::string>{}(kStringToHash);
    REPORTER_ASSERT(reporter, hashValue != 0);

    absl::btree_map<int, int> m;
    m[42] = 47;
    m[41] = 46;

    int expected_key = 41,
        expected_val = 46;
    for (auto [k,v] : m) {
        REPORTER_ASSERT(reporter, k == expected_key++);
        REPORTER_ASSERT(reporter, v == expected_val++);
    }
}

#endif  // SK_BUILD_FOR_ANDROID_FRAMEWORK
