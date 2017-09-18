/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"

#include "Test.h"

static constexpr int Half() {
    return 0xff * 0.5;
}

static constexpr int Double() {
    return 0xff * 2;
}

extern const int kDefinedElsewhere;

DEF_TEST(SkColor_SkColorConstARGB, reporter) {
    constexpr SkColor kLiteral = SkColorConstRGB(0xbb, 0xcc, 0xdd);
    constexpr SkColor kFromConst = SkColorConstRGB(Half(), Half(), Half());
    constexpr SkColor kLiteralA = SkColorConstARGB(SK_AlphaOPAQUE, 0xbb, 0xcc, 0xdd);
    constexpr SkColor kFromConstA = SkColorConstARGB(Half(), Half(), Half(), Half());
    REPORTER_ASSERT(reporter, kLiteral == SkColorSetRGB(0xbb, 0xcc, 0xdd));
    REPORTER_ASSERT(reporter, kFromConst == SkColorSetRGB(Half(), Half(), Half()));
    REPORTER_ASSERT(reporter, kLiteralA == SkColorSetARGB(0xaa, 0xbb, 0xcc, 0xdd));
    REPORTER_ASSERT(reporter, kFromConstA == SkColorSetARGB(Half(), Half(), Half(), Half()));

    // These generate "not a constant expression" compile errors.
    // SkColor kError1 = SkColorConstARGB(kDefinedElsewhere, 0, 0, 0);
    // SkColor kError2 = SkColorConstRGB(kDefinedElsewhere, 0, 0);

    // The following will static_assert.
    // SkColor kError3 = SkColorConstRGB(Double(), Double(), Double());
    // SkColor kError4 = SkColorConstARGB(Double(), Double(), Double(), Double());

    // Avoid unused warnings (and sanity check for hidden truncation).
    REPORTER_ASSERT(reporter, 4 * Half() == Double());
}
