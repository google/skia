/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathSink.h"

SkPathSink& SkPathSink::moveTo(SkPoint a) {
    this->onMoveTo(a);
    return *this;
}

SkPathSink& SkPathSink::lineTo(SkPoint a) {
    this->onLineTo(a);
    return *this;
}

SkPathSink& SkPathSink::quadTo(SkPoint a, SkPoint b) {
    this->onQuadTo(a, b);
    return *this;
}

SkPathSink& SkPathSink::conicTo(SkPoint a, SkPoint b, SkScalar w) {
    // check for <= 0 or NaN with this test
    if (!(w > 0)) {
        this->lineTo(b);
    } else if (!SkScalarIsFinite(w)) {
        this->lineTo(a).lineTo(b);
    } else if (SK_Scalar1 == w) {
        this->quadTo(a, b);
    } else {
        this->onConicTo(a, b, w);
    }
    return *this;
}

SkPathSink& SkPathSink::cubicTo(SkPoint a, SkPoint b, SkPoint c) {
    this->onCubicTo(a, b, c);
    return *this;
}
