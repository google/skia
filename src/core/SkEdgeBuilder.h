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
#include "SkEdge.h"
#include "SkAnalyticEdge.h"

struct SkEdge;
struct SkAnalyticEdge;
class SkEdgeClipper;
class SkPath;

class SkEdgeBuilder {
public:
    enum EdgeType {
        // Used in supersampling or non-AA scan coverter; it stores only integral y coordinates.
        kEdge,

        // Used in Analytic AA scan converter; it uses SkFixed to store fractional y.
        kAnalyticEdge,

        // Used in Delta AA scan converter; it's a super-light wrapper of SkPoint, which can then be
        // used to construct SkAnalyticEdge (kAnalyticEdge) later. We use kBezier to save the memory
        // allocation time (a SkBezier is much lighter than SkAnalyticEdge or SkEdge). Note that
        // Delta AA only has to deal with one SkAnalyticEdge at a time (whereas Analytic AA has to
        // deal with all SkAnalyticEdges at the same time). Thus for Delta AA, we only need to
        // allocate memory for n SkBeziers and 1 SkAnalyticEdge. (Analytic AA need to allocate
        // memory for n SkAnalyticEdges.)
        kBezier
    };

    SkEdgeBuilder(EdgeType, int shiftEdgesUp);

    int buildEdges(const SkPath& path,
                   const SkIRect* shiftedClip,
                   bool pathContainedInClip);

    SkEdge**                 edgeList() { return         (SkEdge**)fEdgeList; }
    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fEdgeList; }
    SkBezier**             bezierList() { return       (SkBezier**)fEdgeList; }

private:
    enum Combine {
        kNo_Combine,
        kPartial_Combine,
        kTotal_Combine
    };

    int build    (const SkPath& path, const SkIRect* clip, bool clipToTheRight);
    int buildPoly(const SkPath& path, const SkIRect* clip, bool clipToTheRight);

    Combine combineVertical(const SkEdge* edge, SkEdge* last);
    Combine   checkVertical(const SkEdge* edge, SkEdge** edgePtr);
    bool       verticalLine(const SkEdge* edge);

    Combine combineVertical(const SkAnalyticEdge* edge, SkAnalyticEdge* last);
    Combine   checkVertical(const SkAnalyticEdge* edge, SkAnalyticEdge** edgePtr);
    bool       verticalLine(const SkAnalyticEdge* edge);

    void addLine(const SkPoint pts[]);
    void addQuad(const SkPoint pts[]);
    void addCubic(const SkPoint pts[]);
    void addClipper(SkEdgeClipper*);
    void addPolyLine(SkPoint pts[], char* &edge, size_t edgeSize, char** &edgePtr);


    // In general mode we allocate pointers in fList and this points to its head.
    // In polygon mode we preallocated pointers in fAlloc and this points there.
    void**              fEdgeList;
    SkSTArenaAlloc<512> fAlloc;
    SkTDArray<void*>    fList;

    const EdgeType    fEdgeType;
    const int         fShiftUp;
    bool              fIsFinite;
};

#endif
