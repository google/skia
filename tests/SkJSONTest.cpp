/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkJSON.h"

DEF_TEST(SkJSON_Parse, reporter) {
    static constexpr auto t1 =
        "{ "
        "  \"foo1\":   \"shh\" ,"
        "  \"foo2\": [ \"s1 \", \"s2\" ], "
        "  \"foo3\": { "
        "    \"bar1\" : true,"
        "    \"bar2\" : false,"
        "    \"bar3\" : null"
        "  }"
        "} ";

    REPORTER_ASSERT(reporter, skjson::Parse(t1, strlen(t1)));
}
