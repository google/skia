/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathParser_DEFINED
#define GrCCPathParser_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrTessellator.h"
#include "src/gpu/ccpr/GrCCCoverageProcessor.h"
#include "src/gpu/ccpr/GrCCFillGeometry.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrOnFlushResourceProvider;
class SkMatrix;
class SkPath;

/**
 * This class parses SkPaths into CCPR primitives in GPU buffers, then issues calls to draw their
 * coverage counts.
 */
class GrCCFiller {
public:
    GrCCFiller(int numPaths, int numSkPoints, int numSkVerbs, int numConicWeights);

    // Parses a device-space SkPath into the current batch, using the SkPath's original verbs and
    // 'deviceSpacePts'. Accepts an optional post-device-space translate for placement in an atlas.
    void parseDeviceSpaceFill(const SkPath&, const SkPoint* deviceSpacePts, GrScissorTest,
                              const SkIRect& clippedDevIBounds, const SkIVector& devToAtlasOffset);

    using BatchID = int;

    // Compiles the outstanding parsed paths into a batch, and returns an ID that can be used to
    // draw their fills in the future.
    BatchID closeCurrentBatch();

    // Builds internal GPU buffers and prepares for calls to drawFills(). Caller must close the
    // current batch before calling this method, and cannot parse new paths afer.
    bool prepareToDraw(GrOnFlushResourceProvider*);

    // Called after prepareToDraw(). Draws the given batch of path fills.
    void drawFills(
            GrOpFlushState*, GrCCCoverageProcessor*, BatchID, const SkIRect& drawBounds) const;

private:
    static constexpr int kNumScissorModes = 2;
    using PrimitiveTallies = GrCCFillGeometry::PrimitiveTallies;

    // Every kBeginPath verb has a corresponding PathInfo entry.
    class PathInfo {
    public:
        PathInfo(GrScissorTest scissorTest, const SkIVector& devToAtlasOffset)
                : fScissorTest(scissorTest), fDevToAtlasOffset(devToAtlasOffset) {}

        GrScissorTest scissorTest() const { return fScissorTest; }
        const SkIVector& devToAtlasOffset() const { return fDevToAtlasOffset; }

        // An empty tessellation fan is also valid; we use negative count to denote not tessellated.
        bool hasFanTessellation() const { return fFanTessellationCount >= 0; }
        int fanTessellationCount() const {
            SkASSERT(this->hasFanTessellation());
            return fFanTessellationCount;
        }
        const GrTessellator::WindingVertex* fanTessellation() const {
            SkASSERT(this->hasFanTessellation());
            return fFanTessellation.get();
        }
        void tessellateFan(const GrCCFillGeometry&, int verbsIdx, int ptsIdx,
                           const SkIRect& clippedDevIBounds, PrimitiveTallies* newTriangleCounts);

    private:
        GrScissorTest fScissorTest;
        SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
        int fFanTessellationCount = -1;
        std::unique_ptr<const GrTessellator::WindingVertex[]> fFanTessellation;
    };

    // Defines a batch of CCPR primitives. Start indices are deduced by looking at the previous
    // Batch in the list.
    struct Batch {
        PrimitiveTallies fEndNonScissorIndices;
        int fEndScissorSubBatchIdx;
        PrimitiveTallies fTotalPrimitiveCounts;
    };

    // Defines a sub-batch that will be drawn with the given scissor rect. Start indices are deduced
    // by looking at the previous ScissorSubBatch in the list.
    struct ScissorSubBatch {
        PrimitiveTallies fEndPrimitiveIndices;
        SkIRect fScissor;
    };

    void drawPrimitives(GrOpFlushState*, const GrCCCoverageProcessor&, const GrPipeline&, BatchID,
                        int PrimitiveTallies::*instanceType, const SkIRect& drawBounds) const;

    GrCCFillGeometry fGeometry;
    SkSTArray<32, PathInfo, true> fPathInfos;
    SkSTArray<32, Batch, true> fBatches;
    SkSTArray<32, ScissorSubBatch, true> fScissorSubBatches;
    PrimitiveTallies fTotalPrimitiveCounts[kNumScissorModes];
    int fMaxMeshesPerDraw = 0;

    sk_sp<GrGpuBuffer> fInstanceBuffer;
    PrimitiveTallies fBaseInstances[kNumScissorModes];
    mutable SkSTArray<32, GrMesh> fMeshesScratchBuffer;
    mutable SkSTArray<32, SkIRect> fScissorRectScratchBuffer;
};

#endif
