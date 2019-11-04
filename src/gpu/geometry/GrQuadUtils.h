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

        skvx::Vec<4, float> inset(GrQuadAAFlags aaFlags, GrQuad* deviceInset, GrQuad* localInset);

        void outset(GrQuadAAFlags aaFlags, GrQuad* deviceOutset, GrQuad* localOutset);

    private:
        struct EdgeVectors;
        struct OutsetRequest;

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            skvx::Vec<4, float> fX, fY, fW;
            // U, V, and R coordinates representing local quad.
            // Ignored depending on uvrCount (0, 1, 2).
            skvx::Vec<4, float> fU, fV, fR;
            int fUVRCount;

            // Update the device and optional local coordinates by adding outsets * (dx, dy) and
            // outsetsCW * next_cw(dx, dy) to each corner. This creates valid inset or outset
            // geometry when the outset request is not degenerate.
            void moveAlong(const EdgeVectors& edgeVectors,
                           const OutsetRequest& outsetRequest,
                           bool inset);
            // Update the device coordinates by deriving (x,y,w) that project to (x2d, y2d), with
            // optional local coordinates updated to match the new vertices. It is assumed that
            // 'mask' was respected when determing (x2d, y2d), but it is used to ensure that only
            // unmasked unprojected edge vectors are used when computing device and local coords.
            void moveTo(const skvx::Vec<4, float>& x2d,
                        const skvx::Vec<4, float>& y2d,
                        const skvx::Vec<4, int32_t>& mask);
        };

        struct EdgeVectors {
            // Projected corners (x/w and y/w); these are the 2D coordinates that determine the
            // actual edge direction vectors, dx, dy, and invLengths
            skvx::Vec<4, float> fX2D, fY2D;
            // Normalized edge vectors of the device space quad, ordered L, B, T, R
            // (i.e. next_ccw(x) - x).
            skvx::Vec<4, float> fDX, fDY;
            // Reciprocal of edge length of the device space quad, i.e. 1 / sqrt(dx*dx + dy*dy)
            skvx::Vec<4, float> fInvLengths;
        };

        struct EdgeEquations {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            skvx::Vec<4, float> fA, fB, fC;
            // Whether or not the edge normals had to be flipped to preserve positive distance on
            // the inside
            bool fFlipped;

            skvx::Vec<4, float> estimateCoverage(const skvx::Vec<4, float>& x2d,
                                                 const skvx::Vec<4, float>& y2d) const;
        };

        struct OutsetRequest {
            // Positive edge distances to move each edge of the quad. These distances represent the
            // shortest (perpendicular) distance between the original edge and the inset or outset
            // edge. If the distance is 0, then the edge will not move.
            skvx::Vec<4, float> fEdgeDistances;
            // Amount to move along each edge vector for an outset (or an inset if mul. by -1). (the
            // signed distance is determined by the actual function call, storing positive values
            // allows calculations to be shared between insets and outsets). When moving a corner,
            // it is moved along two independent vectors (its edge and its cw-rotated edge), scaled
            // by the appropriate lengths stored below.
            skvx::Vec<4, float> fOutsets;
            skvx::Vec<4, float> fOutsetsCW;
            // True if the new corners cannot be calculated by simply adding scaled edge vectors.
            // If degenerate, fOutsets[CW] should be ignored.
            bool fDegenerate;
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

        EdgeVectors getEdgeVectors() const;
        OutsetRequest getOutsetRequest(const EdgeVectors& edgeVectors) const;
        EdgeEquations getEdgeEquations(const EdgeVectors& edgeVectors) const;

        // Outsets or insets 'x2d' and 'y2d' in place. To be used when the interior is very small,
        // edges are near parallel, or edges are very short/zero-length. Returns number of effective
        // vertices in the degenerate quad.
        int computeDegenerateQuad(const skvx::Vec<4, float>& signedEdgeDistances,
                                  const EdgeEquations& edges,
                                  skvx::Vec<4, float>* x2d,
                                  skvx::Vec<4, float>* y2d);
        // Outsets or insets 'vertices' based on the outset request described by 'outsetRequest'
        // and 'inset' (true for insetting instead). If the outset is not degenerate,
        // 'edgeEquations' can be null. Returns number of effective vertices in the adjusted quad.
        int adjustVertices(const OutsetRequest& outsetRequest, bool inset,
                           const EdgeVectors& edgeVectors, const EdgeEquations* edgeEquations,
                           Vertices* vertices);
    };

}; // namespace GrQuadUtils

#endif
