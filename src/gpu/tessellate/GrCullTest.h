/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCullTest_DEFINED
#define GrCullTest_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkVx.h"
#include "src/gpu/GrVx.h"

// This class determines whether the given local-space points will be contained in the cull bounds
// post transform. For the versions that take >1 point, it returns whether any region of their
// device-space bounding box will be in the cull bounds.
//
// NOTE: Our view matrix is not a normal matrix. M*p maps to the float4 [x, y, -x, -y] in device
// space. We do this to aid in quick bounds calculations. The matrix also does not have a
// translation element. Instead we unapply the translation to the cull bounds ahead of time.
class GrCullTest {
public:
    GrCullTest() = default;

    GrCullTest(const SkRect& devCullBounds, const SkMatrix& m) {
        this->set(devCullBounds, m);
    }

    void set(const SkRect& devCullBounds, const SkMatrix& m) {
        SkASSERT(!m.hasPerspective());
        // [fMatX, fMatY] maps path coordinates to the float4 [x, y, -x, -y] in device space.
        fMatX = {m.getScaleX(), m.getSkewY(), -m.getScaleX(), -m.getSkewY()};
        fMatY = {m.getSkewX(), m.getScaleY(), -m.getSkewX(), -m.getScaleY()};
        // Store the cull bounds as [l, t, -r, -b] for faster math.
        // Also subtract the matrix translate from the cull bounds ahead of time, rather than adding
        // it to every point every time we test.
        fCullBounds = {devCullBounds.fLeft - m.getTranslateX(),
                       devCullBounds.fTop - m.getTranslateY(),
                       m.getTranslateX() - devCullBounds.fRight,
                       m.getTranslateY() - devCullBounds.fBottom};
    }

    // Returns whether M*p will be in the viewport.
    bool isVisible(SkPoint p) const {
        // devPt = [x, y, -x, -y] in device space.
        auto devPt = grvx::fast_madd<4>(fMatX, p.fX, fMatY * p.fY);
        // i.e., l < x && t < y && r > x && b > y.
        return skvx::all(fCullBounds < devPt);
    }

    // Returns whether any region of the bounding box of M * p0..2 will be in the viewport.
    bool areVisible3(const SkPoint p[3]) const {
        // Transform p0..2 to device space.
        auto val0 = fMatY * p[0].fY;
        auto val1 = fMatY * p[1].fY;
        auto val2 = fMatY * p[2].fY;
        val0 = grvx::fast_madd<4>(fMatX, p[0].fX, val0);
        val1 = grvx::fast_madd<4>(fMatX, p[1].fX, val1);
        val2 = grvx::fast_madd<4>(fMatX, p[2].fX, val2);
        // At this point: valN = {xN, yN, -xN, -yN} in device space.

        // Find the device-space bounding box of p0..2.
        val0 = skvx::max(val0, val1);
        val0 = skvx::max(val0, val2);
        // At this point: val0 = [r, b, -l, -t] of the device-space bounding box of p0..2.

        // Does fCullBounds intersect the device-space bounding box of p0..2?
        // i.e., l0 < r1 && t0 < b1 && r0 > l1 && b0 > t1.
        return skvx::all(fCullBounds < val0);
    }

    // Returns whether any region of the bounding box of M * p0..3 will be in the viewport.
    bool areVisible4(const SkPoint p[4]) const {
        // Transform p0..3 to device space.
        auto val0 = fMatY * p[0].fY;
        auto val1 = fMatY * p[1].fY;
        auto val2 = fMatY * p[2].fY;
        auto val3 = fMatY * p[3].fY;
        val0 = grvx::fast_madd<4>(fMatX, p[0].fX, val0);
        val1 = grvx::fast_madd<4>(fMatX, p[1].fX, val1);
        val2 = grvx::fast_madd<4>(fMatX, p[2].fX, val2);
        val3 = grvx::fast_madd<4>(fMatX, p[3].fX, val3);
        // At this point: valN = {xN, yN, -xN, -yN} in device space.

        // Find the device-space bounding box of p0..3.
        val0 = skvx::max(val0, val1);
        val2 = skvx::max(val2, val3);
        val0 = skvx::max(val0, val2);
        // At this point: val0 = [r, b, -l, -t] of the device-space bounding box of p0..3.

        // Does fCullBounds intersect the device-space bounding box of p0..3?
        // i.e., l0 < r1 && t0 < b1 && r0 > l1 && b0 > t1.
        return skvx::all(fCullBounds < val0);
    }

private:
    // [fMatX, fMatY] maps path coordinates to the float4 [x, y, -x, -y] in device space.
    grvx::float4 fMatX;
    grvx::float4 fMatY;
    grvx::float4 fCullBounds;  // [l, t, -r, -b]
};

#endif
