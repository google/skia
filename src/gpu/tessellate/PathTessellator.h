/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PathTessellator_DEFINED
#define tessellate_PathTessellator_DEFINED

#include "src/gpu/tessellate/Tessellation.h"

class SkPath;

#if SK_GPU_V1

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/PatchWriter.h"

class GrGpuBuffer;
class GrMeshDrawTarget;
class GrOpFlushState;

#endif

namespace skgpu {

class PatchWriter;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class PathTessellator {
public:
    // This is the maximum number of segments contained in our vertex and index buffers for
    // fixed-count rendering. If rendering in fixed-count mode and a curve requires more segments,
    // it must be chopped.
    constexpr static int kMaxFixedResolveLevel = 5;

    struct PathDrawList {
        PathDrawList(const SkMatrix& pathMatrix,
                     const SkPath& path,
                     const SkPMColor4f& color,
                     PathDrawList* next = nullptr)
                : fPathMatrix(pathMatrix), fPath(path), fColor(color), fNext(next) {}

        SkMatrix fPathMatrix;
        SkPath fPath;
        SkPMColor4f fColor;
        PathDrawList* fNext;

        struct Iter {
            void operator++() { fHead = fHead->fNext; }
            bool operator!=(const Iter& b) const { return fHead != b.fHead; }
            std::tuple<const SkMatrix&, const SkPath&, const SkPMColor4f&> operator*() const {
                return {fHead->fPathMatrix, fHead->fPath, fHead->fColor};
            }
            const PathDrawList* fHead;
        };
        Iter begin() const { return {this}; }
        Iter end() const { return {nullptr}; }
    };

    virtual ~PathTessellator() {}

    PatchAttribs patchAttribs() const { return fAttribs; }

    // Gives an approximate initial buffer size for this class to write patches into. Ideally the
    // whole path will fit into this initial buffer, but if it requires a lot of chopping, the
    // PatchWriter will allocate more buffer(s).
    virtual int patchPreallocCount(int totalCombinedPathVerbCnt) const = 0;

    // Writes out patches to the given PatchWriter, chopping as necessary so the curves all fit in
    // maxTessellationSegments or fewer.
    //
    // Each path's fPathMatrix in the list is applied on the CPU while the geometry is being written
    // out. This is a tool for batching, and is applied in addition to the shader's on-GPU matrix.
    virtual void writePatches(PatchWriter&,
                              int maxTessellationSegments,
                              const SkMatrix& shaderMatrix,
                              const PathDrawList&) = 0;

#if SK_GPU_V1
    // Initializes the internal vertex and index buffers required for drawFixedCount().
    virtual void prepareFixedCountBuffers(GrMeshDrawTarget*) = 0;

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    void prepare(GrMeshDrawTarget* target,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList& pathDrawList,
                 int totalCombinedPathVerbCnt,
                 bool willUseTessellationShaders) {
        if (int patchPreallocCount = this->patchPreallocCount(totalCombinedPathVerbCnt)) {
            PatchWriter patchWriter(target, this, patchPreallocCount);
            this->writePatches(patchWriter, maxTessellationSegments, shaderMatrix, pathDrawList);
        }
        if (!willUseTessellationShaders) {
            this->prepareFixedCountBuffers(target);
        }
    }

    // Issues hardware tessellation draw calls over the patches. The caller is responsible for
    // binding its desired pipeline ahead of time.
    virtual void drawTessellated(GrOpFlushState*) const = 0;

    // Issues fixed-count instanced draw calls over the patches. The caller is responsible for
    // binding its desired pipeline ahead of time.
    virtual void drawFixedCount(GrOpFlushState*) const = 0;

    void draw(GrOpFlushState* flushState, bool willUseTessellationShaders) {
        if (willUseTessellationShaders) {
            this->drawTessellated(flushState);
        } else {
            this->drawFixedCount(flushState);
        }
    }
#endif

    // Returns an upper bound on the number of combined edges there might be from all inner fans in
    // a PathDrawList.
    static int MaxCombinedFanEdgesInPathDrawList(int totalCombinedPathVerbCnt) {
        // Path fans might have an extra edge from an implicit kClose at the end, but they also
        // always begin with kMove. So the max possible number of edges in a single path is equal to
        // the number of verbs. Therefore, the max number of combined fan edges in a PathDrawList is
        // the number of combined path verbs in that PathDrawList.
        return totalCombinedPathVerbCnt;
    }

protected:
    // How many triangles are in a curve with 2^resolveLevel line segments?
    constexpr static int NumCurveTrianglesAtResolveLevel(int resolveLevel) {
        // resolveLevel=0 -> 0 line segments -> 0 triangles
        // resolveLevel=1 -> 2 line segments -> 1 triangle
        // resolveLevel=2 -> 4 line segments -> 3 triangles
        // resolveLevel=3 -> 8 line segments -> 7 triangles
        // ...
        return (1 << resolveLevel) - 1;
    }

    PathTessellator(bool infinitySupport, PatchAttribs attribs) : fAttribs(attribs) {
        if (!infinitySupport) {
            fAttribs |= PatchAttribs::kExplicitCurveType;
        }
    }

    PatchAttribs fAttribs;

    // Calculated during prepare(). If using fixed count, this is the resolveLevel to use on our
    // instanced draws. 2^resolveLevel == numSegments.
    int fFixedResolveLevel = 0;

#if SK_GPU_V1
    friend class PatchWriter;  // To access fVertexChunkArray.

    GrVertexChunkArray fVertexChunkArray;

    // If using fixed-count rendering, these are the vertex and index buffers.
    sk_sp<const GrGpuBuffer> fFixedVertexBuffer;
    sk_sp<const GrGpuBuffer> fFixedIndexBuffer;
#endif
};

}  // namespace skgpu

#endif  // tessellate_PathTessellator_DEFINED
