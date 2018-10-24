/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEdgeBuilder_DEFINED
#define SkEdgeBuilder_DEFINED

#include "SkAnalyticEdge.h"
#include "SkArenaAlloc.h"
#include "SkEdge.h"
#include "SkRect.h"
#include "SkTDArray.h"

class SkPath;

class SkEdgeBuilder {
public:
    int buildEdges(const SkPath& path,
                   const SkIRect* shiftedClip);

protected:
    SkEdgeBuilder() = default;
    virtual ~SkEdgeBuilder() = default;

    SkTDArray<void*>    fList;
    SkSTArenaAlloc<512> fAlloc;

private:
    int build(const SkPath& path, const SkIRect* clip, bool clipToTheRight);

    virtual SkRect recoverClip(const SkIRect&) const = 0;
    virtual bool chopCubics() const = 0;

    virtual void addLine (const SkPoint pts[]) = 0;
    virtual void addQuad (const SkPoint pts[]) = 0;
    virtual void addCubic(const SkPoint pts[]) = 0;
};

class SkBasicEdgeBuilder final : public SkEdgeBuilder {
public:
    explicit SkBasicEdgeBuilder(int clipShift) : fClipShift(clipShift) {}

    SkEdge** edgeList() { return (SkEdge**)fList.begin(); }

private:
    enum Combine { kNo_Combine, kPartial_Combine, kTotal_Combine };
    Combine combineVertical(const SkEdge* edge, SkEdge* last);

    SkRect recoverClip(const SkIRect&) const override;
    bool chopCubics() const override { return true; }

    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;

    const int fClipShift;
};

class SkAnalyticEdgeBuilder final : public SkEdgeBuilder {
public:
    SkAnalyticEdgeBuilder() {}

    SkAnalyticEdge** analyticEdgeList() { return (SkAnalyticEdge**)fList.begin(); }

private:
    enum Combine { kNo_Combine, kPartial_Combine, kTotal_Combine };
    Combine combineVertical(const SkAnalyticEdge* edge, SkAnalyticEdge* last);

    SkRect recoverClip(const SkIRect&) const override;
    bool chopCubics() const override { return true; }

    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
};

class SkBezierEdgeBuilder final : public SkEdgeBuilder {
public:
    SkBezierEdgeBuilder() {}

    SkBezier** bezierList() { return (SkBezier**)fList.begin(); }

private:
    SkRect recoverClip(const SkIRect&) const override;
    bool chopCubics() const override { return false; }

    void addLine (const SkPoint pts[]) override;
    void addQuad (const SkPoint pts[]) override;
    void addCubic(const SkPoint pts[]) override;
};

#endif
