/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColor.h"

namespace sksg {

Color::Color(SkColor c) : fColor(c) {}

void Color::onApplyToPaint(SkPaint* paint) const {
    paint->setColor(fColor);
}

bool Color::onGetColor(SkColor4f* c) const {
    SkPaint p;
    p.setColor(fColor);
    *c = p.getColor4f();
    return true;
}

bool Color::onSetColor(const SkColor4f& c) {
    SkPaint p;
    p.setColor4f(c, nullptr);
    fColor = p.getColor();
    return true;
}

} // namespace sksg
