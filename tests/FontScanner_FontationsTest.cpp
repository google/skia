/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/ports/SkFontScanner_Fontations.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/FontScanner.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

DEF_TEST(FontScanner_Fontations_VariableFont, reporter) {
    FontScanner_VariableFont(reporter, SkFontScanner_Make_Fontations().get());
}

DEF_TEST(FontScanner_Fontations_NamedInstances1, reporter) {
    FontScanner_NamedInstances1(reporter, SkFontScanner_Make_Fontations().get());
}

DEF_TEST(FontScanner_Fontations_NamedInstances2, reporter) {
    FontScanner_NamedInstances2(reporter, SkFontScanner_Make_Fontations().get());
}

DEF_TEST(FontScanner_Fontations_FontCollection, reporter) {
    FontScanner_FontCollection(reporter, SkFontScanner_Make_Fontations().get());
}
