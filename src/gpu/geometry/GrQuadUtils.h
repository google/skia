/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadUtils_DEFINED
#define GrQuadUtils_DEFINED

#include "include/private/skvx.h"

enum class GrQuadAAFlags;
enum class GrAA : bool;
enum class GrAAType : unsigned;
class GrQuad;
struct SkRect;

namespace GrQuadUtils {

    // Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
    // Both outAAType and outEdgeFlags will be updated.
    void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                       const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

    /**
     * Crops this quad to the provided device-space axis-aligned rectangle. If the intersection of
     * this quad (projected) and clipDevRect results in a quadrilateral, this returns true. If not,
     * this quad will be updated to be the smallest quad of the same type such that its intersection
     * with clipDevRect is visually the same.
     *
     * The provided edge flags are updated to reflect edges clipped by clipDevRect (toggling on or
     * or off based on clipAA policy). The provided local coordinates will be updated to reflect
     * the updated device coordinates of this quad.
     *
     * 'local' may be null, in which case the new local coordinates will not be calculated. This is
     * useful when it's known a paint does not require local coordinates. However, 'edgeFlags'
     * cannot be null.
     */
    bool CropToRect(const SkRect& cropRect, GrAA cropAA, GrQuadAAFlags* edgeAA, GrQuad* quad,
                    GrQuad* local=nullptr);

    // Stateful helper for processing and manipulating the vertices of a device-space GrQuad
    // and optional local coordinates.
    class VertexHelper {
    public:
        // Provide nullptr if there are no local coordinates to track
        VertexHelper(const GrQuad& deviceQuad, const GrQuad* localQuad);

        // Calculates a new quadrilateral that had edges parallel to the original except that they
        // have been moved inwards by edgeDistances (which should be positive). Distances are
        // ordered L, B, T, R to match CCW tristrip ordering of GrQuad vertices. Edges that are
        // not moved (i.e. distance == 0) will not be used in calculations and the corners will
        // remain on that edge.
        //
        // The number of logical vertices will be returned, between 1 and 4 depending on if the
        // inset geometry collapses to a point, line or triangle. The inset geometry will be
        // written to 'deviceInset' and its corresponding interpolated local coordinates will be
        // written to 'localInset'. If this helper was not provided local coordinates, 'localInset'
        // can be null, otherwise it must be non-null.
        //
        // Note: the edge distances are in device pixel units, so after rendering the new quad
        // edge's shortest distance to the original quad's edge would be equal to provided edge dist
        int inset(const skvx::Vec<4, float>& edgeDistances,
                  GrQuad* deviceInset, GrQuad* localInset) {
            return this->adjustQuad(edgeDistances, false, deviceInset, localInset);
        }

        // Calculates a new quadrilateral that outsets the original edges by the given distances.
        // Other than moving edges outwards, this function is equivalent to inset(). If the exact
        // same edge distances are provided, certain internal computations can be reused across
        // subsequent calls to inset() and outset().
        int outset(const skvx::Vec<4, float>& edgeDistances,
                   GrQuad* deviceOutset, GrQuad* localOutset) {
            return this->adjustQuad(edgeDistances, true, deviceOutset, localOutset);
        }

        // Calculate area of intersection of the projected quad with the device space rectangle.
        float areaOfIntersection(const SkRect& deviceRect);

        // Calculate area of intersection of the quad projected to 2D with a pixel centered at the
        // given point. The pixel center can be located on fractional coordinates.
        float pixelCoverage(const SkPoint& pixelCenter) {
            return this->areaOfIntersection(SkRect::MakeLTRB(pixelCenter.fX - 0.5f,
                                                             pixelCenter.fY - 0.5f,
                                                             pixelCenter.fX + 0.5f,
                                                             pixelCenter.fY + 0.5f));
        }

        // Returns edge equations such that a * x + b * y + c returns signed distance of a point
        // (x, y) to the projected edges of the quad. Positive distances are inside the quad.
        // Returns true if the edge normals had to be flipped to point inside (i.e. the quad was
        // transformed by a negative scale).
        bool edgeEquations(skvx::Vec<4, float>* a, skvx::Vec<4, float>* b, skvx::Vec<4, float>* c) {
            const EdgeEquations& e = this->edgeEquations();
            *a = e.fA;
            *b = e.fB;
            *c = e.fC;
            return e.fFlipped;
        }

    private:
        // Internal structures that may be calculated and reused across multiple calculations

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            skvx::Vec<4, float> fX, fY, fW;
            // U, V, and R coordinates representing local quad. Ignored based on uvrCount (0, 2, 3).
            skvx::Vec<4, float> fU, fV, fR;
            int fUVRCount;

            Vertices() = default;
            Vertices(const GrQuad& deviceQuad, const GrQuad* localQuad);

            void moveAlong(const EdgeVectors& edgeVectors, const Sk4f& outset, const Sk4f& mask);

            void moveTo(const Sk4f& x2d, const Sk4f& y2d, const Sk4f& mask);
        };

        struct EdgeVectors {
            // Normalized edge vectors of the device space quad, ordered L, B, T, R
            // (i.e. nextCCW(x) - x).
            skvx::Vec<4, float> fDX, fDY;
            // 1 / edge length of the device space quad
            skvx::Vec<4, float> fInvLengths;
        };

        struct EdgeEquations {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            skvx::Vec<4, float> fA, fB, fC;
            // Whether or not the edge normals had to be flipped to preserve positive distance
            bool fFlipped;
        };

        struct OutsetInfo {
            // Amount to move along each edge vector for an inset (mul by -1) or outset (mul by +1),
            // unless fDegenerate is true, in which case slower code paths have to be taken. For
            // convenience, fOutsets = fEdgeDistances when fDegenerate is true
            skvx::Vec<4, float> fOutsets;
            // The requested edge distances to move for insetting or outsetting
            skvx::Vec<4, float> fEdgeDistances;
            // 1.f if fEdgeDistances != 0, 0.f if == 0.
            skvx::Vec<4, float> fMask;
            bool fDegenerate;
        };

        // Computes fX2D, fY2D and sets fProjectedCornersValid if needed
        void ensureProjectedCoords();

        // Wraps fX2D, fY2D in a Vertices with no local coordinates
        Vertices projectedVertices();

        // Computes fEdgeVectors and sets fEdgeVectorsValid if needed
        const EdgeVectors& edgeVectors();

        // Computes fEdgeEquations and sets fEdgeEquationsValid if needed
        const EdgeEquations& edgeEquations();

        // Computes fOutsetInfo and sets fOutsetInfoValid if needed (invalid or new distances)
        const OutsetInfo& outsetInfo(const skvx::Vec<4, float>& edgeDistances);

        // Calculate 2D projected coordinates of quad with edges adjusted by signedEdgeDistances,
        // respecting mask, relying on solving edge equation intersections and handling all
        // degenerate conditions. x2d and y2d are updated in place.
        int calculateDegenerateQuad(const skvx::Vec<4, float>& signedEdgeDistances,
                                    const skvx::Vec<4, float>& mask,
                                    skvx::Vec<4, float>* x2d, skvx::Vec<4, float>* y2d) const;

        int adjustQuad(const skvx::Vec<4, float>& edgeDistances, bool outset,
                       GrQuad* deviceOut, GrQuad* localOut);

        // Always valid, will not be modified
        const Vertices fOriginalVertices;
        const GrQuad::Type fDeviceType; // FIXME have to track local quad type so that we can set it appropriately on output

        // Projection of perspective device corners into 2D. Requires fProjectedCornersValid.
        // These are only lazily calculated if the original device quad has perspective.
        skvx::Vec<4, float> fX2D;
        skvx::Vec<4, float> fY2D;
        // Edge vector information. Requires fEdgeVectorsValid. Lazily calculated
        EdgeVectors fEdgeVectors;
        // Edge equations. Requires fEdgeEquationsValid. Lazily calculated if the quad
        // is degenerate, or if requested with edgeEquations() or areaOfIntersection().
        EdgeEquations fEdgeEquations;
        // Cached processing of requested edge distances, to be reused between inset()/outset()
        // calls with the same edge distance inputs. Requires fOutsetInfoValid.
        OutsetInfo    fOutsetInfo;

        // Validity of cached geometric state
        unsigned fProjectedCornersValid : 1;
        unsigned fEdgeVectorsValid      : 1;
        unsigned fEdgeEquationsValid    : 1;
        unsigned fOutsetInfoValid       : 1;
    };

}; // namespace GrQuadUtils

#endif
