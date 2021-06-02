/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellator_DEFINED
#define GrPathTessellator_DEFINED

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class SkPath;
class GrPathTessellationShader;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class GrPathTessellator {
public:
    using BreadcrumbTriangleList = GrInnerFanTriangulator::BreadcrumbTriangleList;

    // For subclasses that use this enum, if DrawInnerFan is kNo, the class only emits the path's
    // outer curves. In that case the caller is responsible to handle the path's inner fan.
    enum class DrawInnerFan : bool {
        kNo = false,
        kYes
    };

    // Creates the tessellator best suited to draw the given path.
    static GrPathTessellator* Make(SkArenaAlloc*, const SkPath&, const SkMatrix&,
                                   const SkPMColor4f&, DrawInnerFan, const GrCaps&);

    const GrPathTessellationShader* shader() const { return fShader; }

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate. If the
    // given BreadcrumbTriangleList is non-null, then this class will also include the breadcrumb
    // triangles in its draw.
    virtual void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkPath&,
                         const BreadcrumbTriangleList* = nullptr) = 0;

    // Issues draw calls for the tessellated geometry. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    // Draws a 4-point instance for each curve. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time. This method is not supported by every subclass.
    virtual void drawHullInstances(GrOpFlushState*) const { SK_ABORT("Not supported."); }

    virtual ~GrPathTessellator() {}

protected:
    GrPathTessellator(GrPathTessellationShader* shader) : fShader(shader) {}

    // Returns an upper bound on the number of segments (lineTo, quadTo, conicTo, cubicTo) in a
    // path, also accounting for any implicit lineTos from closing contours.
    static int MaxSegmentsInPath(const SkPath& path) {
        // There might be an implicit kClose at the end, but the path always begins with kMove. So
        // the max number of segments in the path is equal to the number of verbs.
        SkASSERT(path.countVerbs() == 0 || SkPathPriv::VerbData(path)[0] == SkPath::kMove_Verb);
        return path.countVerbs();
    }

    // Returns an upper bound on the number of triangles it would require to fan a path's inner
    // polygon, in the case where no additional vertices are introduced.
    static int MaxTrianglesInInnerFan(const SkPath& path) {
        int maxEdgesInFan = MaxSegmentsInPath(path);
        return std::max(maxEdgesInFan - 2, 0);  // An n-sided polygon is fanned by n-2 triangles.
    }

    // Writes out the non-degenerate triangles from 'breadcrumbTriangleList' as 4-point conic
    // patches with w=Infinity.
    static int WriteBreadcrumbTriangles(GrVertexWriter* writer,
                                        const BreadcrumbTriangleList* breadcrumbTriangleList) {
        int numWritten = 0;
        SkDEBUGCODE(int count = 0;)
        for (const auto* tri = breadcrumbTriangleList->head(); tri; tri = tri->fNext) {
            SkDEBUGCODE(++count;)
            auto p0 = grvx::float2::Load(tri->fPts);
            auto p1 = grvx::float2::Load(tri->fPts + 1);
            auto p2 = grvx::float2::Load(tri->fPts + 2);
            if (skvx::any((p0 == p1) & (p1 == p2))) {
                // Cull completely horizontal or vertical triangles. GrTriangulator can't always get
                // these breadcrumb edges right when they run parallel to the sweep direction
                // because their winding is undefined by its current definition.
                // FIXME(skia:12060): This seemed safe, but if there is a view matrix it will
                // introduce T-junctions.
                continue;
            }
            writer->writeArray(tri->fPts, 3);
            // Mark this instance as a triangle by setting it to a conic with w=Inf.
            writer->fill(GrVertexWriter::kIEEE_32_infinity, 2);
            ++numWritten;
        }
        SkASSERT(count == breadcrumbTriangleList->count());
        return numWritten;
    }

    GrPathTessellationShader* fShader;
};

#endif
