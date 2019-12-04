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
        // Set the original device and (optional) local coordinates that are inset or outset
        // by the requested edge distances. Use nullptr if there are no local coordinates to update.
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
        skvx::Vec<4, float> inset(const skvx::Vec<4, float>& edgeDistances,
                                  GrQuad* deviceInset, GrQuad* localInset);

        // Calculates a new quadrilateral that outsets the original edges by the given distances.
        // Other than moving edges outwards, this function is equivalent to inset(). If the exact
        // same edge distances are provided, certain internal computations can be reused across
        // consecutive calls to inset() and outset() (in any order).
        void outset(const skvx::Vec<4, float>& edgeDistances,
                    GrQuad* deviceOutset, GrQuad* localOutset);

    private:
        // This enum is used to specialize the methods on the various following helper structs
        // when assumptions can be made on the geometric nature of the device coordinates.
        enum class DeviceMode : int {
            // All device-space quads submitted to the TessellationHelper will have
            // GrQuad::Type::kRectilinear or kAxisAligned (all corners are 90 degrees).
            // Fast paths will always be taken.
            kRectilinear,
            // The device-space quads submitted may have any GrQuad::Type; fast paths will be taken
            // on a per-quad basis.
            kAny
        };

        // Like DeviceMode, this is used to specialize the helper structs based on the
        // dimensionality of the local coordinates that are kept in-sync with device inset/outsets.
        enum class LocalMode : int {
            // The localQuad will be ignored and is allowed to be null
            kNone,
            // The localQuad will have its x and y coordinates updated, asserts that it's never null
            // and that its type is not kPerspective
            kUV,
            // The localQuad will have its x, y, and w coordinates updated, asserts that it's never
            // null and supports any type of local coords.
            kUVR,
        };

        // NOTE: This struct is named 'EdgeVectors' because it holds a lot of cached calculations
        // pertaining to the edge vectors of the input quad, projected into 2D device coordinates.
        // While they are not direction vectors, this struct represents a convenient storage space
        // for the projected corners of the quad.
        struct EdgeVectors {
            // Projected corners (x/w and y/w); these are the 2D coordinates that determine the
            // actual edge direction vectors, dx, dy, and invLengths
            skvx::Vec<4, float> fX2D, fY2D;
            // Normalized edge vectors of the device space quad, ordered L, B, T, R
            // (i.e. next_ccw(x) - x).
            skvx::Vec<4, float> fDX, fDY;
            // Reciprocal of edge length of the device space quad, i.e. 1 / sqrt(dx*dx + dy*dy)
            skvx::Vec<4, float> fInvLengths;
            // Theta represents the angle formed by the two edges connected at each corner.
            skvx::Vec<4, float> fCosTheta;
            skvx::Vec<4, float> fInvSinTheta; // 1 / sin(theta)

            template<DeviceMode kDM>
            SK_ALWAYS_INLINE void reset(const skvx::Vec<4, float>& xs,
                                        const skvx::Vec<4, float>& ys,
                                        const skvx::Vec<4, float>& ws,
                                        GrQuad::Type quadType);
        };

        struct EdgeEquations {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            skvx::Vec<4, float> fA, fB, fC;

            SK_ALWAYS_INLINE void reset(const EdgeVectors& edgeVectors);

            SK_ALWAYS_INLINE skvx::Vec<4, float> estimateCoverage(
                    const skvx::Vec<4, float>& x2d, const skvx::Vec<4, float>& y2d) const;

            // Outsets or insets 'x2d' and 'y2d' in place. To be used when the interior is very
            // small, edges are near parallel, or edges are very short/zero-length. Returns number
            // of effective vertices in the degenerate quad.
            SK_ALWAYS_INLINE int computeDegenerateQuad(
                    const skvx::Vec<4, float>& signedEdgeDistances,
                    skvx::Vec<4, float>* x2d, skvx::Vec<4, float>* y2d) const;
        };

        struct OutsetRequest {
            // Positive edge distances to move each edge of the quad. These distances represent the
            // shortest (perpendicular) distance between the original edge and the inset or outset
            // edge. If the distance is 0, then the edge will not move.
            skvx::Vec<4, float> fEdgeDistances;
            // True if the new corners cannot be calculated by simply adding scaled edge vectors.
            // The quad may be degenerate because of the original geometry (near colinear edges), or
            // be because of the requested edge distances (collapse of inset, etc.)
            bool fInsetDegenerate;
            bool fOutsetDegenerate;

            template<DeviceMode kDM>
            SK_ALWAYS_INLINE void reset(const EdgeVectors& edgeVectors, GrQuad::Type quadType,
                                        const skvx::Vec<4, float>& edgeDistances);
        };

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            skvx::Vec<4, float> fX, fY, fW;
            // U, V, and R coordinates representing local quad.
            // Ignored depending on the LocalMode of the function calls.
            skvx::Vec<4, float> fU, fV, fR;

            template<DeviceMode kDM, LocalMode kLM>
            SK_ALWAYS_INLINE void reset(const GrQuad& deviceQuad, const GrQuad* localQuad);

            template<DeviceMode kDM, LocalMode kLM>
            SK_ALWAYS_INLINE void asGrQuads(GrQuad* deviceOut, GrQuad::Type deviceType,
                                            GrQuad* localOut, GrQuad::Type localType) const;

            // Update the device and optional local coordinates by moving the corners along their
            // edge vectors such that the new edges have moved 'signedEdgeDistances' from their
            // original lines. This should only be called if the 'edgeVectors' fInvSinTheta data is
            // numerically sound.
            template<DeviceMode kDM, LocalMode kLM>
            SK_ALWAYS_INLINE void moveAlong(const EdgeVectors& edgeVectors,
                                            const skvx::Vec<4, float>& signedEdgeDistances);

            // Update the device coordinates by deriving (x,y,w) that project to (x2d, y2d), with
            // optional local coordinates updated to match the new vertices. It is assumed that
            // 'mask' was respected when determining (x2d, y2d), but it is used to ensure that only
            // unmasked unprojected edge vectors are used when computing device and local coords.
            template<LocalMode kLM>
            SK_ALWAYS_INLINE void moveTo(const skvx::Vec<4, float>& x2d,
                                         const skvx::Vec<4, float>& y2d,
                                         const skvx::Vec<4, int32_t>& mask);
        };

        // Dynamic way to go from the public, un-templated API functions reset/inset/outset to the
        // templated versions that use compile-time branch removal to specialize the tessellation
        // based on what's known about the quads.
        struct FunctionTable {
            typedef void (*ResetProc)(TessellationHelper*, const GrQuad&, const GrQuad*);
            typedef skvx::Vec<4, float> (*InsetProc)(TessellationHelper*,
                                                     const skvx::Vec<4, float>&,
                                                     GrQuad*, GrQuad*);
            typedef void (*OutsetProc)(TessellationHelper*, const skvx::Vec<4, float>&,
                                       GrQuad*, GrQuad*);

            // This is needed so we can specify the static function template parameters as
            // constructor arguments. The FunctionTable ctor cannot be templated if it has no args.
            template<DeviceMode kDM, LocalMode kLM>
            struct Config {
                static constexpr DeviceMode kDeviceMode = kDM;
                static constexpr LocalMode kLocalMode = kLM;
            };

            template<typename C>
            constexpr FunctionTable(C config)
                    : fResetProc(Reset<C::kDeviceMode, C::kLocalMode>)
                    , fInsetProc(Inset<C::kDeviceMode, C::kLocalMode>)
                    , fOutsetProc(Outset<C::kDeviceMode, C::kLocalMode>) {}

            ResetProc  fResetProc;
            InsetProc  fInsetProc;
            OutsetProc fOutsetProc;
        };
        // 2 device modes X 3 local modes = 6 total function table configs
        static FunctionTable kFunctionTables[6];
        static SK_ALWAYS_INLINE const FunctionTable* GetFunctionTable(DeviceMode, LocalMode);

        const FunctionTable* fFunctions = nullptr; // Refers to an entry in kFunctionTables
        Vertices             fOriginal;
        EdgeVectors          fEdgeVectors;
        GrQuad::Type         fDeviceType;
        GrQuad::Type         fLocalType;

        // Lazily computed as needed; use accessor functions instead of direct access.
        OutsetRequest       fOutsetRequest;
        EdgeEquations       fEdgeEquations;

        // Validity of outset request (true after calling getOutsetRequest() until next set() call
        // or next inset/outset() with different edge distances).
        bool fOutsetRequestValid = false;
        // Validity of edge equations (true after calling getEdgeEquations() until next set() call).
        bool fEdgeEquationsValid = false;

        // The requested edge distances must be positive so that they can be reused between inset
        // and outset calls.
        template<DeviceMode kDM>
        SK_ALWAYS_INLINE const OutsetRequest& getOutsetRequest(
                const skvx::Vec<4, float>& edgeDistances);
        SK_ALWAYS_INLINE const EdgeEquations& getEdgeEquations();

        // Outsets or insets 'vertices' by the given perpendicular 'signedEdgeDistances' (inset or
        // outset is determined implicitly by the sign of the distances).
        template<DeviceMode kDM, LocalMode kLM>
        SK_ALWAYS_INLINE void adjustVertices(
                const skvx::Vec<4, float>& signedEdgeDistances, Vertices* vertices);
        // Like adjustVertices() but handles empty edges, collapsed quads, numerical issues, and
        // returns the number of effective vertices in the adjusted shape.
        template<DeviceMode kDM, LocalMode kLM>
        SK_ALWAYS_INLINE int adjustDegenerateVertices(
                const skvx::Vec<4, float>& signedEdgeDistances, Vertices* vertices);

        // Specialized implementations of public API, filled in via FunctionTable.
        // All helper functions that these use are marked SK_ALWAYS_INLINE so we end up with
        // 6 blocks of skvx/vector instructions (w/o any branches for kDM == kRectilinear).

        template<DeviceMode kDM, LocalMode kLM>
        static void Reset(TessellationHelper*, const GrQuad& deviceQuad, const GrQuad* localQuad);

        template<DeviceMode kDM, LocalMode kLM>
        static skvx::Vec<4, float> Inset(TessellationHelper*,
                                         const skvx::Vec<4, float>& edgeDistances,
                                         GrQuad* deviceInset, GrQuad* localInset);

        template<DeviceMode kDM, LocalMode kLM>
        static void Outset(TessellationHelper*, const skvx::Vec<4, float>& edgeDistances,
                           GrQuad* deviceOutset, GrQuad* localOutset);
    };

}; // namespace GrQuadUtils

#endif
