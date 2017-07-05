/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCoverageOpsBuilder_DEFINED
#define GrCCPRCoverageOpsBuilder_DEFINED

#include "GrBuffer.h"
#include "SkRefCnt.h"
#include "SkRect.h"

class GrCCPRCoverageOp;
class GrDrawOp;
class GrOnFlushResourceProvider;
class GrResourceProvider;
class SkMatrix;
class SkPath;
struct SkDCubic;
enum class SkCubicType;

/**
 * This class produces GrDrawOps that render coverage count masks for one or multiple paths. A path
 * is added to the current op in two steps:
 *
 *   1) parsePath(ScissorMode, viewMatrix, path, &devBounds, &devBounds45);
 *
 *   <client decides where to put the mask within an atlas, if wanted>
 *
 *   2) saveParsedPath(offsetX, offsetY, clipBounds);
 *
 * The client can then produce a GrDrawOp for all currently saved paths by calling either
 * createIntermediateOp() or finalize().
 */
class GrCCPRCoverageOpsBuilder {
public:
    // Indicates whether a path should enforce a scissor clip when rendering its mask.
    enum class ScissorMode : int {
        kNonScissored = 0,
        kScissored = 1
    };
    static constexpr int kNumScissorModes = 2;

    struct MaxPrimitives {
        int fMaxTriangles = 0;
        int fMaxQuadratics = 0;
        int fMaxCubics = 0;

        void operator+=(const MaxPrimitives&);
        int sum() const;
    };

    struct MaxBufferItems {
        int             fMaxFanPoints = 0;
        int             fMaxControlPoints = 0;
        MaxPrimitives   fMaxPrimitives[kNumScissorModes];
        int             fMaxPaths = 0;

        void operator+=(const MaxBufferItems&);
        void countPathItems(ScissorMode, const SkPath&);
    };

    GrCCPRCoverageOpsBuilder() : fScissorBatches(512) {
        SkDEBUGCODE(fPointsData = nullptr;)
        SkDEBUGCODE(fInstanceData = nullptr;)
    }

    bool init(GrOnFlushResourceProvider*, const MaxBufferItems&);

    void parsePath(ScissorMode, const SkMatrix&, const SkPath&, SkRect* devBounds,
                   SkRect* devBounds45);
    void saveParsedPath(int16_t offsetX, int16_t offsetY, const SkIRect& clipBounds);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT createIntermediateOp(SkISize drawBounds);
    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT finalize(SkISize drawBounds);

    class CoverageOp;
    class AccumulatingViewMatrix;

private:
    struct PrimitiveTallies {
        int fTriangles;
        int fQuadratics;
        int fSerpentines;
        int fLoops;

        PrimitiveTallies operator+(const PrimitiveTallies&) const;
        PrimitiveTallies operator-(const PrimitiveTallies&) const;
        int sum() const;
    };

    struct ScissorBatch {
        PrimitiveTallies   fInstanceCounts;
        SkIRect            fScissor;
    };

    void startContour(AccumulatingViewMatrix&, const SkPoint& anchorPoint);
    void fanTo(AccumulatingViewMatrix&, const SkPoint& pt);
    void quadraticTo(AccumulatingViewMatrix&, const SkPoint P[3]);
    void cubicTo(AccumulatingViewMatrix&, const SkPoint P[4]);
    void emitCubicSegment(AccumulatingViewMatrix&, SkCubicType, const SkDCubic&,
                          const SkPoint& ts0, const SkPoint& ts1);
    void closeContour();
    void emitHierarchicalFan(int indices[], int count);
    SkDEBUGCODE(void validate();)

    ScissorMode              fCurrScissorMode;
    PrimitiveTallies         fCurrPathIndices;
    int                      fCurrContourStartIdx;
    SkPoint                  fCurrPathSpaceAnchorPoint;

    sk_sp<GrBuffer>          fPointsBuffer;
    SkPoint*                 fPointsData;
    int                      fFanPtsIdx;
    int                      fControlPtsIdx;
    SkDEBUGCODE(int          fMaxFanPoints;)
    SkDEBUGCODE(int          fMaxControlPoints;)

    sk_sp<GrBuffer>          fInstanceBuffer;
    std::array<int, 4>*      fInstanceData;
    PrimitiveTallies         fBaseInstances[kNumScissorModes];
    PrimitiveTallies         fInstanceIndices[kNumScissorModes];

    SkTArray<ScissorBatch>   fScissorBatches;
};

inline void GrCCPRCoverageOpsBuilder::MaxBufferItems::operator+=(const MaxBufferItems& b) {
    fMaxFanPoints += b.fMaxFanPoints;
    fMaxControlPoints += b.fMaxControlPoints;
    fMaxPrimitives[0] += b.fMaxPrimitives[0];
    fMaxPrimitives[1] += b.fMaxPrimitives[1];
    fMaxPaths += b.fMaxPaths;
}

inline void GrCCPRCoverageOpsBuilder::MaxPrimitives::operator+=(const MaxPrimitives& b) {
    fMaxTriangles += b.fMaxTriangles;
    fMaxQuadratics += b.fMaxQuadratics;
    fMaxCubics += b.fMaxCubics;
}

inline int GrCCPRCoverageOpsBuilder::MaxPrimitives::sum() const {
    return fMaxTriangles + fMaxQuadratics + fMaxCubics;
}

#endif
