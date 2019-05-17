/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEdgeBuilder_DEFINED
#define SkEdgeBuilder_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkAnalyticEdge.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEdge.h"

class SkPath;

class SkEdgeBuilder {
public:
    int buildEdges(const SkPath& path,
                   const SkIRect* shiftedClip);

protected:
    SkEdgeBuilder() = default;
    virtual ~SkEdgeBuilder() = default;

    // In general mode we allocate pointers in fList and fEdgeList points to its head.
    // In polygon mode we preallocated edges contiguously in fAlloc and fEdgeList points there.
    void**              fEdgeList = nullptr;
    SkTDArray<void*>    fList;
    SkSTArenaAlloc<512> fAlloc;

    enum Combine {
        kNo_Combine,
        kPartial_Combine,
        kTotal_Combine
    };

private:
    int build    (const SkPath& path, const SkIRect* clip, bool clipToTheRight);
    int buildPoly(const SkPath& path, const SkIRect* clip, bool clipToTheRight);

    virtual char* allocEdges(size_t n, size_t* sizeof_edge) = 0;
    virtual SkRect recoverClip(const SkIRect&) const = 0;

    virtual void addLine (const SkPoint pts[]) = 0;
    virtual void addQuad (const SkPoint pts[]) = 0;
    virtual void addCubic(const SkPoint pts[]) = 0;
    virtual Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) = 0;
};

class SkBasicEdgeBuilder final : public SkEdgeBuilder {
public:
    explicit SkBasicEdgeBuilder(int clipShift) : fClipShift(clipShift) {}

    SkEdge** edgeList() { return (SkEdge**)fEdgeList; }

private:
    Combine combineVertical(const SkEdge* edge, SkEdge* last);

    char* allocEdges(size_t, size_t*) override;
    SkRect recoverClip(const SkIRect&) const override;

    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
    Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) override;

    const int fClipShift;
};

class SkAnalyticEdgeBuilder final : public SkEdgeBuilder {
public:
    SkAnalyticEdgeBuilder() {}

    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fEdgeList; }

private:
    Combine combineVertical(const SkAnalyticEdge* edge, SkAnalyticEdge* last);

    char* allocEdges(size_t, size_t*) override;
    SkRect recoverClip(const SkIRect&) const override;

    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
    Combine addPolyLine(SkPoint pts[], char* edge, char** edgePtr) override;
};
#endif
