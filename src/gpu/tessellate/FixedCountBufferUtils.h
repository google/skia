/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_FixedCountBufferUtils_DEFINED
#define tessellate_FixedCountBufferUtils_DEFINED

#include "include/core/SkPaint.h"

#include <algorithm>

namespace skgpu {

struct VertexWriter;

// This is the maximum number of segments contained in our vertex and index buffers for
// fixed-count rendering. If rendering in fixed-count mode and a curve requires more segments,
// it must be chopped.
constexpr static int kMaxFixedResolveLevel = 5;

// This is the maximum number of parametric segments (linear sections) that a curve can be split
// into. This is the same for path filling and stroking, although fixed-count stroking also uses
// additional vertices to handle radial segments, joins, and caps. Additionally the fixed-count
// path filling algorithms snap their dynamic vertex counts to powers-of-two, whereas the stroking
// algorithm does not.
constexpr static int kMaxParametricSegments = 1 << kMaxFixedResolveLevel;

// Returns an upper bound on the number of combined edges there might be from all inner fans in
// a list of paths (specified by their total combined verb count).
constexpr static int MaxCombinedFanEdgesInPaths(int totalCombinedPathVerbCnt) {
    // Path fans might have an extra edge from an implicit kClose at the end, but they also
    // always begin with kMove. So the max possible number of edges in a single path is equal to
    // the number of verbs. Therefore, the max number of combined fan edges in a path list is
    // the number of combined verbs from the paths in the list.
    return totalCombinedPathVerbCnt;
}

// How many triangles are in a curve with 2^resolveLevel line segments?
// Resolve level defines the tessellation factor for filled paths drawn using curves or wedges.
constexpr static int NumCurveTrianglesAtResolveLevel(int resolveLevel) {
    // resolveLevel=0 -> 0 line segments -> 0 triangles
    // resolveLevel=1 -> 2 line segments -> 1 triangle
    // resolveLevel=2 -> 4 line segments -> 3 triangles
    // resolveLevel=3 -> 8 line segments -> 7 triangles
    // ...
    return (1 << resolveLevel) - 1;
}

// Returns the fixed number of edges that are always emitted with the given join type. If the
// join is round, the caller needs to account for the additional radial edges on their own.
// Specifically, each join always emits:
//
//   * Two colocated edges at the beginning (a full-width edge to seam with the preceding stroke
//     and a half-width edge to begin the join).
//
//   * An extra edge in the middle for miter joins, or else a variable number of radial edges
//     for round joins (the caller is responsible for counting radial edges from round joins).
//
//   * A half-width edge at the end of the join that will be colocated with the first
//     (full-width) edge of the stroke.
//
constexpr static int NumFixedEdgesInJoin(SkPaint::Join joinType) {
    switch (joinType) {
        case SkPaint::kMiter_Join:
            return 4;
        case SkPaint::kRound_Join:
            // The caller is responsible for counting the variable number of middle, radial
            // segments on round joins.
            [[fallthrough]];
        case SkPaint::kBevel_Join:
            return 3;
    }
    SkUNREACHABLE;
}

// Returns the worst-case number of edges we will need in order to draw a join of the given type.
constexpr static int WorstCaseEdgesInJoin(SkPaint::Join joinType,
                                          float numRadialSegmentsPerRadian) {
    int numEdges = NumFixedEdgesInJoin(joinType);
    if (joinType == SkPaint::kRound_Join) {
        // For round joins we need to count the radial edges on our own. Account for a worst-case
        // join of 180 degrees (SK_ScalarPI radians).
        numEdges += std::max(SkScalarCeilToInt(numRadialSegmentsPerRadian * SK_ScalarPI) - 1, 0);
    }
    return numEdges;
}

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

    // Return the number of bytes to allocate for a buffer filled via WriteVertexBuffer, assuming
    // the shader and curve instances do require more than kMaxParametricSegments segments.
    static constexpr size_t VertexBufferSize() {
        return (kMaxParametricSegments + 1) * (2 * sizeof(float));
    }

    // As above but for the corresponding index buffer, written via WriteIndexBuffer.
    static constexpr size_t IndexBufferSize() {
        return NumCurveTrianglesAtResolveLevel(kMaxFixedResolveLevel) * 3 * sizeof(uint16_t);
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
        return (MaxCombinedFanEdgesInPaths(totalCombinedPathVerbCnt) * 5 + 3) / 4;
    }

    static constexpr size_t VertexBufferSize() {
        return ((kMaxParametricSegments + 1) + 1/*fan vertex*/) * (2 * sizeof(float));
    }

    static constexpr size_t IndexBufferSize() {
        return (NumCurveTrianglesAtResolveLevel(kMaxFixedResolveLevel) + 1/*fan triangle*/) *
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

    static constexpr size_t VertexBufferSize() {
        // Each vertex is a single float (explicit id) and each edge is composed of two vertices.
        return 2 * kMaxEdgesNoVertexIDs * sizeof(float);
    }

    // Initializes the fallback vertex buffer that should be bound when sk_VertexID is not supported
    static void WriteVertexBuffer(VertexWriter, size_t bufferSize);
};

}  // namespace skgpu

#endif // tessellate_FixedCountBufferUtils
