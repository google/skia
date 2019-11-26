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

    enum class DeviceMode :int {
        // All device-space quads submitted to the TessellationHelper will have
        // GrQuad::Type::kRectilinear or kAxisAligned (a subclass of rectilinear for its purposes).
        // Fast paths will always be taken.
        kAssumeRectilinear,
        // The device-space quads submitted may have any GrQuad::Type; fast paths will be taken
        // on a per-quad basis.
        kAny
    };

    enum class LocalMode :int{
        // The localQuad will be ignored and is allowed to be null
        kNone,
        // The localQuad will have its x and y coordinates updated, asserts that it's never null
        // and that its type is not kPerspective
        kUV,
        // The localQuad will have its x, y, and w coordinates updated, asserts that it's never
        // null and supports any type of local coords.
        kUVR,
    };

    class TessellationHelper {
    public:
        // A tessellation helper that handles any device quad type and any local quad type.
        TessellationHelper();
        // A tessellation helper that operates with the given device and local mode optimizations.
        TessellationHelper(DeviceMode deviceMode, LocalMode localMode);
        // Convenience to choose the device mode and local mode appropriate for ops that are
        // limited to the device type and local coordinate dimensionality (0, 2, or 3).
        TessellationHelper(GrQuad::Type deviceType, int localDimensionality);

        // Set the original device and (optional) local coordinates that are inset or outset
        // by the requested edge distances. Use nullptr if there are no local coordinates to update.
        // Must meet the requirements of the DeviceMode and LocalMode the helper was created with.
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

            template<DeviceMode opt>
            static void Reset(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
                              const skvx::Vec<4, float>& ws, GrQuad::Type quadType,
                              EdgeVectors* edgeVec);
            // Type of all EdgeVectors::Reset() instantiations.
            typedef void (*ResetProc)(const skvx::Vec<4, float>&, const skvx::Vec<4, float>&,
                                      const skvx::Vec<4, float>&, GrQuad::Type, EdgeVectors*);
        };

        // NOTE: EdgeEquations has no specializations for DeviceMode or LocalMode so it employs
        // no static template function trickery.
        struct EdgeEquations {
            // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
            skvx::Vec<4, float> fA, fB, fC;

            void reset(const EdgeVectors& edgeVectors);

            skvx::Vec<4, float> estimateCoverage(const skvx::Vec<4, float>& x2d,
                                                 const skvx::Vec<4, float>& y2d) const;

            // Outsets or insets 'x2d' and 'y2d' in place. To be used when the interior is very
            // small, edges are near parallel, or edges are very short/zero-length. Returns number
            // of effective vertices in the degenerate quad.
            int computeDegenerateQuad(const skvx::Vec<4, float>& signedEdgeDistances,
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

            template<DeviceMode opt>
            static void Reset(const EdgeVectors& edgeVectors, GrQuad::Type quadType,
                              const skvx::Vec<4, float>& edgeDistances,
                              OutsetRequest* outReq);
            // Type of all OutsetRequest::Reset() instantiations
            typedef void (*ResetProc)(const EdgeVectors&, GrQuad::Type, const skvx::Vec<4, float>&,
                                      OutsetRequest*);
        };

        struct Vertices {
            // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
            skvx::Vec<4, float> fX, fY, fW;
            // U, V, and R coordinates representing local quad.
            // Ignored depending on uvrCount (0, 1, 2).
            skvx::Vec<4, float> fU, fV, fR;

            template<DeviceMode devOpt, LocalMode localOpt>
            static void Reset(const GrQuad& deviceQuad, const GrQuad* localQuad, Vertices* verts);
            // Type of all Vertices::Reset() instantiations
            typedef void (*ResetProc)(const GrQuad&, const GrQuad*, Vertices*);

            template<DeviceMode devOpt, LocalMode localOpt>
            static void AsGrQuads(const Vertices& verts, GrQuad* deviceOut, GrQuad::Type deviceType,
                                  GrQuad* localOut, GrQuad::Type localType);
            // Type of all Vertices::AsGrQuads() instantiations
            typedef void (*AsGrQuadsProc)(const Vertices&, GrQuad*, GrQuad::Type,
                                          GrQuad*, GrQuad::Type);

            // Update the device and optional local coordinates by moving the corners along their
            // edge vectors such that the new edges have moved 'signedEdgeDistances' from their
            // original lines. This should only be called if the 'edgeVectors' fInvSinTheta data is
            // numerically sound.
            template<LocalMode opt>
            static void MoveAlong(const EdgeVectors& edgeVectors,
                                  const skvx::Vec<4, float>& signedEdgeDistances,
                                  Vertices* verts);
            // Type of all Vertices::MoveAlong() instantiations
            typedef void (*MoveAlongProc)(const EdgeVectors&, const skvx::Vec<4, float>&,
                                          Vertices*);

            // Update the device coordinates by deriving (x,y,w) that project to (x2d, y2d), with
            // optional local coordinates updated to match the new vertices. It is assumed that
            // 'mask' was respected when determining (x2d, y2d), but it is used to ensure that only
            // unmasked unprojected edge vectors are used when computing device and local coords.
            template<LocalMode opt>
            static void MoveTo(const skvx::Vec<4, float>& x2d,
                               const skvx::Vec<4, float>& y2d,
                               const skvx::Vec<4, int32_t>& mask,
                               Vertices* verts);
            // Type of all Vertices::MoveTo instantiations
            typedef void (*MoveToProc)(const skvx::Vec<4, float>&, const skvx::Vec<4, float>&,
                                       const skvx::Vec<4, int32_t>&, Vertices*);
        };

        // Type of all TessellationHelper::Adjust[Degenerate]Vertices instantiations. The
        // mutable TessellationHelper is included to provide access to its cached metadata.
        typedef int (*AdjustVerticesProc)(const skvx::Vec<4, float>&, TessellationHelper*,
                                          Vertices*);

        template<DeviceMode kDM, LocalMode kLM>
        struct Config {
            static constexpr DeviceMode kDeviceMode = kDM;
            static constexpr LocalMode kLocalMode = kLM;
        };

        // Dynamic way to go from the untemplated TessellationHelper's input modes to the resolved
        // function pointers that refer to one of the static template functions. The static
        // functions use templates because it makes code reuse convenient, but this lets us prevent
        // templates from bubbling up to the users of TessellationHelper.
        struct FunctionTable {
            AdjustVerticesProc       fAdjustProc;
            AdjustVerticesProc       fAdjustDegenerateProc;

            EdgeVectors::ResetProc   fResetEdgeVectorsProc;
            OutsetRequest::ResetProc fResetOutsetRequestProc;
            Vertices::ResetProc      fResetVerticesProc;

            Vertices::AsGrQuadsProc  fAsGrQuadsProc;

            // May not need these...
            // Vertices::MoveAlongProc  fMoveAlongProc;
            // Vertices::MoveToProc     fMoveToProc;

            template<typename C>
            constexpr FunctionTable(C c)
                : fAdjustProc( AdjustVertices<C::kDeviceMode, C::kLocalMode>)
                , fAdjustDegenerateProc(AdjustDegenerateVertices<C::kDeviceMode, C::kLocalMode>)
                , fResetEdgeVectorsProc(EdgeVectors::Reset<C::kDeviceMode>)
                , fResetOutsetRequestProc(OutsetRequest::Reset<C::kDeviceMode>)
                , fResetVerticesProc(Vertices::Reset<C::kDeviceMode, C::kLocalMode>)
                , fAsGrQuadsProc(Vertices::AsGrQuads<C::kDeviceMode, C::kLocalMode>) {}
        };
        // 2 device modes X 3 local modes = 6 total function table configs
        static FunctionTable kFunctionTables[6];
        static const FunctionTable* GetFunctionTable(DeviceMode, LocalMode);

        const FunctionTable* fFunctions; // Refers to an entry in kFunctionTables
        Vertices             fOriginal;
        EdgeVectors          fEdgeVectors;
        GrQuad::Type         fDeviceType;
        GrQuad::Type         fLocalType;

        // Lazily computed as needed; use accessor functions instead of direct access.
        OutsetRequest        fOutsetRequest;
        EdgeEquations        fEdgeEquations;

        // Validity of Vertices/EdgeVectors (always true after first call to set()).
        bool fVerticesValid      = false;
        // Validity of outset request (true after calling getOutsetRequest() until next set() call
        // or next inset/outset() with different edge distances).
        bool fOutsetRequestValid = false;
        // Validity of edge equations (true after calling getEdgeEquations() until next set() call).
        bool fEdgeEquationsValid = false;

        // The requested edge distances must be positive so that they can be reused between inset
        // and outset calls.
        const OutsetRequest& getOutsetRequest(const skvx::Vec<4, float>& edgeDistances);
        const EdgeEquations& getEdgeEquations();

        // Outsets or insets 'vertices' by the given perpendicular 'signedEdgeDistances' (inset or
        // outset is determined implicitly by the sign of the distances).
        template<DeviceMode devOpt, LocalMode localOpt>
        static int AdjustVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                  TessellationHelper* helper, Vertices* vertices);
        // Like adjustVertices() but handles empty edges, collapsed quads, numerical issues, and
        // returns the number of effective vertices in the adjusted shape.
        template<DeviceMode devOpt, LocalMode localOpt>
        static int AdjustDegenerateVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                            TessellationHelper* helper, Vertices* vertices);
    };

}; // namespace GrQuadUtils

#endif
