/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPathMeasure.h"

SkPathMeasure::SkPathMeasure() {}

SkPathMeasure::SkPathMeasure(const SkPath& path, bool forceClosed, SkScalar resScale)
    : fIter(path, forceClosed, resScale)
{
    fContour = fIter.next();
}

SkPathMeasure::~SkPathMeasure() {}

void SkPathMeasure::setPath(const SkPath* path, bool forceClosed) {
    fIter.reset(path ? *path : SkPath(), forceClosed);
    fContour = fIter.next();
}

SkScalar SkPathMeasure::getLength() {
    return fContour ? fContour->length() : 0;
}

bool SkPathMeasure::getPosTan(SkScalar distance, SkPoint* position, SkVector* tangent) {
    return fContour && fContour->getPosTan(distance, position, tangent);
}

bool SkPathMeasure::getMatrix(SkScalar distance, SkMatrix* matrix, MatrixFlags flags) {
    return fContour && fContour->getMatrix(distance, matrix, (SkContourMeasure::MatrixFlags)flags);
}

bool SkPathMeasure::getSegment(SkScalar startD, SkScalar stopD, SkPath* dst, bool startWithMoveTo) {
    return fContour && fContour->getSegment(startD, stopD, dst, startWithMoveTo);
}

bool SkPathMeasure::isClosed() {
    return fContour && fContour->isClosed();
}

bool SkPathMeasure::nextContour() {
    fContour = fIter.next();
    return !!fContour;
}

#ifdef SK_DEBUG
void SkPathMeasure::dump() {}
#endif
