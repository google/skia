/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "SkPathTypes.h"
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
    SkPathBuilder(SkPathFillType = SkPathFillType::kWinding);

    SkPathBuilder& reset();

    SkPathFillType getFillType() const;
    SkPathBuilder& setFillType(SkPathFillType);

    SkPathBuilder& moveTo(SkPoint);
    SkPathBuilder& lineTo(SkPoint);
    SkPathBuilder& quadTo(SkPoint, SkPoint);
    SkPathBuilder& conicTo(SkPoint, SkPoint, SkScalar w);
    SkPathBuilder& cubicTo(SkPoint, SkPoint, SkPoint);
    SkPathBuilder& close();

    SkPathBulider& addRect(const SkRect&, SkPathDirection = SkPathDirection::kCW);
    SkPathBulider& addOval(const SkRect&, SkPathDirection = SkPathDirection::kCW);
    SkPathBulider& addRRect(const SkRRect&, SkPathDirection = SkPathDirection::kCW);

    SkPathRef make();
};

#endif
