/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEdgeBuilder_DEFINED
#define SkEdgeBuilder_DEFINED

#include "SkArenaAlloc.h"
#include "SkRect.h"
#include "SkTDArray.h"

struct SkEdge;
struct SkAnalyticEdge;
class SkEdgeClipper;
class SkPath;

class SkEdgeBuilder {
public:
    SkEdgeBuilder();

    // returns the number of built edges. The array of those edge pointers
    // is returned from edgeList().
    int build(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight,
              bool analyticAA = false);

    SkEdge** edgeList() { return (SkEdge**)fEdgeList; }
    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fEdgeList; }

private:
    enum Combine {
        kNo_Combine,
        kPartial_Combine,
        kTotal_Combine
    };

    Combine CombineVertical(const SkEdge* edge, SkEdge* last);
    Combine CombineVertical(const SkAnalyticEdge* edge, SkAnalyticEdge* last);
    Combine checkVertical(const SkEdge* edge, SkEdge** edgePtr);
    Combine checkVertical(const SkAnalyticEdge* edge, SkAnalyticEdge** edgePtr);
    bool vertical_line(const SkEdge* edge);
    bool vertical_line(const SkAnalyticEdge* edge);

    char                fStorage[512];
    SkArenaAlloc        fAlloc{fStorage};
    SkTDArray<void*>    fList;

    /*
     *  If we're in general mode, we allcoate the pointers in fList, and this
     *  will point at fList.begin(). If we're in polygon mode, fList will be
     *  empty, as we will have preallocated room for the pointers in fAlloc's
     *  block, and fEdgeList will point into that.
     */
    void**      fEdgeList;

    int         fShiftUp;
    bool        fAnalyticAA;

public:
    void addLine(const SkPoint pts[]);
    void addQuad(const SkPoint pts[]);
    void addCubic(const SkPoint pts[]);
    void addClipper(SkEdgeClipper*);

    int buildPoly(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight);
};

#endif
