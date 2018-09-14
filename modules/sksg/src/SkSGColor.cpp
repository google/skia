/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColor.h"

namespace sksg {

Color::Color(SkColor c) : fColor(c) {
    SkDebugf("");
}

void Color::onApplyToPaint(SkPaint* paint) const {
    paint->setColor(fColor);
}

bool Color::onSetColor(const SkColor4f& c) {
    SkPaint p;
    p.setColor4f(c, nullptr);
    this->setColor(p.getColor());
    return true;
}

} // namespace sksg
