/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColor.h"

namespace sksg {

Color::Color(SkColor c) : fColor(c) {}

SkPaint Color::onMakePaint() const {
    SkPaint paint;
    paint.setColor(fColor);
    return paint;
}

} // namespace sksg
