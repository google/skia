/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "SkRect.h"

class SkPathRef : public SkRefCnt {
public:
    bool isEmpty() const;
    bool isFinite() const;
    bool isConvex() const;

    SkPathFillType fillType() const;

    SkRect bounds() const;

    SkPathRef transform(const SkMatrix&) const;
};


class SkPathBuilder {
public:
    SkPathBuilder(SkPathFillType = kWinding_SkPathFillType);

    SkPathBuilder& setFillType(SkPathFillType);

    SkPathBuilder& moveTo(SkPoint);
    SkPathBuilder& lineTo(SkPoint);
    SkPathBuilder& quadTo(SkPoint, SkPoint);
    SkPathBuilder& conicTo(SkPoint, SkPoint, SkScalar w);
    SkPathBuilder& cubicTo(SkPoint, SkPoint, SkPoint);
    SkPathBuilder& close();

    SkPathBulider& addRect(const SkRect&, SkPathDirection = kCW_SkPathDirection);
    SkPathBulider& addOval(const SkRect&, SkPathDirection = kCW_SkPathDirection);
    SkPathBulider& addRRect(const SkRRect&, SkPathDirection = kCW_SkPathDirection);

    SkPathRef make();
};

#endif
