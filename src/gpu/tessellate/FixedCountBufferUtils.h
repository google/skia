/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_FixedCountBufferUtils_DEFINED
#define skgpu_tessellate_FixedCountBufferUtils_DEFINED

#include "src/gpu/tessellate/LinearTolerances.h"
#include "src/gpu/tessellate/Tessellation.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace skgpu { struct VertexWriter; }

namespace skgpu::tess {

/**
 * Fixed-count tessellation operates in three modes, two for filling paths, and one for stroking.
 * These modes may have additional sub-variations, but in terms of vertex buffer management, these
 * three categories are sufficient:
 *
 * - FixedCountCurves: for filling paths where just the curves are tessellated. Additional measures
 *     to fill space between the inner control points of the paths are needed.
 * - FixedCountWedges: for filling paths by tessellating the curves and adding an additional inline
 *     triangle with a shared vertex that all verbs connect to. Works with PatchAttribs::kFanPoint.
 * - FixedCountStrokes: for stroking a path. Likely paired with PatchAttribs::kJoinControlPoint and
 *     PatchAttribs::kStrokeParams.
 *
 * The three types defined below for these three modes provide utility functions for heuristics to
 * choose pre-allocation size when accumulating instance attributes with a PatchWriter, and
 * functions for creating static/GPU-private vertex and index buffers that are used as the template
 * for instanced rendering.
 */
class FixedCountCurves {
    FixedCountCurves() = delete;
public:
    // A heuristic function for reserving instance attribute space before using a PatchWriter.
    static constexpr int PreallocCount(int totalCombinedPathVerbCnt) {
        // Over-allocate enough curves for 1 in 4 to chop. Every chop introduces 2 new patches:
        // another curve patch and a triangle patch that glues the two chops together,
        // i.e. + 2 * ((count + 3) / 4) == (count + 3) / 2
        return totalCombinedPathVerbCnt + (totalCombinedPathVerbCnt + 3) / 2;
    }

    // Convert the accumulated worst-case tolerances into an index count passed into an instanced,
    // indexed draw function that uses FixedCountCurves static vertex and index buffers.
    static int VertexCount(const LinearTolerances& tolerances) {
        // We should already chopped curves to make sure none needed a higher resolveLevel than
        // kMaxResolveLevel.
        int resolveLevel = std::min(tolerances.requiredResolveLevel(), kMaxResolveLevel);
        return NumCurveTrianglesAtResolveLevel(resolveLevel) * 3;
    }

    static constexpr size_t VertexBufferVertexCount() {
        return kMaxParametricSegments + 1;
    }

    static constexpr size_t VertexBufferStride() {
        return 2 * sizeof(float);
    }

    // Return the number of bytes to allocate for a buffer filled via WriteVertexBuffer, assuming
    // the shader and curve instances do require more than kMaxParametricSegments segments.
    static constexpr size_t VertexBufferSize() {
        return VertexBufferVertexCount() * VertexBufferStride();
    }

    // As above but for the corresponding index buffer, written via WriteIndexBuffer.
    static constexpr size_t IndexBufferSize() {
        return NumCurveTrianglesAtResolveLevel(kMaxResolveLevel) * 3 * sizeof(uint16_t);
    }

    static void WriteVertexBuffer(VertexWriter, size_t bufferSize);

    static void WriteIndexBuffer(VertexWriter, size_t bufferSize);
};

class FixedCountWedges {
    FixedCountWedges() = delete;
public:
    // These functions provide equivalent functionality to the matching ones in FixedCountCurves,
    // but are intended for use with a shader and PatchWriter that has enabled the kFanPoint attrib.

    static constexpr int PreallocCount(int totalCombinedPathVerbCnt)  {
        // Over-allocate enough wedges for 1 in 4 to chop, i.e., ceil(maxWedges * 5/4)
        return (totalCombinedPathVerbCnt * 5 + 3) / 4;
    }

    static int VertexCount(const LinearTolerances& tolerances) {
        // Emit 3 vertices per curve triangle, plus 3 more for the wedge fan triangle.
        int resolveLevel = std::min(tolerances.requiredResolveLevel(), kMaxResolveLevel);
        return (NumCurveTrianglesAtResolveLevel(resolveLevel) + 1) * 3;
    }

    static constexpr size_t VertexBufferVertexCount() {
        return (kMaxParametricSegments + 1) + 1/*fan vertex*/;
    }

    static constexpr size_t VertexBufferStride() {
        return 2 * sizeof(float);
    }

    static constexpr size_t VertexBufferSize() {
        return VertexBufferVertexCount() * VertexBufferStride();
    }

    static constexpr size_t IndexBufferSize() {
        return (NumCurveTrianglesAtResolveLevel(kMaxResolveLevel) + 1/*fan triangle*/) *
               3 * sizeof(uint16_t);
    }

    static void WriteVertexBuffer(VertexWriter, size_t bufferSize);

    static void WriteIndexBuffer(VertexWriter, size_t bufferSize);
};

class FixedCountStrokes {
    FixedCountStrokes() = delete;
public:
    // These functions provide equivalent functionality to the matching ones in FixedCountCurves,
    // but are intended for a shader that that strokes a path instead of filling, where vertices
    // are associated with joins, caps, radial segments, or parametric segments.
    //
    // NOTE: The fixed-count stroke buffer is only needed when vertex IDs are not available as an
    // SkSL built-in. And unlike the curve and wedge variants, stroke drawing never relies on an
    // index buffer so those functions are not provided.

    // Don't draw more vertices than can be indexed by a signed short. We just have to draw the line
    // somewhere and this seems reasonable enough. (There are two vertices per edge, so 2^14 edges
    // make 2^15 vertices.)
    static constexpr int kMaxEdges = (1 << 14) - 1;
    static constexpr int kMaxEdgesNoVertexIDs = 1024;

    static constexpr int PreallocCount(int totalCombinedPathVerbCnt) {
        // Over-allocate enough patches for each stroke to chop once, and for 8 extra caps. Since
        // we have to chop at inflections, points of 180 degree rotation, and anywhere a stroke
        // requires too many parametric segments, many strokes will end up getting choppped.
        return (totalCombinedPathVerbCnt * 2) + 8/* caps */;
    }

    // Does not account for falling back to kMaxEdgesNoVertexIDs
    static int VertexCount(const LinearTolerances& tolerances) {
        return std::min(tolerances.requiredStrokeEdges(), kMaxEdges) * 2;
    }

    static constexpr size_t VertexBufferSize() {
        // Each vertex is a single float (explicit id) and each edge is composed of two vertices.
        return 2 * kMaxEdgesNoVertexIDs * sizeof(float);
    }

    // Initializes the fallback vertex buffer that should be bound when sk_VertexID is not supported
    static void WriteVertexBuffer(VertexWriter, size_t bufferSize);
};

}  // namespace skgpu::tess

#endif // skgpu_tessellate_FixedCountBufferUtils
