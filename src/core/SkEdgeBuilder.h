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

    // static constexpr int kEdgeSizes[3] = {sizeof(SkEdge), sizeof(SkAnalyticEdge), sizeof(SkBezier)};

    SkEdgeBuilder();

    // returns the number of built edges. The array of those edge pointers
    // is returned from edgeList().
    int build(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight,
              EdgeType edgeType = kEdge);

    int build_edges(const SkPath& path, const SkIRect* shiftedClip,
            int shiftEdgesUp, bool pathContainedInClip, EdgeType edgeType = kEdge);

    SkEdge** edgeList() { return (SkEdge**)fEdgeList; }
    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fEdgeList; }
    SkBezier** bezierList() { return (SkBezier**)fEdgeList; }

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

    SkSTArenaAlloc<512> fAlloc;
    SkTDArray<void*>    fList;

    /*
     *  If we're in general mode, we allcoate the pointers in fList, and this
     *  will point at fList.begin(). If we're in polygon mode, fList will be
     *  empty, as we will have preallocated room for the pointers in fAlloc's
     *  block, and fEdgeList will point into that.
     */
    void**      fEdgeList;

    int         fShiftUp;
    EdgeType    fEdgeType;

public:
    void addLine(const SkPoint pts[]);
    void addQuad(const SkPoint pts[]);
    void addCubic(const SkPoint pts[]);
    void addClipper(SkEdgeClipper*);

    EdgeType edgeType() const { return fEdgeType; }

    int buildPoly(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight);

    inline void addPolyLine(SkPoint pts[], char* &edge, size_t edgeSize, char** &edgePtr,
            int shiftUp) {
        if (fEdgeType == kBezier) {
            if (((SkLine*)edge)->set(pts)) {
                *edgePtr++ = edge;
                edge += edgeSize;
            }
            return;
        }
        bool analyticAA = fEdgeType == kAnalyticEdge;
        bool setLineResult = analyticAA ?
                ((SkAnalyticEdge*)edge)->setLine(pts[0], pts[1]) :
                ((SkEdge*)edge)->setLine(pts[0], pts[1], shiftUp);
        if (setLineResult) {
            Combine combine = analyticAA ?
                    checkVertical((SkAnalyticEdge*)edge, (SkAnalyticEdge**)edgePtr) :
                    checkVertical((SkEdge*)edge, (SkEdge**)edgePtr);
            if (kNo_Combine == combine) {
                *edgePtr++ = edge;
                edge += edgeSize;
            } else if (kTotal_Combine == combine) {
                --edgePtr;
            }
        }
    }
};

#endif
