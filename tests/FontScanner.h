/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef FontScanner_DEFINED
#define FontScanner_DEFINED

#include "include/core/SkFontScanner.h"
#include "tests/Test.h"
#include "tools/Resources.h"

void FontScanner_VariableFont(skiatest::Reporter* reporter, SkFontScanner* scanner);
void FontScanner_NamedInstances1(skiatest::Reporter* reporter, SkFontScanner* scanner);
void FontScanner_NamedInstances2(skiatest::Reporter* reporter, SkFontScanner* scanner);
void FontScanner_FontCollection(skiatest::Reporter* reporter, SkFontScanner* scanner);

#endif // FontScanner_DEFINED
