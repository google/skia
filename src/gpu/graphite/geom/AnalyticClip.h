/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticClip_DEFINED
#define skgpu_graphite_geom_AnalyticClip_DEFINED

#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

/**
 * Represents a rect or rrect clip with any non-rect corners having the same circular radii.
 */
struct CircularRRectClip {
    // Indicate which edges are adjacent to circular corners.
    enum EdgeFlags {
        kLeft_EdgeFlag   = 0b0001,
        kTop_EdgeFlag    = 0b0010,
        kRight_EdgeFlag  = 0b0100,
        kBottom_EdgeFlag = 0b1000,

        kNone_EdgeFlag   = 0b0000,
        kAll_EdgeFlag    = 0b1111,
    };
    Rect     fBounds;           // Bounds of clip
    float    fRadius = 0;       // Circular radius, if any
    uint32_t fEdgeFlags = kNone_EdgeFlag;
    bool     fInverted = false;

    bool isEmpty() const { return fBounds.isEmptyNegativeOrNaN(); }
    SkRect edgeSelectRect() const {
        return { fEdgeFlags & kLeft_EdgeFlag   ? 1.f : 0.f,
                 fEdgeFlags & kTop_EdgeFlag    ? 1.f : 0.f,
                 fEdgeFlags & kRight_EdgeFlag  ? 1.f : 0.f,
                 fEdgeFlags & kBottom_EdgeFlag ? 1.f : 0.f };
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_AnalyticClip_DEFINED
