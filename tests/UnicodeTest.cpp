
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkUtils.h"

// Unicode Variation Selector ranges: inclusive
#define UVS_MIN0    0x180B
#define UVS_MAX0    0x180D
#define UVS_MIN1    0xFE00
#define UVS_MAX1    0xFE0F
#define UVS_MIN2    0xE0100
#define UVS_MAX2    0xE01EF

static bool isUVS(SkUnichar uni) {
    return (uni >= UVS_MIN0 && uni <= UVS_MAX0) ||
           (uni >= UVS_MIN1 && uni <= UVS_MAX1) ||
           (uni >= UVS_MIN2 && uni <= UVS_MAX2);
}

static void test_uvs(skiatest::Reporter* reporter) {
    // [min, max], [min, max] ... inclusive
    static const SkUnichar gRanges[] = {
        UVS_MIN0, UVS_MAX0, UVS_MIN1, UVS_MAX1, UVS_MIN2, UVS_MAX2
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRanges); i += 2) {
        for (SkUnichar uni = gRanges[i] - 8; uni <= gRanges[i+1] + 8; ++uni) {
            bool uvs0 = isUVS(uni);
            bool uvs1 = SkUnichar_IsVariationSelector(uni);
            REPORTER_ASSERT(reporter, uvs0 == uvs1);
        }
    }
}

static void TestUnicode(skiatest::Reporter* reporter) {
    test_uvs(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Unicode", TestUnicodeClass, TestUnicode)
