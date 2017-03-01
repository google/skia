/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathBuilder_DEFINED
#define SkPathBuilder_DEFINED

#include "SkPath.h"
#include "SkTArray.h"

// SkPathBuilder is not thread-safe.
class SkPathBuilder {
public:
    SkPathBuilder() {}
    ~SkPathBuilder() {}

    enum class FillType : uint8_t {
        kWinding,
        kEvenOdd,
        kInverseWinding,
        kInverseEvenOdd,
    };

    void setFillType(SkPathBuilder::FillType ft) { fFillType = ft; }

    void moveTo(SkScalar x, SkScalar y) {
        fSegments.push_back({Verb::kMove, {x, y, 0.0f, 0.0f, 0.0f, 0.0f}});
    }
    void lineTo(SkScalar x, SkScalar y) {
        fSegments.push_back({Verb::kLine, {x, y, 0.0f, 0.0f, 0.0f, 0.0f}});
    }
    void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
        fSegments.push_back({Verb::kQuad, {x1, y1, x2, y2, 0.0f, 0.0f}});
    }
    void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w) {
        fSegments.push_back({Verb::kConic, {x1, y1, x2, y2, w, 0.0f}});
    }
    void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3) {
        fSegments.push_back({Verb::kCubic, {x1, y1, x2, y2, x3, y3}});
    }
    void close() {
        fSegments.push_back({Verb::kClose, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}});
    }

    SkPath build() const;

    // TODO(halcanary): add convenience methods:
    //   void rMoveTo(SkScalar dx, SkScalar dy);
    //   void rLineTo(SkScalar dx, SkScalar dy);
    //   void rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2);
    //   void rConicTo(SkScalar, SkScalar, SkScalar, SkScalar, SkScalar);
    //   void rCubicTo(SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar);
    //   void arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool);
    //   void arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius);
    //   void arcTo(SkScalar, SkScalar, SkScalar, ArcSize, Direction, SkScalar, SkScalar);
    //   void rArcTo(SkScalar, SkScalar, SkScalar, ArcSize, Direction, SkScalar, SkScalar);
    //   void addRect(const SkRect& rect, Direction dir = kCW_Direction);
    //   void addRect(const SkRect& rect, Direction dir, unsigned start);
    //   void addOval(const SkRect& oval, Direction dir = kCW_Direction);
    //   void addOval(const SkRect& oval, Direction dir, unsigned start);
    //   void addCircle(SkScalar x, SkScalar y, SkScalar radius, Direction dir = kCW_Direction);
    //   void addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle);
    //   void addRoundRect(const SkRRect&, Direction dir = kCW_Direction);
    //   void addRRect(const SkRRect& rrect, Direction dir, unsigned start);
    //   void addPoly(const SkPoint pts[], int count, bool close);
    //   void addPath(const SkPath& src, SkScalar dx, SkScalar dy,
    //                AddPathMode mode = kAppend_AddPathMode);
    //   void addPath(const SkPath& src, const SkMatrix& matrix,
    //                AddPathMode mode = kAppend_AddPathMode);

private:
    enum class Verb : uint8_t {
        kMove,
        kLine,
        kQuad,
        kConic,
        kCubic,
        kClose,
    };
    struct Segment {
        Verb     fVerb;
        SkScalar fValues[6];
    };
    SkTArray<Segment, true> fSegments;
    FillType fFillType = FillType::kWinding;

    SkPathBuilder(SkPathBuilder&&) = delete;
    SkPathBuilder(const SkPathBuilder&) = delete;
    SkPathBuilder& operator=(SkPathBuilder&&) = delete;
    SkPathBuilder& operator=(const SkPathBuilder&) = delete;
};

inline SkPath SkPathBuilder::build() const {
    // TODO: optimize this to skip atomic on each path call.
    SkPath p;
    for (const SkPathBuilder::Segment& s : fSegments) {
        switch (s.fVerb) {
            case SkPathBuilder::Verb::kMove:
                p.moveTo(s.fValues[0], s.fValues[1]);
                break;
            case SkPathBuilder::Verb::kLine:
                p.lineTo(s.fValues[0], s.fValues[1]);
                break;
            case SkPathBuilder::Verb::kQuad:
                p.quadTo(s.fValues[0], s.fValues[1],
                         s.fValues[2], s.fValues[3]);
                break;
            case SkPathBuilder::Verb::kConic:
                p.conicTo(s.fValues[0], s.fValues[1],
                          s.fValues[2], s.fValues[3],
                          s.fValues[4]);
                break;
            case SkPathBuilder::Verb::kCubic:
                p.cubicTo(s.fValues[0], s.fValues[1],
                          s.fValues[2], s.fValues[3],
                          s.fValues[4], s.fValues[5]);
                break;
            case SkPathBuilder::Verb::kClose:
                break;
                p.close();
        }
    }
    SkPath::FillType f;
    switch (fFillType) {
        case SkPathBuilder::FillType::kWinding:
            f = SkPath::kWinding_FillType;
            break;
        case SkPathBuilder::FillType::kEvenOdd:
            f = SkPath::kEvenOdd_FillType;
            break;
        case SkPathBuilder::FillType::kInverseWinding:
            f = SkPath::kInverseWinding_FillType;
            break;
        case SkPathBuilder::FillType::kInverseEvenOdd:
            f = SkPath::kInverseEvenOdd_FillType;
            break;
    }
    p.setFillType(f);
    p.updateBoundsCache();
    return p;
}
#endif  // SkPathBuilder_DEFINED
