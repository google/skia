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
#include "src/gpu/tessellate/Tessellation.h"

#define AI SK_ALWAYS_INLINE

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
     * writeX functions for supported patch geometry types. Every geometric type is converted to an
     * equivalent cubic or conic, so this will always write at minimum 8 floats for the four control
     * points (cubic) or three control points and {w, inf} (conics). The PatchWriter additionally
     * writes the current values of all attributes enabled in its PatchAttribs flags.
     */

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        // TODO: Have cubic store or automatically compute wang's formula so this can automatically
        // call into chopAndWriteCubics.
        this->writePatch(p0, p1, p2, p3, kCubicCurveType);
    }
    AI void writeCubic(float4 p0p1, float4 p2p3) {
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi);
    }
    AI void writeCubic(float2 p0, float4 p1p2, float2 p3) {
        this->writeCubic(p0, p1p2.lo, p1p2.hi, p3);
    }
    AI void writeCubic(const SkPoint pts[4]) {
        this->writeCubic(float4::Load(pts), float4::Load(pts + 2));
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w) {
        // TODO: Have Conic store or automatically compute Wang's formula so this can automatically
        // call into chopAndWriteConics.
        this->writePatch(p0, p1, p2, {w, SK_FloatInfinity}, kConicCurveType);
    }
    AI void writeConic(const SkPoint pts[3], float w) {
        this->writeConic(skvx::bit_pun<float2>(pts[0]),
                         skvx::bit_pun<float2>(pts[1]),
                         skvx::bit_pun<float2>(pts[2]),
                         w);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2) {
        // TODO: Have Quadratic store or automatically compute Wang's formula so this can
        // automatically  call into chopAndWriteQuadratics *before* it is converted to an equivalent
        // cubic if needed.
        this->writeCubic(p0, mix(float4(p0, p2), p1.xyxy(), 2/3.f), p2);
    }
    AI void writeQuadratic(const SkPoint pts[3]) {
        this->writeQuadratic(skvx::bit_pun<float2>(pts[0]),
                             skvx::bit_pun<float2>(pts[1]),
                             skvx::bit_pun<float2>(pts[2]));
    }

    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLine(float4 p0p1) {
        this->writeCubic(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
    }
    AI void writeLine(float2 p0, float2 p1) { this->writeLine({p0, p1}); }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(skvx::bit_pun<float2>(p0), skvx::bit_pun<float2>(p1));
    }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        this->writePatch(p0, p1, p2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriangularConicCurveType);
    }
    AI void writeTriangle(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->writeTriangle(skvx::bit_pun<float2>(p0),
                            skvx::bit_pun<float2>(p1),
                            skvx::bit_pun<float2>(p2));
    }

    // Writes a circle used for round caps and joins in stroking, encoded as a cubic with
    // identical control points and an empty join.
    AI void writeCircle(SkPoint p) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoinControlPointAttrib and never defers.
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (VertexWriter vw = fChunker.appendVertex()) {
            vw << VertexWriter::Repeat<4>(p); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), p, kCubicCurveType);
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
                          SkPoint joinControlPoint,
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
            this->emitPatchAttribs(std::move(vw), fJoinControlPointAttrib, explicitCurveType);
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

#undef AI

#endif  // tessellate_PatchWriter_DEFINED
