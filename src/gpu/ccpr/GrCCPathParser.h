/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCoverageOp_DEFINED
#define GrCCCoverageOp_DEFINED

#include "GrMesh.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "ccpr/GrCCCoverageProcessor.h"
#include "ccpr/GrCCGeometry.h"
#include "ops/GrDrawOp.h"

class GrCCCoverageOp;
class GrOnFlushResourceProvider;
class SkMatrix;
class SkPath;

/**
 * This class produces GrCCCoverageOps that render coverage count masks and atlases. A path is
 * added to the current op in two steps:
 *
 *   1) parsePath(ScissorMode, viewMatrix, path, &devBounds, &devBounds45);
 *
 *   <client decides where to put the mask within an atlas, if wanted>
 *
 *   2) saveParsedPath(offsetX, offsetY, clipBounds);
 *
 * The client can flush the currently saved paths to a GrCCCoverageOp by calling emitOp, and
 * retrieve all emitted ops after calling finalize().
 */
class GrCCCoverageOpsBuilder {
public:
    // Indicates whether a path should enforce a scissor clip when rendering its mask. (Specified
    // as an int because these values get used directly as indices into arrays.)
    enum class ScissorMode : int { kNonScissored = 0, kScissored = 1 };
    static constexpr int kNumScissorModes = 2;

    GrCCCoverageOpsBuilder(int maxTotalPaths, int maxPathPoints, int numSkPoints, int numSkVerbs)
            : fPathsInfo(maxTotalPaths)
            , fLocalDevPtsBuffer(maxPathPoints + 1)  // Overallocate by one point to accomodate for
                                                     // overflow with Sk4f. (See parsePath.)
            , fGeometry(numSkPoints, numSkVerbs)
            , fTallies{PrimitiveTallies(), PrimitiveTallies()}
            , fScissorBatches(maxTotalPaths) {}

    ~GrCCCoverageOpsBuilder() {
        // Enforce the contract that the client always calls saveParsedPath or discardParsedPath.
        SkASSERT(!fParsingPath);
    }

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
    void saveParsedPath(ScissorMode, const SkIRect& clippedDevIBounds, int16_t atlasOffsetX,
                        int16_t atlasOffsetY);
    void discardParsedPath();

    // Flushes all currently-saved paths internally to a GrCCCoverageOp.
    //
    // NOTE: if there is a parsed path in the staging area, it will not be included. But the client
    // may still call saveParsedPath to include it in a future Op.
    void emitOp(SkISize drawBounds);

    // Builds GPU buffers and returns the list of GrCCCoverageOps as specified by calls to emitOp.
    bool finalize(GrOnFlushResourceProvider*, SkTArray<std::unique_ptr<GrCCCoverageOp>>*);

private:
    using PrimitiveTallies = GrCCGeometry::PrimitiveTallies;

    // Every kBeginPath verb has a corresponding PathInfo entry.
    struct PathInfo {
        ScissorMode fScissorMode;
        int16_t fAtlasOffsetX, fAtlasOffsetY;
        std::unique_ptr<GrCCCoverageOp> fTerminatingOp;
    };

    // Every PathInfo with a mode of kScissored has a corresponding ScissorBatch.
    struct ScissorBatch {
        PrimitiveTallies fInstanceCounts;
        SkIRect fScissor;
    };

    void parsePath(const SkPath&, const SkPoint* deviceSpacePts);
    void endContourIfNeeded(bool insideContour);

    // Staging area for the path being parsed.
    SkDEBUGCODE(int fParsingPath = false);
    int fCurrPathPointsIdx;
    int fCurrPathVerbsIdx;
    PrimitiveTallies fCurrPathTallies;

    SkSTArray<32, PathInfo, true> fPathsInfo;

    const SkAutoSTArray<32, SkPoint> fLocalDevPtsBuffer;
    GrCCGeometry fGeometry;

    PrimitiveTallies fTallies[kNumScissorModes];
    SkTArray<ScissorBatch, true> fScissorBatches;

    std::unique_ptr<GrCCCoverageOp> fTerminatingOp;

    friend class GrCCCoverageOp;  // For ScissorBatch.
};

/**
 * This Op renders coverage count masks and atlases. Create it using GrCCCoverageOpsBuilder.
 */
class GrCCCoverageOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    // GrDrawOp interface.
    const char* name() const override { return "GrCCCoverageOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*) override;

private:
    static constexpr int kNumScissorModes = GrCCCoverageOpsBuilder::kNumScissorModes;
    using PrimitiveTallies = GrCCGeometry::PrimitiveTallies;
    using ScissorBatch = GrCCCoverageOpsBuilder::ScissorBatch;

    GrCCCoverageOp(SkTArray<ScissorBatch, true>&& scissorBatches, const SkISize& drawBounds)
            : INHERITED(ClassID())
            , fScissorBatches(std::move(scissorBatches))
            , fDrawBounds(drawBounds) {
        this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    void setInstanceBuffer(sk_sp<GrBuffer> instanceBuffer,
                           const PrimitiveTallies baseInstances[kNumScissorModes],
                           const PrimitiveTallies endInstances[kNumScissorModes]);

    void drawMaskPrimitives(GrOpFlushState*, const GrPipeline&, GrCCCoverageProcessor::RenderPass,
                            int PrimitiveTallies::*instanceType) const;

    sk_sp<GrBuffer> fInstanceBuffer;
    PrimitiveTallies fBaseInstances[kNumScissorModes];
    PrimitiveTallies fInstanceCounts[kNumScissorModes];
    const SkTArray<ScissorBatch, true> fScissorBatches;
    const SkISize fDrawBounds;

    mutable SkTArray<GrMesh> fMeshesScratchBuffer;
    mutable SkTArray<GrPipeline::DynamicState, true> fDynamicStatesScratchBuffer;

    friend class GrCCCoverageOpsBuilder;

    typedef GrDrawOp INHERITED;
};

#endif
