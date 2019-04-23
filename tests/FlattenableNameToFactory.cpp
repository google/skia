/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "tests/Test.h"

DEF_TEST(FlattenableNameToFactory, r) {
    if (!SkFlattenable::NameToFactory("SkImageShader")) {
        ERRORF(r, "SkFlattenable::NameToFactory() fails with SkImageShader.");
    }
    if (SkFlattenable::NameToFactory("AAA-non-existent")) {
        ERRORF(r, "SkFlattenable::NameToFactory() succeeds with AAA-non-existent.");
    }
    if (SkFlattenable::NameToFactory("SkNonExistent")) {
        ERRORF(r, "SkFlattenable::NameToFactory() succeeds with SkNonExistent");
    }
    if (SkFlattenable::NameToFactory("ZZZ-non-existent")) {
        ERRORF(r, "SkFlattenable::NameToFactory() succeeds with ZZZ-non-existent.");
    }
}
