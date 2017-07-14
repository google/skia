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
#include "ccpr/GrCCPRCoverageProcessor.h"

class GrCCPRCoverageOp;
class GrDrawOp;
class GrOnFlushResourceProvider;
class GrResourceProvider;
class SkMatrix;
class SkPath;
struct SkDCubic;
enum class SkCubicType;

/**
 * This class produces GrDrawOps that render coverage count masks and atlases. A path is added to
 * the current op in two steps:
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
    // Indicates whether a path should enforce a scissor clip when rendering its mask. (Specified
    // as an int because these values get used directly as indices into arrays.)
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

    // Parses an SkPath into a temporary staging area. The path will not yet be included in the next
    // Op until there is a matching call to saveParsedPath.
    //
    // Returns two tight bounding boxes: device space and "45 degree" (| 1 -1 | * devCoords) space.
    //                                                                 | 1  1 |
    void parsePath(ScissorMode, const SkMatrix&, const SkPath&, SkRect* devBounds,
                   SkRect* devBounds45);

    // Commits the currently-parsed path from the staging area to the GPU buffers and next Op.
    // Accepts an optional post-device-space translate for placement in an atlas.
    void saveParsedPath(const SkIRect& clippedDevIBounds,
                        int16_t atlasOffsetX, int16_t atlasOffsetY);

    // Flushes all currently-saved paths to a GrDrawOp and leaves the GPU buffers open to accept
    // new paths (e.g. for when an atlas runs out of space).
    // NOTE: if there is a parsed path in the staging area, it will not be included. But the client
    // may still call saveParsedPath to include it in a future Op.
    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT createIntermediateOp(SkISize drawBounds);

    // Flushes the remaining saved paths to a final GrDrawOp and closes off the GPU buffers. This
    // must be called before attempting to draw any Ops produced by this class.
    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT finalize(SkISize drawBounds);

    class CoverageOp;
    class AccumulatingViewMatrix;

private:
    using PrimitiveInstance = GrCCPRCoverageProcessor::PrimitiveInstance;

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
    void emitHierarchicalFan(int32_t indices[], int count);
    SkDEBUGCODE(void validate();)

    ScissorMode              fCurrScissorMode;
    PrimitiveTallies         fCurrPathIndices;
    int32_t                  fCurrContourStartIdx;
    SkPoint                  fCurrPathSpaceAnchorPoint;

    sk_sp<GrBuffer>          fPointsBuffer;
    SkPoint*                 fPointsData;
    int32_t                  fFanPtsIdx;
    int32_t                  fControlPtsIdx;
    SkDEBUGCODE(int          fMaxFanPoints;)
    SkDEBUGCODE(int          fMaxControlPoints;)

    sk_sp<GrBuffer>          fInstanceBuffer;
    PrimitiveInstance*       fInstanceData;
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
