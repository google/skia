/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EdgeAAQuad_DEFINED
#define skgpu_graphite_geom_EdgeAAQuad_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <cstdint>

namespace skgpu::graphite {

/**
 * EdgeAAQuad contains (x,y) coordinates for the four corners of a quadrilateral, assumed to be
 * convex and in a consistent winding (CW vs. CCW is fine). Locally, the vertices are ordered
 * "top-left", "top-right", "bottom-right", "bottom-left". The edges are in order left (p0-p3),
 * top (p1-p0), right (p2-p1), and bottom (p3-p2).
 */
class EdgeAAQuad {
public:
    // SkEnumBitMask<Flags> is a typesafe equivalent to SkCanvas::QuadAAFlags.
    enum class Flags : uint8_t {
        kLeft   = 0b0001,
        kTop    = 0b0010,
        kRight  = 0b0100,
        kBottom = 0b1000,

        kNone   = 0b0000,
        kAll    = 0b1111,
    };

    EdgeAAQuad() = delete;

    EdgeAAQuad(const SkRect& rect, SkEnumBitMask<Flags> edgeFlags)
            : fXs{rect.fLeft, rect.fRight, rect.fRight, rect.fLeft}
            , fYs{rect.fTop, rect.fTop, rect.fBottom, rect.fBottom}
            , fEdgeFlags(edgeFlags)
            , fIsRect(true) {}
    EdgeAAQuad(const Rect& rect, SkEnumBitMask<Flags> edgeFlags)
            : fXs{skvx::shuffle<0,2,2,0>(rect.ltrb())}
            , fYs{skvx::shuffle<1,1,3,3>(rect.ltrb())}
            , fEdgeFlags(edgeFlags)
            , fIsRect(true) {}
    EdgeAAQuad(const SkPoint points[4], SkEnumBitMask<Flags> edgeFlags)
            : fXs{points[0].fX, points[1].fX, points[2].fX, points[3].fX}
            , fYs{points[0].fY, points[1].fY, points[2].fY, points[3].fY}
            , fEdgeFlags(edgeFlags)
            , fIsRect(false) {}
    EdgeAAQuad(const skvx::float4& xs, const skvx::float4& ys, SkEnumBitMask<Flags> edgeFlags)
            : fXs(xs)
            , fYs(ys)
            , fEdgeFlags(edgeFlags)
            , fIsRect(false) {}

    // The bounding box of the quadrilateral (not counting any outsetting for anti-aliasing).
    Rect bounds() const {
        if (fIsRect) {
            return Rect({fXs[0], fYs[0]}, {fXs[2], fYs[2]});
        }

        Rect p0p1 = Rect::LTRB(skvx::shuffle<0,2,1,3>(skvx::float4(fXs.lo, fYs.lo))).makeSorted();
        Rect p2p3 = Rect::LTRB(skvx::shuffle<0,2,1,3>(skvx::float4(fXs.hi, fYs.hi))).makeSorted();
        return p0p1.makeJoin(p2p3);
    }

    // Access the individual elements of the quad data.
    const skvx::float4& xs() const { return fXs; }
    const skvx::float4& ys() const { return fYs; }
    SkEnumBitMask<Flags> edgeFlags() const { return fEdgeFlags; }

    bool isRect() const { return fIsRect; }

private:
    skvx::float4 fXs;
    skvx::float4 fYs;
    SkEnumBitMask<Flags> fEdgeFlags;
    bool fIsRect;
};

SK_MAKE_BITMASK_OPS(EdgeAAQuad::Flags)

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_EdgeAAQuad_DEFINED
