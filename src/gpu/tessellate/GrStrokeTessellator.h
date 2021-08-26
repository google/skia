/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellator_DEFINED
#define GrStrokeTessellator_DEFINED

#include "src/gpu/GrVx.h"
#include "src/gpu/tessellate/shaders/GrStrokeTessellationShader.h"

class GrMeshDrawTarget;
class GrOpFlushState;

// Prepares GPU data for, and then draws a stroke's tessellated geometry.
class GrStrokeTessellator {
public:
    using ShaderFlags = GrStrokeTessellationShader::ShaderFlags;

    struct PathStrokeList {
        PathStrokeList(const SkPath& path, const SkStrokeRec& stroke, const SkPMColor4f& color)
                : fPath(path), fStroke(stroke), fColor(color) {}
        SkPath fPath;
        SkStrokeRec fStroke;
        SkPMColor4f fColor;
        PathStrokeList* fNext = nullptr;
    };

    GrStrokeTessellator(const GrShaderCaps& shaderCaps, GrStrokeTessellationShader::Mode shaderMode,
                        ShaderFlags shaderFlags, int8_t maxParametricSegments_log2,
                        const SkMatrix& viewMatrix, PathStrokeList* pathStrokeList,
                        std::array<float, 2> matrixMinMaxScales, const SkRect& strokeCullBounds)
            : fShader(shaderCaps, shaderMode, shaderFlags, viewMatrix, pathStrokeList->fStroke,
                      pathStrokeList->fColor, maxParametricSegments_log2)
            , fPathStrokeList(pathStrokeList)
            , fMatrixMinMaxScales(matrixMinMaxScales)
            , fStrokeCullBounds(strokeCullBounds) {
    }

    const GrTessellationShader* shader() const { return &fShader; }

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    virtual void prepare(GrMeshDrawTarget*, int totalCombinedVerbCnt) = 0;

#if SK_GPU_V1
    // Issues draw calls for the tessellated stroke. The caller is responsible for creating and
    // binding a pipeline that uses this class's shader() before calling draw().
    virtual void draw(GrOpFlushState*) const = 0;
#endif

    virtual ~GrStrokeTessellator() {}

protected:
    GrStrokeTessellationShader fShader;
    PathStrokeList* fPathStrokeList;
    const std::array<float,2> fMatrixMinMaxScales;
    const SkRect fStrokeCullBounds;  // See SkStrokeRec::inflationRadius.
};

// These tolerances decide the number of parametric and radial segments the tessellator will
// linearize strokes into. These decisions are made in (pre-viewMatrix) local path space.
struct GrStrokeTolerances {
    // Decides the number of parametric segments the tessellator adds for each curve. (Uniform
    // steps in parametric space.) The tessellator will add enough parametric segments so that,
    // once transformed into device space, they never deviate by more than
    // 1/TessellationPathRenderer::kLinearizationPrecision pixels from the true curve.
    constexpr static float CalcParametricPrecision(float matrixMaxScale) {
        return matrixMaxScale * GrTessellationShader::kLinearizationPrecision;
    }
    // Decides the number of radial segments the tessellator adds for each curve. (Uniform steps
    // in tangent angle.) The tessellator will add this number of radial segments for each
    // radian of rotation in local path space.
    static float CalcNumRadialSegmentsPerRadian(float parametricPrecision,
                                                float strokeWidth) {
        return .5f / acosf(std::max(1 - 2 / (parametricPrecision * strokeWidth), -1.f));
    }
    template<int N> static grvx::vec<N> ApproxNumRadialSegmentsPerRadian(
            float parametricPrecision, grvx::vec<N> strokeWidths) {
        grvx::vec<N> cosTheta = skvx::max(1 - 2 / (parametricPrecision * strokeWidths), -1);
        // Subtract GRVX_APPROX_ACOS_MAX_ERROR so we never account for too few segments.
        return .5f / (grvx::approx_acos(cosTheta) - GRVX_APPROX_ACOS_MAX_ERROR);
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
    static GrStrokeTolerances Make(const float matrixMinMaxScales[2], float strokeWidth) {
        return MakeNonHairline(matrixMinMaxScales[1],
                               GetLocalStrokeWidth(matrixMinMaxScales, strokeWidth));
    }
    static GrStrokeTolerances MakeNonHairline(float matrixMaxScale, float strokeWidth) {
        SkASSERT(strokeWidth > 0);
        float parametricPrecision = CalcParametricPrecision(matrixMaxScale);
        return {parametricPrecision,
                CalcNumRadialSegmentsPerRadian(parametricPrecision, strokeWidth)};
    }
    float fParametricPrecision;
    float fNumRadialSegmentsPerRadian;
};

// Calculates and buffers up future values for "numRadialSegmentsPerRadian" using SIMD.
class alignas(sizeof(float) * 4) GrStrokeToleranceBuffer {
public:
    using PathStrokeList = GrStrokeTessellator::PathStrokeList;

    GrStrokeToleranceBuffer(float parametricPrecision)
            : fParametricPrecision(parametricPrecision) {
    }

    float fetchRadialSegmentsPerRadian(PathStrokeList* head) {
        // StrokeTessellateOp::onCombineIfPossible does not allow hairlines to become dynamic. If
        // this changes, we will need to call GrStrokeTolerances::GetLocalStrokeWidth() for each
        // stroke.
        SkASSERT(!head->fStroke.isHairlineStyle());
        if (fBufferIdx == 4) {
            // We ran out of values. Peek ahead and buffer up 4 more.
            PathStrokeList* peekAhead = head;
            int i = 0;
            do {
                fStrokeWidths[i++] = peekAhead->fStroke.getWidth();
            } while ((peekAhead = peekAhead->fNext) && i < 4);
            auto tol = GrStrokeTolerances::ApproxNumRadialSegmentsPerRadian(fParametricPrecision,
                                                                            fStrokeWidths);
            tol.store(fNumRadialSegmentsPerRadian);
            fBufferIdx = 0;
        }
        SkASSERT(0 <= fBufferIdx && fBufferIdx < 4);
        SkASSERT(fStrokeWidths[fBufferIdx] == head->fStroke.getWidth());
        return fNumRadialSegmentsPerRadian[fBufferIdx++];
    }

private:
    grvx::float4 fStrokeWidths{};  // Must be first for alignment purposes.
    float fNumRadialSegmentsPerRadian[4];
    const float fParametricPrecision;
    int fBufferIdx = 4;  // Initialize the buffer as "empty";
};

#endif
