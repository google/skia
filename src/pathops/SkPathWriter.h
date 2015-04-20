/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathWriter_DEFINED
#define SkPathWriter_DEFINED

#include "SkPath.h"

class SkPathWriter {
public:
    SkPathWriter(SkPath& path);
    void close();
    void conicTo(const SkPoint& pt1, const SkPoint& pt2, SkScalar weight);
    void cubicTo(const SkPoint& pt1, const SkPoint& pt2, const SkPoint& pt3);
    void deferredLine(const SkPoint& pt);
    void deferredMove(const SkPoint& pt);
    void deferredMoveLine(const SkPoint& pt);
    bool hasMove() const;
    void init();
    bool isClosed() const;
    bool isEmpty() const { return fEmpty; }
    void lineTo();
    const SkPath* nativePath() const;
    void nudge();
    void quadTo(const SkPoint& pt1, const SkPoint& pt2);
    bool someAssemblyRequired() const;

private:
    bool changedSlopes(const SkPoint& pt) const;
    void moveTo();

    SkPath* fPathPtr;
    SkPoint fDefer[2];
    SkPoint fFirstPt;
    int fCloses;
    int fMoves;
    bool fEmpty;
    bool fHasMove;
    bool fMoved;
};

#endif /* defined(__PathOps__SkPathWriter__) */
