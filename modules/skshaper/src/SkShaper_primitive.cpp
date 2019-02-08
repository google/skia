/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"
#include "SkShaperPriv.h"

struct SkShaper::Impl {};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(nullptr) {}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return true; }

SkPoint SkShaper::shape(RunHandler* handler,
                        const SkFont& font,
                        const char* utf8text,
                        size_t textBytes,
                        bool leftToRight,
                        SkPoint point,
                        SkScalar width) const {
    return ShapePrimitive(handler, font, utf8text, textBytes, leftToRight, point, width);
}
