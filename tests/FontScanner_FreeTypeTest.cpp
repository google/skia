/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkWriteBuffer.h"

#include "tests/FontScanner.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include "include/ports/SkFontScanner_FreeType.h"

DEF_TEST(FontScanner_FreeType_VariableFont, reporter) {
    FontScanner_VariableFont(reporter, SkFontScanner_Make_FreeType().get());
}

DEF_TEST(FontScanner_FreeType__NamedInstances1, reporter) {
    FontScanner_NamedInstances1(reporter, SkFontScanner_Make_FreeType().get());
}

DEF_TEST(FontScanner_FreeType__NamedInstances2, reporter) {
    FontScanner_NamedInstances2(reporter, SkFontScanner_Make_FreeType().get());
}

DEF_TEST(FontScanner_FreeType_FontCollection, reporter) {
    FontScanner_FontCollection(reporter, SkFontScanner_Make_FreeType().get());
}
