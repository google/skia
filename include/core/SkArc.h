/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArc_DEFINED
#define SkArc_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"

// Represents an arc along an oval boundary, or a closed wedge of the oval.
struct SkArc {
    enum class Type : bool {
        kArc,   // An arc along the perimeter of the oval
        kWedge  // A closed wedge that includes the oval's center
    };

    SkArc() = default;
    SkArc(const SkArc& arc) = default;
    SkArc& operator=(const SkArc& arc) = default;

    const SkRect& oval() const { return fOval; }
    SkScalar startAngle() const { return fStartAngle; }
    SkScalar sweepAngle() const { return fSweepAngle; }
    bool isWedge() const { return fType == Type::kWedge; }

    friend bool operator==(const SkArc& a, const SkArc& b) {
        return a.fOval == b.fOval && a.fStartAngle == b.fStartAngle &&
               a.fSweepAngle == b.fSweepAngle && a.fType == b.fType;
    }

    friend bool operator!=(const SkArc& a, const SkArc& b) { return !(a == b); }

    // Preferred factory that explicitly states which type of arc
    static SkArc Make(const SkRect& oval,
                      SkScalar startAngleDegrees,
                      SkScalar sweepAngleDegrees,
                      Type type) {
        return SkArc(oval, startAngleDegrees, sweepAngleDegrees, type);
    }

    // Deprecated factory to assist with legacy code based on `useCenter`
    static SkArc Make(const SkRect& oval,
                      SkScalar startAngleDegrees,
                      SkScalar sweepAngleDegrees,
                      bool useCenter) {
        return SkArc(
                oval, startAngleDegrees, sweepAngleDegrees, useCenter ? Type::kWedge : Type::kArc);
    }

    // Bounds of oval containing the arc.
    SkRect   fOval = SkRect::MakeEmpty();

    // Angle in degrees where the arc begins. Zero means horizontally to the right.
    SkScalar fStartAngle = 0;
    // Sweep angle in degrees; positive is clockwise.
    SkScalar fSweepAngle = 0;

    Type     fType = Type::kArc;

private:
    SkArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, Type type)
            : fOval(oval), fStartAngle(startAngle), fSweepAngle(sweepAngle), fType(type) {}
};

#endif
