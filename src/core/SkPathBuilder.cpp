/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathBuilder.h"

SkPathBuilder::SkPathBuilder() {
}

SkPathBuilder::~SkPathBuilder() {
}

void SkPathBuilder::incReserve(int extraPtCount) {
}

SkPathBuilder& SkPathBuilder::moveTo(SkPoint pt) {
    return *this;
}

SkPathBuilder& SkPathBuilder::lineTo(SkPoint pt) {
    return *this;
}

SkPathBuilder& SkPathBuilder::quadTo(SkPoint pt1, SkPoint pt2) {
    return *this;
}

SkPathBuilder& SkPathBuilder::conicTo(SkPoint pt1, SkPoint pt2, SkScalar w) {
    return *this;
}

SkPathBuilder& SkPathBuilder::cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3) {
    return *this;
}

SkPathBuilder& SkPathBuilder::close() {
    return *this;
}

void SkPathBuilder::addRect(const SkRect&, SkPathDirection dir) {
}
void SkPathBuilder::addOval(const SkRect&, SkPathDirection dir) {
}
void SkPathBuilder::addRRect(const SkRRect&, SkPathDirection dir) {
}


