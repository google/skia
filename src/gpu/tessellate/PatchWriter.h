/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PatchWriter_DEFINED
#define tessellate_PatchWriter_DEFINED

#include "include/private/SkColorData.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
class StrokeTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                size_t patchStride,
                int initialAllocCount)
            : fAttribs(attribs)
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {}

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*, int initialPatchAllocCount);
#endif

    ~PatchWriter() {
        // finishStrokeContour() should have been called before this was deleted (or never used).
        SkASSERT(!fHasDeferredPatch && !fHasJoinControlPoint);
    }

    PatchAttribs attribs() const { return fAttribs; }

    // Updates the stroke's join control point that will be written out with each patch. This is
    // automatically adjusted when appending various geometries (e.g. Conic/Cubic), but sometimes
    // must be set explicitly.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint && lastControlPoint.isFinite());
        fJoinControlPointAttrib = lastControlPoint;
        fHasJoinControlPoint = true;
    }
    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information. Automatically resets the join control point attribute.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void writeDeferredStrokePatch() {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (fHasDeferredPatch) {
            SkASSERT(fHasJoinControlPoint);
            // Overwrite join control point with updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatchStorage, 4 * sizeof(SkPoint)),
                   &fJoinControlPointAttrib, sizeof(SkPoint));
            if (VertexWriter vw = fChunker.appendVertex()) {
                vw << VertexWriter::Array<char>(fDeferredPatchStorage, fChunker.stride());
            }
        }

        fHasDeferredPatch = false;
        fHasJoinControlPoint = false;
    }
    // TODO: These are only used by StrokeHardwareTessellator and ideally its patch writing logic
    // should be simplified like StrokeFixedCountTessellator's, and then this can go away.
    void resetJoinControlPointAttrib() {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        // Should have already been written or caller should manually defer
        SkASSERT(!fHasDeferredPatch);
        fHasJoinControlPoint = false;
    }
    SkPoint joinControlPoint() const { return fJoinControlPointAttrib; }
    bool hasJoinControlPoint() const { return fHasJoinControlPoint; }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    // PatchAttribs::kFanPoint must be enabled.
    void updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPointAttrib = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    // PatchAttribs::kStrokeParams must be enabled.
    void updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParamsAttrib = strokeParams;
    }

    // Updates the color that will be written out with each patch.
    // PatchAttribs::kColor must be enabled.
    void updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        fColorAttrib.set(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
    }

    /**
     * Helper structs that represent a path verb and geometry, that can be used to convert to
     * patches of instance data using operator<< on the PatchWriter, e.g.
     *
     *   patchWriter << Cubic(p0, p1, p2, p3);
     *   patchWriter << Conic(p0, p1, p2, w);
     *   patchWriter << Quadratic(p0, p1, p2);
     *   patchWriter << Line(p0, p1);
     *   patchWriter << Triangle(p0, p1, p2);
     *   patchWriter << Circle(p); // only for strokes
     *
     * Every geometric type is converted to an equivalent cubic or conic, so this will always
     * write at minimum 8 floats for the four control points (cubic) or three control points and
     * {w, inf} (conics). The PatchWriter additionally writes the current values of all attributes
     * enabled in its PatchAttribs flags.
     */

    // Defines a cubic curve from four control points.
    struct Cubic {
        Cubic(float2 p0, float2 p1, float2 p2, float2 p3) : fP0(p0), fP1(p1), fP2(p2), fP3(p3) {}
        Cubic(float4 p0p1, float4 p2p3) : fP0(p0p1.lo), fP1(p0p1.hi), fP2(p2p3.lo), fP3(p2p3.hi) {}
        Cubic(float2 p0, float4 p1p2, float2 p3) : fP0(p0), fP1(p1p2.lo), fP2(p1p2.hi), fP3(p3) {}
        Cubic(const SkPoint p[4]) : Cubic(float4::Load(p), float4::Load(p + 2)) {}

        float2 fP0, fP1, fP2, fP3;
    };

    // Defines a conic curve from three control points and 'w'.
    struct Conic {
        Conic(float2 p0, float2 p1, float2 p2, float w) : fP0(p0), fP1(p1), fP2(p2), fW(w) {}
        Conic(const SkPoint p[3], float w)
                : fP0(float2::Load(p))
                , fP1(float2::Load(p + 1))
                , fP2(float2::Load(p + 2))
                , fW(w) {}

        float2 fP0, fP1, fP2;
        float  fW;
    };


    // Defines a quadratic curve that is automatically converted into an equivalent cubic.
    struct Quadratic {
        Quadratic(float2 p0, float2 p1, float2 p2) : fP0(p0), fP1(p1), fP2(p2) {}
        Quadratic(const SkPoint p[3])
                : fP0(float2::Load(p))
                , fP1(float2::Load(p + 1))
                , fP2(float2::Load(p + 2)) {}

        Cubic asCubic() const {
            return Cubic(fP0, mix(float4(fP0, fP2), fP1.xyxy(), 2/3.f), fP2);
        }

        float2 fP0, fP1, fP2;
    };

    // Defines a line that will be automatically converted into an equivalent cubic patch.
    struct Line {
        Line(float4 p0p1) : fP0{p0p1.lo}, fP1{p0p1.hi} {}
        Line(float2 p0, float2 p1) : fP0{p0}, fP1{p1} {}
        Line(SkPoint p0, SkPoint p1)
                : fP0{skvx::bit_pun<float2>(p0)}
                , fP1{skvx::bit_pun<float2>(p1)} {}

        Cubic asCubic() const {
            float4 p0p1{fP0, fP1};
            return Cubic(fP0, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, fP1);
        }

        float2 fP0, fP1;
    };

    // Defines a triangle that will be automatically converted into a conic patch (w = inf).
    struct Triangle {
        Triangle(float2 p0, float2 p1, float2 p2) : fP0(p0), fP1(p1), fP2(p2) {}
        Triangle(SkPoint p0, SkPoint p1, SkPoint p2)
                : fP0(skvx::bit_pun<float2>(p0))
                , fP1(skvx::bit_pun<float2>(p1))
                , fP2(skvx::bit_pun<float2>(p2)) {}

        float2 fP0, fP1, fP2;
    };

    // Defines a circle used for round caps and joins in stroking, encoded as a cubic with
    // identical control points and an empty join.
    struct Circle {
        Circle(float2 p) : fP(p) {}
        Circle(SkPoint p) : fP(skvx::bit_pun<float2>(p)) {}

        float2 fP;
    };

    // operator<< definitions for the patch geometry types.

    SK_ALWAYS_INLINE void operator<<(const Cubic& cubic) {
        // TODO: Have cubic store or automatically compute wang's formula so this can automatically
        // call into chopAndWriteCubics.
        this->writePatch(cubic.fP0, cubic.fP1, cubic.fP2, cubic.fP3,
                         kCubicCurveType);
    }

    SK_ALWAYS_INLINE void operator<<(const Conic& conic) {
        // TODO: Have Conic store or automatically compute Wang's formula so this can automatically
        // call into chopAndWriteConics.
        this->writePatch(conic.fP0, conic.fP1, conic.fP2, {conic.fW, SK_FloatInfinity},
                         kConicCurveType);
    }

    // TODO: Have Quadratic store or automatically compute Wang's formula so this can automatically
    // call into chopAndWriteQuadratics *before* it is converted to an equivalent cubic if needed.
    SK_ALWAYS_INLINE void operator<<(const Quadratic& quad) { *this << quad.asCubic(); }

    SK_ALWAYS_INLINE void operator<<(const Line& line) {
        // Lines are specially encoded as [p0,p0,p1,p1] and detected in the shaders; if that
        // isn't desired, convert to a Cubic directly with Line::asCubic().
        this->writePatch(line.fP0, line.fP0, line.fP1, line.fP1, kCubicCurveType);
    }

    SK_ALWAYS_INLINE void operator<<(const Triangle& tri) {
        // Mark this patch as a triangle by setting it to a conic with w=Inf, and use a distinct
        // explicit curve type for when inf isn't supported in shaders.
        this->writePatch(tri.fP0, tri.fP1, tri.fP2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriangularConicCurveType);
    }

    SK_ALWAYS_INLINE void operator<<(const Circle& circle) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoinControlPointAttrib and never defers.
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (VertexWriter vw = fChunker.appendVertex()) {
            vw << VertexWriter::Repeat<4>(circle.fP); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), circle.fP, kCubicCurveType);
        }
    }

    SK_ALWAYS_INLINE void operator<<(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            *this << Triangle(p0, p1, p2);
        }
    }

    // Chops the given quadratic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches);

    // Chops the given conic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches);

    // Chops the given cubic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches);

private:
    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    void emitPatchAttribs(VertexWriter vertexWriter,
                          float2 joinControlPoint,
                          float explicitCurveType) {
        vertexWriter << If((fAttribs & PatchAttribs::kJoinControlPoint), joinControlPoint)
                     << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    SK_ALWAYS_INLINE
    void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
        const bool defer = (fAttribs & PatchAttribs::kJoinControlPoint) &&
                           !fHasJoinControlPoint;

        SkASSERT(!defer || !fHasDeferredPatch);
        SkASSERT(fChunker.stride() <= kMaxStride);
        VertexWriter vw = defer ? VertexWriter{fDeferredPatchStorage, fChunker.stride()}
                                : fChunker.appendVertex();
        fHasDeferredPatch |= defer;

        if (vw) {
            vw << p0 << p1 << p2 << p3;
            // NOTE: fJoinControlPointAttrib will contain NaN if we're writing to a deferred
            // patch. If that's the case, correct data will overwrite it when the contour is
            // closed (this is fine since a deferred patch writes to CPU memory instead of
            // directly to the GPU buffer).
            this->emitPatchAttribs(std::move(vw),
                                   skvx::bit_pun<float2>(fJoinControlPointAttrib),
                                   explicitCurveType);
            // Automatically update join control point for next patch.
            if (fAttribs & PatchAttribs::kJoinControlPoint) {
                fHasJoinControlPoint = true;
                if (explicitCurveType == kCubicCurveType && any(p3 != p2)) {
                    // p2 is control point defining the tangent vector into the next patch.
                    p2.store(&fJoinControlPointAttrib);
                } else if (any(p2 != p1)) {
                    // p1 is the control point defining the tangent vector.
                    p1.store(&fJoinControlPointAttrib);
                } else {
                    // p0 is the control point defining the tangent vector.
                    p0.store(&fJoinControlPointAttrib);
                }
            }
        }
    }

    const PatchAttribs fAttribs;
    GrVertexChunkBuilder fChunker;

    SkPoint fJoinControlPointAttrib;
    SkPoint fFanPointAttrib;
    StrokeParams fStrokeParamsAttrib;
    VertexColor fColorAttrib;

    bool fHasJoinControlPoint = false;

    static constexpr size_t kMaxStride =
            4 * sizeof(SkPoint) + // control points
                sizeof(SkPoint) + // join control point or fan attrib point (not used at same time)
                sizeof(StrokeParams) + // stroke params
            4 * sizeof(uint32_t); // wide vertex color

    // Only used if kJoinControlPointAttrib is set in fAttribs, in which case it holds data for
    // a single patch waiting for the incoming join control point to be computed.
    // Contents are valid (sans join control point) if fHasDeferredPatch is true.
    char fDeferredPatchStorage[kMaxStride];
    bool fHasDeferredPatch = false;
};

}  // namespace skgpu

#endif  // tessellate_PatchWriter_DEFINED
