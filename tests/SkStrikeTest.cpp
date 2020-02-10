/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkStrike.h"
#include "tools/ToolUtils.h"

#include "tests/Test.h"

DEF_TEST(SkStrikeMultiThread, Reporter) {
sk_sp<SkTypeface> typefaces[] = {
        ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()),
        ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic())};

typefaces[0]->createScalerContext()

SkFont font{typefaces[0], 24};

SkStrike strike{};
}
