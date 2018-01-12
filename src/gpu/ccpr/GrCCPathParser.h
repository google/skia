/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCoverageOp_DEFINED
#define GrCCPRCoverageOp_DEFINED

#include "GrMesh.h"
#include "GrNonAtomicRef.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "ccpr/GrCCPRCoverageProcessor.h"
#include "ccpr/GrCCPRGeometry.h"
#include "ops/GrDrawOp.h"

class GrCCPRAtlas;
class GrOnFlushResourceProvider;
class SkMatrix;
class SkPath;

/**
 * This class produces BLAH BLAH BLAH that render coverage count masks and atlases. A path is
 * added to the current op in two steps:
 *
 *   1) parsePath(ScissorMode, viewMatrix, path, &devBounds, &devBounds45);
 *
 *   <client decides where to put the mask within an atlas, if wanted>
 *
 *   2) saveParsedPath(offsetX, offsetY, clipBounds);
 *
 * The client can flush the currently saved paths to a GrCCPRCoverageOp by calling emitOp, and
 * retrieve all emitted ops after calling finalize().
 */
class GrCCPathParser : public GrNonAtomicRef<GrCCPathParser> {
public:
    // Indicates whether a path should enforce a scissor clip when rendering its mask. (Specified
    // as an int because these values get used directly as indices into arrays.)
    enum class ScissorMode : int {
        kNonScissored = 0,
        kScissored = 1
    };
    static constexpr int kNumScissorModes = 2;

    using PrimitiveTallies = GrCCPRGeometry::PrimitiveTallies;

    GrCCPathParser(int maxTotalPaths, int maxPathPoints, int numSkPoints, int numSkVerbs)
            : fPathsInfo(maxTotalPaths)
            , fLocalDevPtsBuffer(maxPathPoints + 1) // Overallocate by one point to accomodate for
                                                    // overflow with Sk4f. (See parsePath.)
            , fGeometry(numSkPoints, numSkVerbs)
            , fInstanceCounts{PrimitiveTallies(), PrimitiveTallies()} {}

    ~GrCCPathParser() {
        // Enforce the contract that the client always calls saveParsedPath or discardParsedPath.
        SkASSERT(!fParsingPath);
    }

    const PrimitiveTallies* instanceCounts() const { return fInstanceCounts; }

    // Parses an SkPath into a temporary staging area. The path will not yet be included in the next
    // Op unless there is a matching call to saveParsedPath. The user must complement this with a
    // following call to either saveParsedPath or discardParsedPath.
    //
    // Returns two tight bounding boxes: device space and "45 degree" (| 1 -1 | * devCoords) space.
    //                                                                 | 1  1 |
    void parsePath(const SkMatrix&, const SkPath&, SkRect* devBounds, SkRect* devBounds45);

    // Parses a device-space SkPath into a temporary staging area. The path will not yet be included
    // in the next Op unless there is a matching call to saveParsedPath. The user must complement
    // this with a following call to either saveParsedPath or discardParsedPath.
    void parseDeviceSpacePath(const SkPath&);

    // Commits the currently-parsed path from staging to the next Op, and specifies whether the mask
    // should be rendered with a scissor clip in effect. Accepts an optional post-device-space
    // translate for placement in an atlas.
    const PrimitiveTallies& saveParsedPath(ScissorMode, const SkIRect& clippedDevIBounds,
                                           int16_t atlasOffsetX, int16_t atlasOffsetY);
    void discardParsedPath();

    // Blah blah blah.
    bool finalize(GrOnFlushResourceProvider*);

    struct ScissorBatch {
        ScissorBatch(const PrimitiveTallies& instanceCounts, const SkIRect& scissor)
                : fInstanceCounts(instanceCounts), fScissor(scissor) {}
        PrimitiveTallies fInstanceCounts;
        SkIRect fScissor;
    };

    void drawCoverageCount(GrOpFlushState*, const SkIRect& drawBounds, const GrPipeline&,
                           const PrimitiveTallies startIndices[kNumScissorModes],
                           const PrimitiveTallies& unscissoredInstanceCounts,
                           const SkTArray<ScissorBatch, true>&) const;

private:
    // Every kBeginPath verb has a corresponding PathInfo entry.
    struct PathInfo {
        ScissorMode fScissorMode;
        int16_t fAtlasOffsetX, fAtlasOffsetY;
    };

    void parsePath(const SkPath&, const SkPoint* deviceSpacePts);
    void endContourIfNeeded(bool insideContour);

    void drawRenderPass(GrCCPRCoverageProcessor::RenderPass, GrOpFlushState*,
                        const SkIRect& drawBounds, const GrPipeline&,
                        const PrimitiveTallies startIndices[kNumScissorModes],
                        const PrimitiveTallies& unscissoredInstanceCounts,
                        const SkTArray<ScissorBatch, true>&,
                        int PrimitiveTallies::* instanceType) const;

    // Staging area for the path being parsed.
    SkDEBUGCODE(int fParsingPath = false);
    int fCurrPathPointsIdx;
    int fCurrPathVerbsIdx;
    PrimitiveTallies fCurrPathTallies;

    SkSTArray<32, PathInfo, true> fPathsInfo;

    const SkAutoSTArray<32, SkPoint> fLocalDevPtsBuffer;
    GrCCPRGeometry fGeometry;

    PrimitiveTallies fInstanceCounts[kNumScissorModes];

    PrimitiveTallies fBaseInstances[kNumScissorModes];
    sk_sp<GrBuffer> fInstanceBuffer;

    mutable SkTArray<GrMesh> fMeshesScratchBuffer;
    mutable SkTArray<GrPipeline::DynamicState, true> fDynamicStatesScratchBuffer;

    friend class GrCCPRCoverageOp; // For ScissorBatch.
};

#endif
