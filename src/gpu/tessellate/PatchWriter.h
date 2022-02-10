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
#include "src/gpu/tessellate/WangsFormula.h"

#define AI SK_ALWAYS_INLINE

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
class StrokeTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
    using VectorXform = wangs_formula::VectorXform;
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                int maxTessellationSegments,
                size_t patchStride,
                int initialAllocCount)
            : fAttribs(attribs)
            , fMaxSegments_pow2(pow2(maxTessellationSegments))
            , fMaxSegments_pow4(pow2(fMaxSegments_pow2))
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {
        // For fans or strokes, the minimum required segment count is 1 (making either a triangle
        // with the fan point, or a stroked line). Otherwise, we need 2 segments to represent
        // triangles purely from the tessellated vertices.
        fCurrMinSegments_pow4 = (attribs & PatchAttribs::kFanPoint ||
                                 attribs & PatchAttribs::kJoinControlPoint) ? 1.f : 16.f; // 2^4
    }

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
#endif

    ~PatchWriter() {
        // finishStrokeContour() should have been called before this was deleted (or never used).
        SkASSERT(!fHasDeferredPatch && !fHasJoinControlPoint);
    }

    PatchAttribs attribs() const { return fAttribs; }

    // Fast log2 of minimum required # of segments per tracked Wang's formula calculations.
    int requiredResolveLevel() const {
        return wangs_formula::nextlog16(fCurrMinSegments_pow4); // log16(n^4) == log2(n)
    }
    // Fast minimum required # of segments from tracked Wang's formula calculations.
    int requiredFixedSegments() const {
        return SkScalarCeilToInt(wangs_formula::root4(fCurrMinSegments_pow4));
    }

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

    // Writes four control points manually prepared.
    // TODO: Only used by StrokeHardwareTessellator
    AI void writeHwPatch(const SkPoint p[4]) {
        float4 p0p1 = float4::Load(p);
        float4 p2p3 = float4::Load(p + 2);
        this->writePatch(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi, /*unused*/0.f);
    }

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float n4 = wangs_formula::cubic_pow4(precision, p0, p1, p2, p3, shaderXform);
        if (this->writesCurvesOnly()) {
            if (n4 <= 1.f) {
                // This cubic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->updateRequiredSegments(n4)) {
            this->writeCubicPatch(p0, p1, p2, p3);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                    std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteCubics(p0, p1, p2, p3, numPatches);
        }
    }
    AI void writeCubic(const SkPoint pts[4],
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float4 p0p1 = float4::Load(pts);
        float4 p2p3 = float4::Load(pts + 2);
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi, shaderXform, precision);
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float n2 = wangs_formula::conic_pow2(precision, p0, p1, p2, w, shaderXform);
        if (this->writesCurvesOnly()) {
            if (n2 <= 1.f) {
                // This conic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->updateRequiredSegments(n2*n2)) {
            this->writeConicPatch(p0, p1, p2, w);
        } else {
            int numPatches = SkScalarCeilToInt(sqrtf(
                    std::min(n2, pow2(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow2));
            this->chopAndWriteConics(p0, p1, p2, w, numPatches);
        }
    }
    AI void writeConic(const SkPoint pts[3], float w,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        this->writeConic(skvx::bit_pun<float2>(pts[0]),
                         skvx::bit_pun<float2>(pts[1]),
                         skvx::bit_pun<float2>(pts[2]),
                         w, shaderXform, precision);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2,
                           const VectorXform& shaderXform,
                           float precision = kTessellationPrecision) {
        float n4 = wangs_formula::quadratic_pow4(precision, p0, p1, p2, shaderXform);
        if (this->writesCurvesOnly()) {
            if (n4 <= 1.f) {
                // This quad only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->updateRequiredSegments(n4)) {
            this->writeQuadPatch(p0, p1, p2);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                    std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteQuads(p0, p1, p2, numPatches);
        }
    }
    AI void writeQuadratic(const SkPoint pts[3],
                           const VectorXform& shaderXform,
                           float precision = kTessellationPrecision) {
        this->writeQuadratic(skvx::bit_pun<float2>(pts[0]),
                             skvx::bit_pun<float2>(pts[1]),
                             skvx::bit_pun<float2>(pts[2]),
                             shaderXform, precision);
    }

    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLine(float4 p0p1) {
        // No chopping needed, and should have been reset to 1 segment if using writeLine
        SkASSERT(fCurrMinSegments_pow4 >= 1.f);
        this->writeCubicPatch(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
    }
    AI void writeLine(float2 p0, float2 p1) { this->writeLine({p0, p1}); }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(skvx::bit_pun<float2>(p0), skvx::bit_pun<float2>(p1));
    }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        // No chopping needed, and should have been reset to 2 segments if using writeTriangle.
        SkASSERT(fCurrMinSegments_pow4 >= (2*2*2*2));
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

private:
    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    AI void emitPatchAttribs(VertexWriter vertexWriter,
                             SkPoint joinControlPoint,
                             float explicitCurveType) {
        vertexWriter << If((fAttribs & PatchAttribs::kJoinControlPoint), joinControlPoint)
                     << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    AI void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
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
    // Helpers that normalize curves to a generic patch, but does no other work.
    AI void writeCubicPatch(float2 p0, float2 p1, float2 p2, float2 p3) {
        this->writePatch(p0, p1, p2, p3, kCubicCurveType);
    }
    AI void writeCubicPatch(float2 p0, float4 p1p2, float2 p3) {
        this->writeCubicPatch(p0, p1p2.lo, p1p2.hi, p3);
    }
    AI void writeQuadPatch(float2 p0, float2 p1, float2 p2) {
        this->writeCubicPatch(p0, mix(float4(p0, p2), p1.xyxy(), 2/3.f), p2);
    }
    AI void writeConicPatch(float2 p0, float2 p1, float2 p2, float w) {
        this->writePatch(p0, p1, p2, {w, SK_FloatInfinity}, kConicCurveType);
    }

    // Helpers that chop the curve type into 'numPatches' parametrically uniform curves. It is
    // assumed that 'numPatches' is calculated such that the resulting curves require the maximum
    // number of segments to draw appropriately (since the original presumably needed even more).
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches);
    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches);
    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches);

    // Returns true if curve can be written w/o needing to chop
    bool updateRequiredSegments(float n4) {
        if (n4 <= fMaxSegments_pow4) {
            fCurrMinSegments_pow4 = std::max(n4, fCurrMinSegments_pow4);
            return true;
        } else {
            fCurrMinSegments_pow4 = fMaxSegments_pow4;
            return false;
        }
    }

    // True if the patch writer only draws curves (presumably for filling), e.g. does not add a
    // wedge fan point to help fill space, or a join control point for stroking.
    bool writesCurvesOnly() const {
        return !(fAttribs & (PatchAttribs::kJoinControlPoint | PatchAttribs::kFanPoint));
    }

    const PatchAttribs fAttribs;

    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;
    float fCurrMinSegments_pow4;

    GrVertexChunkBuilder fChunker;

    SkPoint fJoinControlPointAttrib;
    SkPoint fFanPointAttrib;
    StrokeParams fStrokeParamsAttrib;
    VertexColor fColorAttrib;

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

    bool fHasJoinControlPoint = false;
};

}  // namespace skgpu

#undef AI

#endif  // tessellate_PatchWriter_DEFINED
