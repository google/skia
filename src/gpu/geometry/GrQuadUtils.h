/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadUtils_DEFINED
#define GrQuadUtils_DEFINED

#include "include/private/SkVx.h"
#include "src/gpu/geometry/GrQuad.h"

enum class GrQuadAAFlags;
enum class GrAA : bool;
enum class GrAAType : unsigned;
struct SkRect;

namespace GrQuadUtils {

    // Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
    // Both outAAType and outEdgeFlags will be updated.
    void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                       const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

    /**
     * Crops quad to the provided device-space axis-aligned rectangle. If the intersection of this
     * quad (projected) and cropRect results in a quadrilateral, this returns true. If not, this
     * quad may be updated to be a smaller quad of the same type such that its intersection with
     * cropRect is visually the same. This function assumes that the 'quad' coordinates are finite.
     *
     * The provided edge flags are updated to reflect edges clipped by cropRect (toggling on or off
     * based on cropAA policy). If provided, the local coordinates will be updated to reflect the
     * updated device coordinates of this quad.
     *
     * 'local' may be null, in which case the new local coordinates will not be calculated. This is
     * useful when it's known a paint does not require local coordinates. However, neither
     * 'edgeFlags' nore 'quad' can be null.
     */
    bool CropToRect(const SkRect& cropRect, GrAA cropAA, GrQuadAAFlags* edgeFlags, GrQuad* quad,
                    GrQuad* local=nullptr);

    class TessellationHelper {
    public:
        // Provide nullptr if there are no local coordinates to track
        TessellationHelper(const GrQuad& deviceQuad, const GrQuad* localQuad);

        skvx::Vec<4, float> pixelCoverage();

        void inset(GrQuadAAFlags aaFlags, GrQuad* deviceInset, GrQuad* localInset);

        void outset(GrQuadAAFlags aaFlags, GrQuad* deviceOutset, GrQuad* localOutset);

    private:
        using V4f = skvx::Vec<4, float>;

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            V4f fX, fY, fW;
            // U, V, and R coordinates representing local quad.
            // Ignored depending on uvrCount (0, 1, 2).
            V4f fU, fV, fR;
            int fUVRCount;
        };

        struct QuadMetadata {
            // Normalized edge vectors of the device space quad, ordered L, B, T, R
            // (i.e. nextCCW(x) - x).
            V4f fDX, fDY;
            // 1 / edge length of the device space quad
            V4f fInvLengths;
            // Edge mask (set to all 1s if aa flags is kAll), otherwise 1.f if edge was AA,
            // 0.f if non-AA.
            V4f fMask;
        };

        struct Edges {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            V4f fA, fB, fC;
            // Whether or not the edge normals had to be flipped to preserve positive distance on
            // the inside
            bool fFlipped;
        };

        // Repeated calls to inset/outset with the same mask skip calculations
        GrQuadAAFlags       fAAFlags;

        Vertices            fOriginal;
        Vertices            fInset;
        Vertices            fOutset;
        skvx::Vec<4, float> fCoverage;

        GrQuad::Type        fDeviceType;
        GrQuad::Type        fLocalType;

        void recomputeInsetAndOutset();
        void setQuads(const Vertices& vertices, GrQuad* deviceOut, GrQuad* localOut) const;

        static QuadMetadata getMetadata(const Vertices& vertices, GrQuadAAFlags aaFlags);
        static Edges getEdgeEquations(const QuadMetadata& metadata, const Vertices& vertices);
        // Sets 'outset' to the magnitude of outset/inset to adjust each corner of a quad given the
        // edge angles and lengths. If the quad is too small, has empty edges, or too sharp of
        // angles, false is returned and the degenerate slow-path should be used.
        static bool getOptimizedOutset(const QuadMetadata& metadata,
                                       bool rectilinear,
                                       skvx::Vec<4, float>* outset);
        // Ignores the quad's fW, use outsetProjectedVertices if it's known to need 3D.
        static void outsetVertices(const skvx::Vec<4, float>& outset,
                                   const QuadMetadata& metadata,
                                   Vertices* quad);
        // Updates (x,y,w) to be at (x2d,y2d) once projected. Updates (u,v,r) to match if provided.
        // Gracefully handles 2D content if *w holds all 1s.
        static void outsetProjectedVertices(const skvx::Vec<4, float>& x2d,
                                            const skvx::Vec<4, float>& y2d,
                                            GrQuadAAFlags aaFlags,
                                            Vertices* quad);
        static skvx::Vec<4, float> getDegenerateCoverage(const skvx::Vec<4, float>& px,
                                                         const skvx::Vec<4, float>& py,
                                                         const Edges& edges);
        // Outsets or insets xs/ys in place. To be used when the interior is very small, edges are
        // near parallel, or edges are very short/zero-length. Returns coverage for each vertex.
        // Requires (dx, dy) to already be fixed for empty edges.
        static skvx::Vec<4, float> computeDegenerateQuad(GrQuadAAFlags aaFlags,
                                                         const skvx::Vec<4, float>& mask,
                                                         const Edges& edges,
                                                         bool outset,
                                                         Vertices* quad);
        // Computes the vertices for the two nested quads used to create AA edges. The original
        // single quad should be duplicated as input in 'inner' and 'outer', and the resulting quad
        // frame will be stored in-place on return. Returns per-vertex coverage for the inner
        // vertices.
        static skvx::Vec<4, float> computeNestedQuadVertices(GrQuadAAFlags aaFlags,
                                                             bool rectilinear,
                                                             Vertices* inner,
                                                             Vertices* outer);
        // Generalizes computeNestedQuadVertices to extrapolate local coords such that after
        // perspective division of the device coordinates, the original local coordinate value is at
        // the original un-outset device position.
        static skvx::Vec<4, float> computeNestedPerspQuadVertices(GrQuadAAFlags aaFlags,
                                                                  Vertices* inner,
                                                                  Vertices* outer);
    };

}; // namespace GrQuadUtils

#endif
