/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "tests/Test.h"

using namespace skrive::internal;

DEF_TEST(SkRive_JsonReader, reporter) {
    static constexpr char json[] = R"({
                                     "version": 24
                                   })";

    auto sr = StreamReader::Make(json, strlen(json));

    REPORTER_ASSERT(reporter, sr);
}
