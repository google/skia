/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/ops/GrDrawOp.h"
#include <type_traits>

class SkArenaAlloc;
class GrAtlasManager;
class GrCaps;
class GrMeshDrawTarget;
class GrOpFlushState;
struct GrSimpleMesh;
class GrSmallPathAtlasMgr;
class GrStrikeCache;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    static bool CanUpgradeAAOnMerge(GrAAType aa1, GrAAType aa2) {
        return (aa1 == GrAAType::kNone && aa2 == GrAAType::kCoverage) ||
               (aa1 == GrAAType::kCoverage && aa2 == GrAAType::kNone);
    }

protected:
    GrMeshDrawOp(uint32_t classID);

    void createProgramInfo(const GrCaps* caps,
                           SkArenaAlloc* arena,
                           const GrSurfaceProxyView& writeView,
                           bool usesMSAASurface,
                           GrAppliedClip&& appliedClip,
                           const GrDstProxyView& dstProxyView,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) {
        this->onCreateProgramInfo(caps, arena, writeView, usesMSAASurface, std::move(appliedClip),
                                  dstProxyView, renderPassXferBarriers, colorLoadOp);
    }

    void createProgramInfo(GrMeshDrawTarget*);

    /** Helper for rendering repeating meshes using a patterned index buffer. This class creates the
        space for the vertices and flushes the draws to the GrMeshDrawTarget. */
    class PatternHelper {
    public:
        PatternHelper(GrMeshDrawTarget*, GrPrimitiveType, size_t vertexStride,
                      sk_sp<const GrBuffer> indexBuffer, int verticesPerRepetition,
                      int indicesPerRepetition, int repeatCount, int maxRepetitions);

        /** Called to issue draws to the GrMeshDrawTarget.*/
        void recordDraw(GrMeshDrawTarget*, const GrGeometryProcessor*) const;
        void recordDraw(GrMeshDrawTarget*, const GrGeometryProcessor*,
                        const GrSurfaceProxy* const primProcProxies[]) const;

        void* vertices() const { return fVertices; }
        GrSimpleMesh* mesh() { return fMesh; }

    protected:
        PatternHelper() = default;
        void init(GrMeshDrawTarget*, GrPrimitiveType, size_t vertexStride,
                  sk_sp<const GrBuffer> indexBuffer, int verticesPerRepetition,
                  int indicesPerRepetition, int repeatCount, int maxRepetitions);

    private:
        void* fVertices = nullptr;
        GrSimpleMesh* fMesh = nullptr;
        GrPrimitiveType fPrimitiveType;
    };

    /** A specialization of InstanceHelper for quad rendering.
     *  It only draws non-antialiased indexed quads.
     */
    class QuadHelper : private PatternHelper {
    public:
        QuadHelper() = delete;
        QuadHelper(GrMeshDrawTarget*, size_t vertexStride, int quadsToDraw);

        using PatternHelper::mesh;
        using PatternHelper::recordDraw;
        using PatternHelper::vertices;

    private:
        using INHERITED = PatternHelper;
    };

    static bool CombinedQuadCountWillOverflow(GrAAType aaType,
                                              bool willBeUpgradedToAA,
                                              int combinedQuadCount);

    virtual void onPrePrepareDraws(GrRecordingContext*,
                                   const GrSurfaceProxyView& writeView,
                                   GrAppliedClip*,
                                   const GrDstProxyView&,
                                   GrXferBarrierFlags renderPassXferBarriers,
                                   GrLoadOp colorLoadOp);

private:
    virtual GrProgramInfo* programInfo() = 0;
    // This method is responsible for creating all the programInfos required
    // by this op.
    virtual void onCreateProgramInfo(const GrCaps*,
                                     SkArenaAlloc*,
                                     const GrSurfaceProxyView& writeView,
                                     bool usesMSAASurface,
                                     GrAppliedClip&&,
                                     const GrDstProxyView&,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) = 0;

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrDstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {
        this->onPrePrepareDraws(context, writeView, clip, dstProxyView, renderPassXferBarriers,
                                colorLoadOp);
    }
    void onPrepare(GrOpFlushState* state) final;

    virtual void onPrepareDraws(GrMeshDrawTarget*) = 0;
    using INHERITED = GrDrawOp;
};

#endif
