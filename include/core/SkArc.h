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
    // Bounds of oval containing the arc.
    SkRect   fOval;

    // Angle in degrees where the arc begins. Zero means horizontally to the right.
    SkScalar fStartAngle;
    // Sweep angle in degrees; positive is clockwise.
    SkScalar fSweepAngle;

    // If true, draws a wedge that includes lines from the oval's center to the arc end points.
    // If false, just draws the arc along the oval's perimeter.
    bool     fUseCenter;
};

#endif
