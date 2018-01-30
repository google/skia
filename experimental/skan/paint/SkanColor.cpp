/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkanColor.h"

namespace skan {

Color::Color(SkColor c) : fColor(c) {}

void Color::onApplyToPaint(SkPaint* paint) const {
    paint->setColor(fColor);
}

} // namespace skan
