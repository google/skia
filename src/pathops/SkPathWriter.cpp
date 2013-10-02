/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsPoint.h"
#include "SkPathWriter.h"

// wrap path to keep track of whether the contour is initialized and non-empty
SkPathWriter::SkPathWriter(SkPath& path)
    : fPathPtr(&path)
    , fCloses(0)
    , fMoves(0)
{
    init();
}

void SkPathWriter::close() {
    if (!fHasMove) {
        return;
    }
    bool callClose = isClosed();
    lineTo();
    if (fEmpty) {
        return;
    }
    if (callClose) {
#if DEBUG_PATH_CONSTRUCTION
        SkDebugf("path.close();\n");
#endif
        fPathPtr->close();
        fCloses++;
    }
    init();
}

void SkPathWriter::cubicTo(const SkPoint& pt1, const SkPoint& pt2, const SkPoint& pt3) {
    lineTo();
    if (fEmpty && AlmostEqualUlps(fDefer[0], pt1) && AlmostEqualUlps(pt1, pt2)
            && AlmostEqualUlps(pt2, pt3)) {
        deferredLine(pt3);
        return;
    }
    moveTo();
    fDefer[1] = pt3;
    nudge();
    fDefer[0] = fDefer[1];
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("path.cubicTo(%1.9g,%1.9g, %1.9g,%1.9g, %1.9g,%1.9g);\n",
            pt1.fX, pt1.fY, pt2.fX, pt2.fY, fDefer[1].fX, fDefer[1].fY);
#endif
    fPathPtr->cubicTo(pt1.fX, pt1.fY, pt2.fX, pt2.fY, fDefer[1].fX, fDefer[1].fY);
    fEmpty = false;
}

void SkPathWriter::deferredLine(const SkPoint& pt) {
    if (pt == fDefer[1]) {
        return;
    }
    if (changedSlopes(pt)) {
        lineTo();
        fDefer[0] = fDefer[1];
    }
    fDefer[1] = pt;
}

void SkPathWriter::deferredMove(const SkPoint& pt) {
    fMoved = true;
    fHasMove = true;
    fEmpty = true;
    fDefer[0] = fDefer[1] = pt;
}

void SkPathWriter::deferredMoveLine(const SkPoint& pt) {
    if (!fHasMove) {
        deferredMove(pt);
    }
    deferredLine(pt);
}

bool SkPathWriter::hasMove() const {
    return fHasMove;
}

void SkPathWriter::init() {
    fEmpty = true;
    fHasMove = false;
    fMoved = false;
}

bool SkPathWriter::isClosed() const {
    return !fEmpty && SkDPoint::ApproximatelyEqual(fFirstPt, fDefer[1]);
}

void SkPathWriter::lineTo() {
    if (fDefer[0] == fDefer[1]) {
        return;
    }
    moveTo();
    nudge();
    fEmpty = false;
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("path.lineTo(%1.9g,%1.9g);\n", fDefer[1].fX, fDefer[1].fY);
#endif
    fPathPtr->lineTo(fDefer[1].fX, fDefer[1].fY);
    fDefer[0] = fDefer[1];
}

const SkPath* SkPathWriter::nativePath() const {
    return fPathPtr;
}

void SkPathWriter::nudge() {
    if (fEmpty || !AlmostEqualUlps(fDefer[1].fX, fFirstPt.fX)
            || !AlmostEqualUlps(fDefer[1].fY, fFirstPt.fY)) {
        return;
    }
    fDefer[1] = fFirstPt;
}

void SkPathWriter::quadTo(const SkPoint& pt1, const SkPoint& pt2) {
    lineTo();
    if (fEmpty && AlmostEqualUlps(fDefer[0], pt1) && AlmostEqualUlps(pt1, pt2)) {
        deferredLine(pt2);
        return;
    }
    moveTo();
    fDefer[1] = pt2;
    nudge();
    fDefer[0] = fDefer[1];
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("path.quadTo(%1.9g,%1.9g, %1.9g,%1.9g);\n",
            pt1.fX, pt1.fY, fDefer[1].fX, fDefer[1].fY);
#endif
    fPathPtr->quadTo(pt1.fX, pt1.fY, fDefer[1].fX, fDefer[1].fY);
    fEmpty = false;
}

bool SkPathWriter::someAssemblyRequired() const {
    return fCloses < fMoves;
}

bool SkPathWriter::changedSlopes(const SkPoint& pt) const {
    if (fDefer[0] == fDefer[1]) {
        return false;
    }
    SkScalar deferDx = fDefer[1].fX - fDefer[0].fX;
    SkScalar deferDy = fDefer[1].fY - fDefer[0].fY;
    SkScalar lineDx = pt.fX - fDefer[1].fX;
    SkScalar lineDy = pt.fY - fDefer[1].fY;
    return deferDx * lineDy != deferDy * lineDx;
}

void SkPathWriter::moveTo() {
    if (!fMoved) {
        return;
    }
    fFirstPt = fDefer[0];
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("path.moveTo(%1.9g,%1.9g);\n", fDefer[0].fX, fDefer[0].fY);
#endif
    fPathPtr->moveTo(fDefer[0].fX, fDefer[0].fY);
    fMoved = false;
    fMoves++;
}
