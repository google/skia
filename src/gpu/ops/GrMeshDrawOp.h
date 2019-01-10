/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "GrAppliedClip.h"
#include "GrDrawOp.h"
#include "GrGeometryProcessor.h"
#include "GrMesh.h"
#include <type_traits>

class GrAtlasManager;
class GrCaps;
class GrGlyphCache;
class GrOpFlushState;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    class Target;
protected:
    GrMeshDrawOp(uint32_t classID);

    /** Helper for rendering repeating meshes using a patterned index buffer. This class creates the
        space for the vertices and flushes the draws to the render target. */
    class PatternHelper {
    public:
        PatternHelper(GrOpFlushState*, GrPrimitiveType, size_t vertexStride, const GrBuffer*,
                      int verticesPerRepetition, int indicesPerRepetition, int repeatCount);

        /** Called to issue draws to the render target.*/
        void recordDraw(GrOpFlushState*, sk_sp<const GrGeometryProcessor>, const GrPipeline*,
                        const GrPipeline::FixedDynamicState*) const;

        void* vertices() const { return fVertices; }

    protected:
        PatternHelper() = default;
        void init(GrOpFlushState*, GrPrimitiveType, size_t vertexStride, const GrBuffer*,
                  int verticesPerRepetition, int indicesPerRepetition, int repeatCount);

    private:
        void* fVertices = nullptr;
        GrMesh* fMesh = nullptr;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private PatternHelper {
    public:
        QuadHelper() = delete;
        QuadHelper(GrOpFlushState*, size_t vertexStride, int quadsToDraw);

        using PatternHelper::recordDraw;
        using PatternHelper::vertices;

    private:
        typedef PatternHelper INHERITED;
    };

private:
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) final;

    typedef GrDrawOp INHERITED;
};

#endif
