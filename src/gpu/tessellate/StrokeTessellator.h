/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_StrokeTessellator_DEFINED
#define tessellate_StrokeTessellator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkColorData.h"
#include "src/gpu/tessellate/Tessellation.h"

class GrMeshDrawTarget;
class GrOpFlushState;

namespace skgpu {

// Prepares GPU data for, and then draws a stroke's tessellated geometry.
class StrokeTessellator {
public:
    struct PathStrokeList {
        PathStrokeList(const SkPath& path, const SkStrokeRec& stroke, const SkPMColor4f& color)
                : fPath(path), fStroke(stroke), fColor(color) {}
        SkPath fPath;
        SkStrokeRec fStroke;
        SkPMColor4f fColor;
        PathStrokeList* fNext = nullptr;
    };

    StrokeTessellator(PatchAttribs attribs) : fAttribs(attribs) {}

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    //
    // Returns the fixed number of edges the tessellator will draw per patch, if using fixed-count
    // rendering, otherwise 0.
    virtual int prepare(GrMeshDrawTarget*,
                        const SkMatrix& shaderMatrix,
                        std::array<float,2> matrixMinMaxScales,
                        PathStrokeList*,
                        int totalCombinedVerbCnt) = 0;

#if SK_GPU_V1
    // Issues draw calls for the tessellated stroke. The caller is responsible for creating and
    // binding a pipeline that uses this class's shader() before calling draw().
    virtual void draw(GrOpFlushState*) const = 0;
#endif

    virtual ~StrokeTessellator() {}

protected:
    PatchAttribs fAttribs;
};

// These tolerances decide the number of parametric and radial segments the tessellator will
// linearize strokes into. These decisions are made in (pre-viewMatrix) local path space.
struct StrokeTolerances {
    // Decides the number of parametric segments the tessellator adds for each curve. (Uniform
    // steps in parametric space.) The tessellator will add enough parametric segments so that,
    // once transformed into device space, they never deviate by more than
    // 1/kTessellationPrecision pixels from the true curve.
    constexpr static float CalcParametricPrecision(float matrixMaxScale) {
        return matrixMaxScale * kTessellationPrecision;
    }
    // Decides the number of radial segments the tessellator adds for each curve. (Uniform steps
    // in tangent angle.) The tessellator will add this number of radial segments for each
    // radian of rotation in local path space.
    static float CalcNumRadialSegmentsPerRadian(float parametricPrecision,
                                                float strokeWidth) {
        return .5f / acosf(std::max(1 - 2 / (parametricPrecision * strokeWidth), -1.f));
    }
    template<int N> static vec<N> ApproxNumRadialSegmentsPerRadian(float parametricPrecision,
                                                                   vec<N> strokeWidths) {
        vec<N> cosTheta = skvx::max(1 - 2 / (parametricPrecision * strokeWidths), -1);
        // Subtract SKVX_APPROX_ACOS_MAX_ERROR so we never account for too few segments.
        return .5f / (skvx::approx_acos(cosTheta) - SKVX_APPROX_ACOS_MAX_ERROR);
    }
    // Returns the equivalent stroke width in (pre-viewMatrix) local path space that the
    // tessellator will use when rendering this stroke. This only differs from the actual stroke
    // width for hairlines.
    static float GetLocalStrokeWidth(const float matrixMinMaxScales[2], float strokeWidth) {
        SkASSERT(strokeWidth >= 0);
        float localStrokeWidth = strokeWidth;
        if (localStrokeWidth == 0) {  // Is the stroke a hairline?
            float matrixMinScale = matrixMinMaxScales[0];
            float matrixMaxScale = matrixMinMaxScales[1];
            // If the stroke is hairline then the tessellator will operate in post-transform
            // space instead. But for the sake of CPU methods that need to conservatively
            // approximate the number of segments to emit, we use
            // localStrokeWidth ~= 1/matrixMinScale.
            float approxScale = matrixMinScale;
            // If the matrix has strong skew, don't let the scale shoot off to infinity. (This
            // does not affect the tessellator; only the CPU methods that approximate the number
            // of segments to emit.)
            approxScale = std::max(matrixMinScale, matrixMaxScale * .25f);
            localStrokeWidth = 1/approxScale;
            if (localStrokeWidth == 0) {
                // We just can't accidentally return zero from this method because zero means
                // "hairline". Otherwise return whatever we calculated above.
                localStrokeWidth = SK_ScalarNearlyZero;
            }
        }
        return localStrokeWidth;
    }
    static StrokeTolerances Make(const float matrixMinMaxScales[2], float strokeWidth) {
        return MakeNonHairline(matrixMinMaxScales[1],
                               GetLocalStrokeWidth(matrixMinMaxScales, strokeWidth));
    }
    static StrokeTolerances MakeNonHairline(float matrixMaxScale, float strokeWidth) {
        SkASSERT(strokeWidth > 0);
        float parametricPrecision = CalcParametricPrecision(matrixMaxScale);
        return {parametricPrecision,
                CalcNumRadialSegmentsPerRadian(parametricPrecision, strokeWidth)};
    }
    float fParametricPrecision;
    float fNumRadialSegmentsPerRadian;
};

// Calculates and buffers up future values for "numRadialSegmentsPerRadian" using SIMD.
class alignas(sizeof(float) * 4) StrokeToleranceBuffer {
public:
    using PathStrokeList = StrokeTessellator::PathStrokeList;

    StrokeToleranceBuffer(float parametricPrecision) : fParametricPrecision(parametricPrecision) {}

    float fetchRadialSegmentsPerRadian(PathStrokeList* head) {
        // StrokeTessellateOp::onCombineIfPossible does not allow hairlines to become dynamic. If
        // this changes, we will need to call StrokeTolerances::GetLocalStrokeWidth() for each
        // stroke.
        SkASSERT(!head->fStroke.isHairlineStyle());
        if (fBufferIdx == 4) {
            // We ran out of values. Peek ahead and buffer up 4 more.
            PathStrokeList* peekAhead = head;
            int i = 0;
            do {
                fStrokeWidths[i++] = peekAhead->fStroke.getWidth();
            } while ((peekAhead = peekAhead->fNext) && i < 4);
            auto tol = StrokeTolerances::ApproxNumRadialSegmentsPerRadian(fParametricPrecision,
                                                                          fStrokeWidths);
            tol.store(fNumRadialSegmentsPerRadian);
            fBufferIdx = 0;
        }
        SkASSERT(0 <= fBufferIdx && fBufferIdx < 4);
        SkASSERT(fStrokeWidths[fBufferIdx] == head->fStroke.getWidth());
        return fNumRadialSegmentsPerRadian[fBufferIdx++];
    }

private:
    float4 fStrokeWidths{};  // Must be first for alignment purposes.
    float fNumRadialSegmentsPerRadian[4];
    const float fParametricPrecision;
    int fBufferIdx = 4;  // Initialize the buffer as "empty";
};

}  // namespace skgpu

#endif  // tessellate_StrokeTessellator_DEFINED
