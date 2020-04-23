/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "absl/hash/hash.h"
#include "absl/strings/substitute.h"

DEF_TEST(AbseilTest, reporter) {
    // Tests that Abseil can be compiled, linked and run. Can be removed once Abseil is in use
    // elsewhere.
    const void* nullVoid = nullptr;
    const std::string kStringToHash = absl::Substitute("$0 $1 $2 $3", "absl", 0, false, nullVoid);
    REPORTER_ASSERT(reporter, kStringToHash == "absl 0 false NULL");

    const size_t hashValue = absl::Hash<std::string>{}(kStringToHash);
    REPORTER_ASSERT(reporter, hashValue != 0);
}

#endif  // SK_BUILD_FOR_ANDROID_FRAMEWORK
