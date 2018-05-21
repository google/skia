/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkJSON.h"

#include "SkData.h"
#include "SkTime.h"

DEF_TEST(SkJSON_Parse, reporter) {
    static constexpr auto t1 =
        "{ "
        "  \"foo1\":   \"shh\" ,"
        "  \"foo2\": [ \"s1 \", \"s2\" ], "
        "  \"foo3\": { "
        "    \"bar1\" : true,"
        "    \"bar2\" : false,"
        "    \"bar3\" : null,"
        "    \"bar4\" : -2.3000e-14"
        "  }"
        "} ";

    REPORTER_ASSERT(reporter, skjson::Parse(t1, strlen(t1)));

    skjson::Dom dom(t1, strlen(t1));

    const auto data = SkData::MakeFromFileName("/tmp/tst.json");
    if (data) {
        REPORTER_ASSERT(reporter, skjson::Parse(reinterpret_cast<const char*>(data->data()), data->size()));
        do {
            skjson::Dom dom(reinterpret_cast<const char*>(data->data()), data->size());

        } while (true);
    }
}
