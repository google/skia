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
    int buildEdges(const SkPath& path,
                   const SkIRect* shiftedClip,
                   bool pathContainedInClip);

protected:
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
    virtual ~SkEdgeBuilder() = default;

    // TODO: move data to subclasses

    // In general mode we allocate pointers in fList and this points to its head.
    // In polygon mode we preallocated pointers in fAlloc and this points there.
    void**              fEdgeList;
    SkSTArenaAlloc<512> fAlloc;
    SkTDArray<void*>    fList;

    const EdgeType    fEdgeType;
    const int         fShiftUp;
    bool              fIsFinite;

    enum Combine {
        kNo_Combine,
        kPartial_Combine,
        kTotal_Combine
    };

private:
    int build    (const SkPath& path, const SkIRect* clip, bool clipToTheRight);
    int buildPoly(const SkPath& path, const SkIRect* clip, bool clipToTheRight);
    void addClipper(SkEdgeClipper*);

    virtual char* allocEdges(size_t n, size_t* sizeof_edge) = 0;

    virtual void addLine (const SkPoint pts[]) = 0;
    virtual void addQuad (const SkPoint pts[]) = 0;
    virtual void addCubic(const SkPoint pts[]) = 0;
    virtual Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) = 0;
};

class SkBasicEdgeBuilder final : public SkEdgeBuilder {
public:
    explicit SkBasicEdgeBuilder(int shiftEdgesUp) :
        SkEdgeBuilder(SkEdgeBuilder::kEdge, shiftEdgesUp) {}

    SkEdge** edgeList() { return (SkEdge**)fEdgeList; }

private:
    Combine combineVertical(const SkEdge* edge, SkEdge* last);

    char* allocEdges(size_t, size_t*) override;
    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
    SkEdgeBuilder::Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) override;
};

class SkAnalyticEdgeBuilder final : public SkEdgeBuilder {
public:
    SkAnalyticEdgeBuilder() : SkEdgeBuilder(SkEdgeBuilder::kAnalyticEdge, 0) {}

    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fEdgeList; }

private:
    Combine combineVertical(const SkAnalyticEdge* edge, SkAnalyticEdge* last);

    char* allocEdges(size_t, size_t*) override;
    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
    SkEdgeBuilder::Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) override;
};

class SkBezierEdgeBuilder final : public SkEdgeBuilder {
public:
    SkBezierEdgeBuilder() : SkEdgeBuilder(SkEdgeBuilder::kBezier, 0) {}

    SkBezier** bezierList() { return (SkBezier**)fEdgeList; }

private:
    char* allocEdges(size_t, size_t*) override;
    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
    SkEdgeBuilder::Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) override;
};


#endif
