/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadUtils_DEFINED
#define GrQuadUtils_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"

#include <cstdint>

enum class GrAA : bool;
enum class GrAAType : unsigned int;
enum class GrQuadAAFlags;
struct SkRect;

namespace GrQuadUtils {

    // Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
    // Both outAAType and outEdgeFlags will be updated.
    void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                       const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

    /**
     * Clip the device vertices of 'quad' to be in front of the W = 0 plane (w/in epsilon). The
     * local coordinates will be updated to match the new clipped vertices. This returns the number
     * of clipped quads that need to be drawn: 0 if 'quad' was entirely behind the plane, 1 if
     * 'quad' did not need to be clipped or if 2 or 3 vertices were clipped, or 2 if 'quad' had one
     * vertex clipped (producing a pentagonal shape spanned by 'quad' and 'extraVertices').
     */
    int ClipToW0(DrawQuad* quad, DrawQuad* extraVertices);

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
     * If 'computeLocal' is false, the local coordinates in 'quad' will not be modified.
     */
    bool CropToRect(const SkRect& cropRect, GrAA cropAA, DrawQuad* quad, bool computeLocal=true);

    inline void Outset(const skvx::float4& edgeDistances, GrQuad* quad);

    bool WillUseHairline(const GrQuad& quad, GrAAType aaType, GrQuadAAFlags edgeFlags);

    class TessellationHelper {
    public:
        // Set the original device and (optional) local coordinates that are inset or outset
        // by the requested edge distances. Use nullptr if there are no local coordinates to update.
        // This assumes all device coordinates have been clipped to W > 0.
        void reset(const GrQuad& deviceQuad, const GrQuad* localQuad);

        // Calculates a new quadrilateral with edges parallel to the original except that they
        // have been moved inwards by edgeDistances (which should be positive). Distances are
        // ordered L, B, T, R to match CCW tristrip ordering of GrQuad vertices. Edges that are
        // not moved (i.e. distance == 0) will not be used in calculations and the corners will
        // remain on that edge.
        //
        // The per-vertex coverage will be returned. When the inset geometry does not collapse to
        // a point or line, this will be 1.0 for every vertex. When it does collapse, the per-vertex
        // coverages represent estimated pixel coverage to simulate drawing the subpixel-sized
        // original quad.
        //
        // Note: the edge distances are in device pixel units, so after rendering the new quad
        // edge's shortest distance to the original quad's edge would be equal to provided edge dist
        skvx::float4 inset(const skvx::float4& edgeDistances,
                           GrQuad* deviceInset, GrQuad* localInset);

        // Calculates a new quadrilateral that outsets the original edges by the given distances.
        // Other than moving edges outwards, this function is equivalent to inset(). If the exact
        // same edge distances are provided, certain internal computations can be reused across
        // consecutive calls to inset() and outset() (in any order).
        void outset(const skvx::float4& edgeDistances,
                    GrQuad* deviceOutset, GrQuad* localOutset);

        // Compute the edge equations of the original device space quad passed to 'reset()'. The
        // coefficients are stored per-edge in 'a', 'b', and 'c', such that ax + by + c = 0, and
        // a positive distance indicates the interior of the quad. Edges are ordered L, B, T, R,
        // matching edge distances passed to inset() and outset().
        void getEdgeEquations(skvx::float4* a,
                              skvx::float4* b,
                              skvx::float4* c);

        // Compute the edge lengths of the original device space quad passed to 'reset()'. The
        // edge lengths are ordered LBTR to match distances passed to inset() and outset().
        skvx::float4 getEdgeLengths();

        // Determine if the original device space quad has vertices closer than 1px to its opposing
        // edges, without going through the full work of computing the insets (assuming that the
        // inset distances would be 0.5px).
        bool isSubpixel();

    private:
        // NOTE: This struct is named 'EdgeVectors' because it holds a lot of cached calculations
        // pertaining to the edge vectors of the input quad, projected into 2D device coordinates.
        // While they are not direction vectors, this struct represents a convenient storage space
        // for the projected corners of the quad.
        struct EdgeVectors {
            // Projected corners (x/w and y/w); these are the 2D coordinates that determine the
            // actual edge direction vectors, dx, dy, and invLengths
            skvx::float4 fX2D, fY2D;
            // Normalized edge vectors of the device space quad, ordered L, B, T, R
            // (i.e. next_ccw(x) - x).
            skvx::float4 fDX, fDY;
            // Reciprocal of edge length of the device space quad, i.e. 1 / sqrt(dx*dx + dy*dy)
            skvx::float4 fInvLengths;
            // Theta represents the angle formed by the two edges connected at each corner.
            skvx::float4 fCosTheta;
            skvx::float4 fInvSinTheta; // 1 / sin(theta)

            void reset(const skvx::float4& xs, const skvx::float4& ys,
                       const skvx::float4& ws, GrQuad::Type quadType);
        };

        struct EdgeEquations {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            skvx::float4 fA, fB, fC;

            void reset(const EdgeVectors& edgeVectors);

            skvx::float4 estimateCoverage(const skvx::float4& x2d,
                                          const skvx::float4& y2d) const;

            bool isSubpixel(const skvx::float4& x2d, const skvx::float4& y2d) const;

            // Outsets or insets 'x2d' and 'y2d' in place. To be used when the interior is very
            // small, edges are near parallel, or edges are very short/zero-length. Returns number
            // of effective vertices in the degenerate quad.
            int computeDegenerateQuad(const skvx::float4& signedEdgeDistances,
                                      skvx::float4* x2d, skvx::float4* y2d,
                                      skvx::Vec<4, int32_t>* aaMask) const;
        };

        struct OutsetRequest {
            // Positive edge distances to move each edge of the quad. These distances represent the
            // shortest (perpendicular) distance between the original edge and the inset or outset
            // edge. If the distance is 0, then the edge will not move.
            skvx::float4 fEdgeDistances;
            // True if the new corners cannot be calculated by simply adding scaled edge vectors.
            // The quad may be degenerate because of the original geometry (near colinear edges), or
            // be because of the requested edge distances (collapse of inset, etc.)
            bool fInsetDegenerate;
            bool fOutsetDegenerate;

            void reset(const EdgeVectors& edgeVectors, GrQuad::Type quadType,
                       const skvx::float4& edgeDistances);
        };

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            skvx::float4 fX, fY, fW;
            // U, V, and R coordinates representing local quad.
            // Ignored depending on uvrCount (0, 1, 2).
            skvx::float4 fU, fV, fR;
            int fUVRCount;

            void reset(const GrQuad& deviceQuad, const GrQuad* localQuad);

            void asGrQuads(GrQuad* deviceOut, GrQuad::Type deviceType,
                           GrQuad* localOut, GrQuad::Type localType) const;

            // Update the device and optional local coordinates by moving the corners along their
            // edge vectors such that the new edges have moved 'signedEdgeDistances' from their
            // original lines. This should only be called if the 'edgeVectors' fInvSinTheta data is
            // numerically sound.
            void moveAlong(const EdgeVectors& edgeVectors,
                           const skvx::float4& signedEdgeDistances);

            // Update the device coordinates by deriving (x,y,w) that project to (x2d, y2d), with
            // optional local coordinates updated to match the new vertices. It is assumed that
            // 'mask' was respected when determining (x2d, y2d), but it is used to ensure that only
            // unmasked unprojected edge vectors are used when computing device and local coords.
            void moveTo(const skvx::float4& x2d,
                        const skvx::float4& y2d,
                        const skvx::Vec<4, int32_t>& mask);
        };

        Vertices            fOriginal;
        EdgeVectors         fEdgeVectors;
        GrQuad::Type        fDeviceType;
        GrQuad::Type        fLocalType;

        // Lazily computed as needed; use accessor functions instead of direct access.
        OutsetRequest       fOutsetRequest;
        EdgeEquations       fEdgeEquations;

        // Validity of Vertices/EdgeVectors (always true after first call to set()).
        bool fVerticesValid      = false;
        // Validity of outset request (true after calling getOutsetRequest() until next set() call
        // or next inset/outset() with different edge distances).
        bool fOutsetRequestValid = false;
        // Validity of edge equations (true after calling getEdgeEquations() until next set() call).
        bool fEdgeEquationsValid = false;

        // The requested edge distances must be positive so that they can be reused between inset
        // and outset calls.
        const OutsetRequest& getOutsetRequest(const skvx::float4& edgeDistances);
        const EdgeEquations& getEdgeEquations();

        // Outsets or insets 'vertices' by the given perpendicular 'signedEdgeDistances' (inset or
        // outset is determined implicitly by the sign of the distances).
        void adjustVertices(const skvx::float4& signedEdgeDistances, Vertices* vertices);
        // Like adjustVertices() but handles empty edges, collapsed quads, numerical issues, and
        // returns the number of effective vertices in the adjusted shape.
        int adjustDegenerateVertices(const skvx::float4& signedEdgeDistances,
                                     Vertices* vertices);

        friend int ClipToW0(DrawQuad*, DrawQuad*); // To reuse Vertices struct
    };

} // namespace GrQuadUtils

void GrQuadUtils::Outset(const skvx::float4& edgeDistances, GrQuad* quad) {
    TessellationHelper outsetter;
    outsetter.reset(*quad, nullptr);
    outsetter.outset(edgeDistances, quad, nullptr);
}

#endif
