/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTConf.h"
#include "SkOSEnvironment.h"
#include "Test.h"

// Friended proxy for SkRTConfRegistry::parse()
template <typename T>
bool test_rt_conf_parse(SkRTConfRegistry* reg, const char* key, T* value) {
    return reg->parse(key, value);
}

DEF_TEST(SkRTConfRegistry, reporter) {
    SkRTConfRegistry reg;

    sk_setenv("skia_nonexistent_item", "132");
    int result = 0;
    test_rt_conf_parse(&reg, "nonexistent.item", &result);
    REPORTER_ASSERT(reporter, result == 132);
}
