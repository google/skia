/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCStroker_DEFINED
#define GrCCStroker_DEFINED

#include "include/private/SkNx.h"
#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/ccpr/GrCCStrokeGeometry.h"

class GrGpuBuffer;
class GrCCCoverageProcessor;
class GrOnFlushResourceProvider;
class GrOpFlushState;
class GrPipeline;
class GrPrimitiveProcessor;
class SkMatrix;
class SkPath;
class SkStrokeRec;

/**
 * This class parses stroked SkPaths into a GPU instance buffer, then issues calls to draw their
 * coverage counts.
 */
class GrCCStroker {
public:
    GrCCStroker(int numPaths, int numSkPoints, int numSkVerbs)
            : fGeometry(numSkPoints, numSkVerbs), fPathInfos(numPaths) {}

    // Parses a device-space SkPath into the current batch, using the SkPath's original verbs with
    // 'deviceSpacePts', and the SkStrokeRec's original settings with 'strokeDevWidth'. Accepts an
    // optional post-device-space translate for placement in an atlas.
    //
    // Strokes intended as hairlines must have a strokeDevWidth of 1. Non-hairline strokes can only
    // be drawn with rigid body transforms; affine transformation of the stroke lines themselves is
    // not yet supported.
    void parseDeviceSpaceStroke(const SkPath&, const SkPoint* deviceSpacePts, const SkStrokeRec&,
                                float strokeDevWidth, GrScissorTest,
                                const SkIRect& clippedDevIBounds,
                                const SkIVector& devToAtlasOffset);

    using BatchID = int;

    // Compiles the outstanding parsed paths into a batch, and returns an ID that can be used to
    // draw their strokes in the future.
    BatchID closeCurrentBatch();

    // Builds an internal GPU buffer and prepares for calls to drawStrokes(). Caller must close the
    // current batch before calling this method, and cannot parse new paths afer.
    bool prepareToDraw(GrOnFlushResourceProvider*);

    // Called after prepareToDraw(). Draws the given batch of path strokes.
    void drawStrokes(
            GrOpFlushState*, GrCCCoverageProcessor*, BatchID, const SkIRect& drawBounds) const;

private:
    static constexpr int kNumScissorModes = 2;
    static constexpr BatchID kEmptyBatchID = -1;
    using Verb = GrCCStrokeGeometry::Verb;
    using InstanceTallies = GrCCStrokeGeometry::InstanceTallies;

    // Every kBeginPath verb has a corresponding PathInfo entry.
    struct PathInfo {
        SkIVector fDevToAtlasOffset;
        float fStrokeRadius;
        GrScissorTest fScissorTest;
    };

    // Defines a sub-batch of stroke instances that have a scissor test and the same scissor rect.
    // Start indices are deduced by looking at the previous ScissorSubBatch.
    struct ScissorSubBatch {
        ScissorSubBatch(GrTAllocator<InstanceTallies>* alloc, const InstanceTallies& startIndices,
                        const SkIRect& scissor)
                : fEndInstances(&alloc->emplace_back(startIndices)), fScissor(scissor) {}
        InstanceTallies* fEndInstances;
        SkIRect fScissor;
    };

    // Defines a batch of stroke instances that can be drawn with drawStrokes(). Start indices are
    // deduced by looking at the previous Batch in the list.
    struct Batch {
        Batch(GrTAllocator<InstanceTallies>* alloc, const InstanceTallies& startNonScissorIndices,
              int startScissorSubBatch)
                : fNonScissorEndInstances(&alloc->emplace_back(startNonScissorIndices))
                , fEndScissorSubBatch(startScissorSubBatch) {}
        InstanceTallies* fNonScissorEndInstances;
        int fEndScissorSubBatch;
    };

    class InstanceBufferBuilder;

    void appendStrokeMeshesToBuffers(int numSegmentsLog2, const Batch&,
                                     const InstanceTallies* startIndices[2],
                                     int startScissorSubBatch, const SkIRect& drawBounds) const;
    void flushBufferedMeshesAsStrokes(const GrPrimitiveProcessor&, GrOpFlushState*, const
                                      GrPipeline&, const SkIRect& drawBounds) const;

    template<int GrCCStrokeGeometry::InstanceTallies::* InstanceType>
    void drawConnectingGeometry(GrOpFlushState*, const GrPipeline&,
                                const GrCCCoverageProcessor&, const Batch&,
                                const InstanceTallies* startIndices[2], int startScissorSubBatch,
                                const SkIRect& drawBounds) const;

    GrCCStrokeGeometry fGeometry;
    SkSTArray<32, PathInfo> fPathInfos;
    SkSTArray<32, Batch> fBatches;
    SkSTArray<32, ScissorSubBatch> fScissorSubBatches;
    int fMaxNumScissorSubBatches = 0;
    bool fHasOpenBatch = false;

    const InstanceTallies fZeroTallies = InstanceTallies();
    GrSTAllocator<128, InstanceTallies> fTalliesAllocator;
    const InstanceTallies* fInstanceCounts[kNumScissorModes] = {&fZeroTallies, &fZeroTallies};

    sk_sp<GrGpuBuffer> fInstanceBuffer;
    // The indices stored in batches are relative to these base instances.
    InstanceTallies fBaseInstances[kNumScissorModes];

    mutable SkSTArray<32, GrMesh> fMeshesBuffer;
    mutable SkSTArray<32, SkIRect> fScissorsBuffer;
};

#endif
