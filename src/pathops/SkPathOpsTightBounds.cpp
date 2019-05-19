/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkOpEdgeBuilder.h"
#include "src/pathops/SkPathOpsCommon.h"

bool TightBounds(const SkPath& path, SkRect* result) {
    SkPath::RawIter iter(path);
    SkRect moveBounds = { SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin };
    bool wellBehaved = true;
    SkPath::Verb verb;
    do {
        SkPoint pts[4];
        verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                moveBounds.fLeft = SkTMin(moveBounds.fLeft, pts[0].fX);
                moveBounds.fTop = SkTMin(moveBounds.fTop, pts[0].fY);
                moveBounds.fRight = SkTMax(moveBounds.fRight, pts[0].fX);
                moveBounds.fBottom = SkTMax(moveBounds.fBottom, pts[0].fY);
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                if (!wellBehaved) {
                    break;
                }
                wellBehaved &= between(pts[0].fX, pts[1].fX, pts[2].fX);
                wellBehaved &= between(pts[0].fY, pts[1].fY, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                if (!wellBehaved) {
                    break;
                }
                wellBehaved &= between(pts[0].fX, pts[1].fX, pts[3].fX);
                wellBehaved &= between(pts[0].fY, pts[1].fY, pts[3].fY);
                wellBehaved &= between(pts[0].fX, pts[2].fX, pts[3].fX);
                wellBehaved &= between(pts[0].fY, pts[2].fY, pts[3].fY);
                break;
            default:
                break;
        }
    } while (verb != SkPath::kDone_Verb);
    if (wellBehaved) {
        *result = path.getBounds();
        return true;
    }
    SkSTArenaAlloc<4096> allocator;  // FIXME: constant-ize, tune
    SkOpContour contour;
    SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&contour);
    SkOpGlobalState globalState(contourList, &allocator  SkDEBUGPARAMS(false)
            SkDEBUGPARAMS(nullptr));
    // turn path into list of segments
    SkOpEdgeBuilder builder(path, contourList, &globalState);
    if (!builder.finish()) {
        return false;
    }
    if (!SortContourList(&contourList, false, false)) {
        *result = moveBounds;
        return true;
    }
    SkOpContour* current = contourList;
    SkPathOpsBounds bounds = current->bounds();
    while ((current = current->next())) {
        bounds.add(current->bounds());
    }
    *result = bounds;
    if (!moveBounds.isEmpty()) {
        result->join(moveBounds);
    }
    return true;
}
