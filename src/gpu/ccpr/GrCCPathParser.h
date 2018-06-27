/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathParser_DEFINED
#define GrCCPathParser_DEFINED

#include "GrMesh.h"
#include "GrNonAtomicRef.h"
#include "GrTessellator.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "ccpr/GrCCCoverageProcessor.h"
#include "ccpr/GrCCGeometry.h"
#include "ops/GrDrawOp.h"

class GrOnFlushResourceProvider;
class SkMatrix;
class SkPath;

/**
 * This class parses SkPaths into CCPR primitives in GPU buffers, then issues calls to draw their
 * coverage counts.
 */
class GrCCPathParser : public GrNonAtomicRef<GrCCPathParser> {
public:
    // Indicates whether a path should enforce a scissor clip when rendering its mask. (Specified
    // as an int because these values get used directly as indices into arrays.)
    enum class ScissorMode : int { kNonScissored = 0, kScissored = 1 };
    static constexpr int kNumScissorModes = 2;

    GrCCPathParser(int maxTotalPaths, int maxPathPoints, int numSkPoints, int numSkVerbs);

    ~GrCCPathParser() {
        // Enforce the contract that the client always calls saveParsedPath or discardParsedPath.
        SkASSERT(!fParsingPath);
    }

    using CoverageCountBatchID = int;

    // Parses an SkPath into a temporary staging area. The path will not be included in the current
    // batch until there is a matching call to saveParsedPath. The user must complement this with a
    // following call to either saveParsedPath or discardParsedPath.
    //
    // Returns two tight bounding boxes: device space and "45 degree" (| 1 -1 | * devCoords) space.
    //                                                                 | 1  1 |
    void parsePath(const SkMatrix&, const SkPath&, SkRect* devBounds, SkRect* devBounds45);

    // Parses a device-space SkPath into a temporary staging area. The path will not be included in
    // the current batch until there is a matching call to saveParsedPath. The user must complement
    // this with a following call to either saveParsedPath or discardParsedPath.
    void parseDeviceSpacePath(const SkPath&);

    // Commits the currently-parsed path from staging to the current batch, and specifies whether
    // the mask should be rendered with a scissor in effect. Accepts an optional post-device-space
    // translate for placement in an atlas.
    void saveParsedPath(ScissorMode, const SkIRect& clippedDevIBounds, int16_t atlasOffsetX,
                        int16_t atlasOffsetY);
    void discardParsedPath();

    // Compiles the outstanding saved paths into a batch, and returns an ID that can be used to draw
    // their coverage counts in the future.
    CoverageCountBatchID closeCurrentBatch();

    // Builds internal GPU buffers and prepares for calls to drawCoverageCount. Caller must close
    // the current batch before calling this method, and cannot parse new paths afer.
    bool finalize(GrOnFlushResourceProvider*);

    // Called after finalize. Draws the given batch of parsed paths.
    void drawCoverageCount(GrOpFlushState*, CoverageCountBatchID, const SkIRect& drawBounds) const;

private:
    using PrimitiveTallies = GrCCGeometry::PrimitiveTallies;

    // Every kBeginPath verb has a corresponding PathInfo entry.
    struct PathInfo {
        PathInfo(ScissorMode scissorMode, int16_t offsetX, int16_t offsetY)
                : fScissorMode(scissorMode), fAtlasOffsetX(offsetX), fAtlasOffsetY(offsetY) {}

        ScissorMode fScissorMode;
        int16_t fAtlasOffsetX, fAtlasOffsetY;
        std::unique_ptr<GrTessellator::WindingVertex[]> fFanTessellation;
        int fFanTessellationCount = 0;
    };

    // Defines a batch of CCPR primitives. Start indices are deduced by looking at the previous
    // CoverageCountBatch in the list.
    struct CoverageCountBatch {
        PrimitiveTallies fEndNonScissorIndices;
        int fEndScissorSubBatchIdx;
        PrimitiveTallies fTotalPrimitiveCounts;
    };

    // Defines a sub-batch from CoverageCountBatch that will be drawn with the given scissor rect.
    // Start indices are deduced by looking at the previous ScissorSubBatch in the list.
    struct ScissorSubBatch {
        PrimitiveTallies fEndPrimitiveIndices;
        SkIRect fScissor;
    };

    void parsePath(const SkPath&, const SkPoint* deviceSpacePts);
    void endContourIfNeeded(bool insideContour);

    void drawRenderPass(GrOpFlushState*, const GrPipeline&, CoverageCountBatchID,
                        GrCCCoverageProcessor::RenderPass, GrCCCoverageProcessor::WindMethod,
                        int PrimitiveTallies::*instanceType, const SkIRect& drawBounds) const;

    // Staging area for the path being parsed.
    SkDEBUGCODE(int fParsingPath = false);
    const SkAutoSTArray<32, SkPoint> fLocalDevPtsBuffer;
    int fCurrPathPointsIdx;
    int fCurrPathVerbsIdx;
    PrimitiveTallies fCurrPathPrimitiveCounts;

    GrCCGeometry fGeometry;
    SkSTArray<32, PathInfo, true> fPathsInfo;
    SkSTArray<32, CoverageCountBatch, true> fCoverageCountBatches;
    SkSTArray<32, ScissorSubBatch, true> fScissorSubBatches;
    PrimitiveTallies fTotalPrimitiveCounts[kNumScissorModes];
    int fMaxMeshesPerDraw = 0;

    sk_sp<GrBuffer> fInstanceBuffer;
    PrimitiveTallies fBaseInstances[kNumScissorModes];
    mutable SkSTArray<32, GrMesh> fMeshesScratchBuffer;
    mutable SkSTArray<32, GrPipeline::DynamicState> fDynamicStatesScratchBuffer;
};

#endif
