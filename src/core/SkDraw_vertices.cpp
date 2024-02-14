/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScan.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"
#include "src/shaders/SkTriColorShader.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

class SkBlitter;

[[nodiscard]] static bool texture_to_matrix(const VertState& state, const SkPoint verts[],
                                            const SkPoint texs[], SkMatrix* matrix) {
    SkPoint src[3], dst[3];

    src[0] = verts[state.f0];
    src[1] = verts[state.f1];
    src[2] = verts[state.f2];
    dst[0] = texs[state.f0];
    dst[1] = texs[state.f1];
    dst[2] = texs[state.f2];
    return matrix->setPolyToPoly(src, dst, 3);
}

// Convert the SkColors into float colors. The conversion depends on some conditions:
// - If the pixmap has a dst colorspace, we have to be "color-correct".
//   Do we map into dst-colorspace before or after we interpolate?
// - We have to decide when to apply per-color alpha (before or after we interpolate)
//
// For now, we will take a simple approach, but recognize this is just a start:
// - convert colors into dst colorspace before interpolation (matches gradients)
// - apply per-color alpha before interpolation (matches old version of vertices)
//
static SkPMColor4f* convert_colors(const SkColor src[],
                                   int count,
                                   SkColorSpace* deviceCS,
                                   SkArenaAlloc* alloc,
                                   bool skipColorXform) {
    SkPMColor4f* dst = alloc->makeArray<SkPMColor4f>(count);

    // Passing `nullptr` for the destination CS effectively disables color conversion.
    auto dstCS = skipColorXform ? nullptr : sk_ref_sp(deviceCS);
    SkImageInfo srcInfo = SkImageInfo::Make(
            count, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkImageInfo dstInfo =
            SkImageInfo::Make(count, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType, dstCS);
    SkAssertResult(SkConvertPixels(dstInfo, dst, 0, srcInfo, src, 0));
    return dst;
}

static bool compute_is_opaque(const SkColor colors[], int count) {
    uint32_t c = ~0;
    for (int i = 0; i < count; ++i) {
        c &= colors[i];
    }
    return SkColorGetA(c) == 0xFF;
}

static void fill_triangle_2(const VertState& state, SkBlitter* blitter, const SkRasterClip& rc,
                            const SkPoint dev2[]) {
    SkPoint tmp[] = {
        dev2[state.f0], dev2[state.f1], dev2[state.f2]
    };
    SkScan::FillTriangle(tmp, rc, blitter);
}

static constexpr int kMaxClippedTrianglePointCount = 4;
static void fill_triangle_3(const VertState& state, SkBlitter* blitter, const SkRasterClip& rc,
                            const SkPoint3 dev3[]) {
    // Compute the crossing point (across zero) for the two values, expressed as a
    // normalized 0...1 value. If curr is 0, returns 0. If next is 0, returns 1.
    auto computeT = [](float curr, float next) {
        // Check that 0 is between next and curr.
        SkASSERT((next <= 0 && 0 < curr) || (curr <= 0 && 0 < next));
        float t = curr / (curr - next);
        SkASSERT(0 <= t && t <= 1);
        return t;
    };

    auto lerp = [](SkPoint3 curr, SkPoint3 next, float t) {
        return curr + t * (next - curr);
    };

    constexpr float tol = 0.05f;
    // tol is the nudge away from zero, to keep the numerics nice.
    // Think of it as our near-clipping-plane (or w-plane).
    auto clip = [&](SkPoint3 curr, SkPoint3 next) {
        // Return the point between curr and next where the fZ value crosses tol.
        // To be (really) perspective correct, we should be computing based on 1/Z, not Z.
        // For now, this is close enough (and faster).
        return lerp(curr, next, computeT(curr.fZ - tol, next.fZ - tol));
    };

    // Clip a triangle (based on its homogeneous W values), and return the projected polygon.
    // Since we only clip against one "edge"/plane, the max number of points in the clipped
    // polygon is 4.
    auto clipTriangle = [&](SkPoint dst[], const int idx[3], const SkPoint3 pts[]) -> int {
        SkPoint3 outPoints[kMaxClippedTrianglePointCount];
        SkPoint3* outP = outPoints;

        for (int i = 0; i < 3; ++i) {
            int curr = idx[i];
            int next = idx[(i + 1) % 3];
            if (pts[curr].fZ > tol) {
                *outP++ = pts[curr];
                if (pts[next].fZ <= tol) { // curr is IN, next is OUT
                    *outP++ = clip(pts[curr], pts[next]);
                }
            } else {
                if (pts[next].fZ > tol) { // curr is OUT, next is IN
                    *outP++ = clip(pts[curr], pts[next]);
                }
            }
        }

        const int count = SkTo<int>(outP - outPoints);
        SkASSERT(count == 0 || count == 3 || count == 4);
        for (int i = 0; i < count; ++i) {
            float scale = sk_ieee_float_divide(1.0f, outPoints[i].fZ);
            dst[i].set(outPoints[i].fX * scale, outPoints[i].fY * scale);
        }
        return count;
    };

    SkPoint tmp[kMaxClippedTrianglePointCount];
    int idx[] = { state.f0, state.f1, state.f2 };
    if (int n = clipTriangle(tmp, idx, dev3)) {
        // TODO: SkScan::FillConvexPoly(tmp, n, ...);
        SkASSERT(n == 3 || n == 4);
        SkScan::FillTriangle(tmp, rc, blitter);
        if (n == 4) {
            tmp[1] = tmp[2];
            tmp[2] = tmp[3];
            SkScan::FillTriangle(tmp, rc, blitter);
        }
    }
}

static void fill_triangle(const VertState& state, SkBlitter* blitter, const SkRasterClip& rc,
                          const SkPoint dev2[], const SkPoint3 dev3[]) {
    if (dev3) {
        fill_triangle_3(state, blitter, rc, dev3);
    } else {
        fill_triangle_2(state, blitter, rc, dev2);
    }
}

void SkDraw::drawFixedVertices(const SkVertices* vertices,
                               sk_sp<SkBlender> blender,
                               const SkPaint& paint,
                               const SkMatrix& ctmInverse,
                               const SkPoint* dev2,
                               const SkPoint3* dev3,
                               SkArenaAlloc* outerAlloc,
                               bool skipColorXform) const {
    SkVerticesPriv info(vertices->priv());

    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const SkPoint* texCoords = info.texCoords();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();

    SkShader* paintShader = paint.getShader();

    if (paintShader) {
        if (!texCoords) {
            texCoords = positions;
        }
    } else {
        texCoords = nullptr;
    }

    bool blenderIsDst = false;
    // We can simplify things for certain blend modes. This is for speed, and SkShader_Blend
    // itself insists we don't pass kSrc or kDst to it.
    if (std::optional<SkBlendMode> bm = as_BB(blender)->asBlendMode(); bm.has_value() && colors) {
        switch (*bm) {
            case SkBlendMode::kSrc:
                colors = nullptr;
                break;
            case SkBlendMode::kDst:
                blenderIsDst = true;
                texCoords = nullptr;
                paintShader = nullptr;
                break;
            default: break;
        }
    }

    // There is a paintShader iff there is texCoords.
    SkASSERT((texCoords != nullptr) == (paintShader != nullptr));

    // Explicit texture coords can't contain perspective - only the CTM can.
    const bool usePerspective = fCTM->hasPerspective();

    SkTriColorShader* triColorShader = nullptr;
    SkPMColor4f* dstColors = nullptr;
    if (colors) {
        dstColors =
                convert_colors(colors, vertexCount, fDst.colorSpace(), outerAlloc, skipColorXform);
        triColorShader = outerAlloc->make<SkTriColorShader>(compute_is_opaque(colors, vertexCount),
                                                            usePerspective);
    }

    // Combines per-vertex colors with 'shader' using 'blender'.
    auto applyShaderColorBlend = [&](SkShader* shader) -> sk_sp<SkShader> {
        if (!colors) {
            return sk_ref_sp(shader);
        }
        if (blenderIsDst) {
            return sk_ref_sp(triColorShader);
        }
        sk_sp<SkShader> shaderWithWhichToBlend;
        if (!shader) {
            // When there is no shader then the blender applies to the vertex colors and opaque
            // paint color.
            shaderWithWhichToBlend = SkShaders::Color(paint.getColor4f().makeOpaque(), nullptr);
        } else {
            shaderWithWhichToBlend = sk_ref_sp(shader);
        }
        return SkShaders::Blend(blender,
                                sk_ref_sp(triColorShader),
                                std::move(shaderWithWhichToBlend));
    };

    // If there are separate texture coords then we need to insert a transform shader to update
    // a matrix derived from each triangle's coords. In that case we will fold the CTM into
    // each update and use an identity matrix.
    SkTransformShader* transformShader = nullptr;
    const SkMatrix* ctm = fCTM;
    if (texCoords && texCoords != positions) {
        paintShader = transformShader = outerAlloc->make<SkTransformShader>(*as_SB(paintShader),
                                                                            usePerspective);
        ctm = &SkMatrix::I();
    }
    sk_sp<SkShader> blenderShader = applyShaderColorBlend(paintShader);

    SkPaint finalPaint{paint};
    finalPaint.setShader(std::move(blenderShader));

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    SkSurfaceProps props = SkSurfacePropsCopyOrDefault(fProps);

    auto blitter = SkCreateRasterPipelineBlitter(fDst,
                                                 finalPaint,
                                                 *ctm,
                                                 outerAlloc,
                                                 fRC->clipShader(),
                                                 props);
    if (!blitter) {
        return;
    }
    while (vertProc(&state)) {
        if (triColorShader && !triColorShader->update(ctmInverse, positions, dstColors,
                                                      state.f0, state.f1, state.f2)) {
            continue;
        }

        SkMatrix localM;
        if (!transformShader || (texture_to_matrix(state, positions, texCoords, &localM) &&
                                 transformShader->update(SkMatrix::Concat(localM, ctmInverse)))) {
            fill_triangle(state, blitter, *fRC, dev2, dev3);
        }
    }
}

void SkDraw::drawVertices(const SkVertices* vertices,
                          sk_sp<SkBlender> blender,
                          const SkPaint& paint,
                          bool skipColorXform) const {
    SkVerticesPriv info(vertices->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();

    // abort early if there is nothing to draw
    if (vertexCount < 3 || (indexCount > 0 && indexCount < 3) || fRC->isEmpty()) {
        return;
    }
    SkMatrix ctmInv;
    if (!fCTM->invert(&ctmInv)) {
        return;
    }

    constexpr size_t kDefVertexCount = 16;
    constexpr size_t kOuterSize = sizeof(SkTriColorShader) +
                                  (2 * sizeof(SkPoint) + sizeof(SkColor4f)) * kDefVertexCount;
    SkSTArenaAlloc<kOuterSize> outerAlloc;

    SkPoint*  dev2 = nullptr;
    SkPoint3* dev3 = nullptr;

    if (fCTM->hasPerspective()) {
        dev3 = outerAlloc.makeArray<SkPoint3>(vertexCount);
        fCTM->mapHomogeneousPoints(dev3, info.positions(), vertexCount);
        // similar to the bounds check for 2d points (below)
        if (!SkScalarsAreFinite((const SkScalar*)dev3, vertexCount * 3)) {
            return;
        }
    } else {
        dev2 = outerAlloc.makeArray<SkPoint>(vertexCount);
        fCTM->mapPoints(dev2, info.positions(), vertexCount);

        SkRect bounds;
        // this also sets bounds to empty if we see a non-finite value
        bounds.setBounds(dev2, vertexCount);
        if (bounds.isEmpty()) {
            return;
        }
    }

    this->drawFixedVertices(
            vertices, std::move(blender), paint, ctmInv, dev2, dev3, &outerAlloc, skipColorXform);
}
