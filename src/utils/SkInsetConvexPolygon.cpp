/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInsetConvexPolygon.h"

struct Segment {
    SkPoint p0, p1;
    SkVector normal;

    void compute_normal(SkScalar radius, SkScalar dir) {
        // compute perpendicular
        normal.fX = p0.fY - p1.fY;
        normal.fY = p1.fX - p0.fX;
        SkASSERT_RELEASE(normal.normalize());
        normal *= radius*dir;
    }

    void inset(Segment* segment) {
        segment->p0 = p0 + normal;
        segment->p1 = p1 + normal;
    }
};

// the assumption is that inputPolygon is well-formed (no incident verts, etc.)
bool SkInsetConvexPolygon(const SkTArray<SkPoint>& inputPolygon, SkScalar insetDistance,
                          SkTArray<SkPoint>* insetPolygon) {
    if (inputPolygon.count() < 3) {
        return false;
    }

    if (inputPolygon.count() == 3) {
        return false;
    }

    // set up
    insetPolygon->reset();
    Segment firstSegment = { inputPolygon[0], inputPolygon[1] };
    firstSegment.compute_normal(insetDistance, 1);
    Segment inset0;
    firstSegment.inset(&inset0);

    Segment secondSegment = { inputPolygon[1], inputPolygon[2] };
    secondSegment.compute_normal(insetDistance, 1);
    Segment inset1;
    secondSegment.inset(&inset1);

    // compute intersection of inset0 and inset1 and insert into insetPoly

    Segment thirdSegment = { inputPolygon[2], inputPolygon[3] };
    thirdSegment.compute_normal(insetDistance, 1);
    Segment inset2;
    thirdSegment.inset(&inset2);

    // compute intersection of inset1 and inset2 and insert into insetPoly

    Segment prevInset = inset2;

    for (int i = 3; i < inputPolygon.count() - 1; ++i) {
        Segment segment = { inputPolygon[i], inputPolygon[i+1] };
        segment.compute_normal(insetDistance, 1);
        Segment currInset;
        segment.inset(&currInset);

        // compute intersection of prevInset and currInset
        bool intersection;
        if (intersection) {
            // if right turn, continue;

            // if not right turn, just add new point

        } else {
            // if prev inset to right of curr, drop last point, set prev to prev-1 and check again
            
            // else continue;
        }

        prevInset = currInset;
    }

    // finish up
    // curr inset becomes first inset
    Segment segment = { inputPolygon[inputPolygon.count() - 1], inputPolygon[0] };
    segment.compute_normal(insetDistance, 1);
    Segment currInset;
    segment.inset(&currInset);

    // compute intersection of prevInset and currInset
    bool intersection;
    if (intersection) {
        // if right turn, drop first point (maybe never add it?) and continue;

        // if not right turn, just add new point
    } else {
        // if prev inset to right of curr, drop last point, set prev to prev-1 and check again

        // else continue;
    }

    prevInset = currInset;
}
