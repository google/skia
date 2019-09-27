/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"

#include "modules/skparagraph/include/TextStyle.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmAliases.h"

using namespace emscripten;

struct SimpleTextStyle {
    SkColor color;
    SkColor foregroundColor;
    SkColor backgroundColor;
    skia::textlayout::TextDecoration decoration;
};

EMSCRIPTEN_BINDINGS(Paragraph) {


    value_object<SimpleTextStyle>("TextStyle")
        .field("color",           &SimpleTextStyle::color)
        .field("foregroundColor", &SimpleTextStyle::foregroundColor)
        .field("backgroundColor", &SimpleTextStyle::backgroundColor)
        .field("decoration",      &SimpleTextStyle::decoration);
}
