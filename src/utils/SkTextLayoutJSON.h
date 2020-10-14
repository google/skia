/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TextLayoutJSONWriter_DEFINED
#define TextLayoutJSONWriter_DEFINED

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkSpan.h"

using namespace skia::textlayout;
class SkJSON;
class SkJSONWriter;

class SkTextLayoutJSON {
public:
    static bool writeInput(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph);
    static bool writeOutput(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph);
    static bool paintOutput(SkCanvas* canvas, SkJSON* JSONReader);
private:
    static bool writeStyles(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph);
    static bool writeSkShaper(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph);
    static bool writeSkUnicode(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph);
};

#endif  // TextLayoutJSON_DEFINED
