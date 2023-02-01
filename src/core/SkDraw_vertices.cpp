/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkString.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkVx.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkScan.h"
#include "src/core/SkVM.h"
#include "src/core/SkVMBlitter.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"

struct Matrix43 {
    float fMat[12];    // column major

    skvx::float4 map(float x, float y) const {
        return skvx::float4::Load(&fMat[0]) * x +
               skvx::float4::Load(&fMat[4]) * y +
               skvx::float4::Load(&fMat[8]);
    }

    // Pass a by value, so we don't have to worry about aliasing with this
    void setConcat(const Matrix43 a, const SkMatrix& b) {
        SkASSERT(!b.hasPerspective());

        fMat[ 0] = a.dot(0, b.getScaleX(), b.getSkewY());
        fMat[ 1] = a.dot(1, b.getScaleX(), b.getSkewY());
        fMat[ 2] = a.dot(2, b.getScaleX(), b.getSkewY());
        fMat[ 3] = a.dot(3, b.getScaleX(), b.getSkewY());

        fMat[ 4] = a.dot(0, b.getSkewX(), b.getScaleY());
        fMat[ 5] = a.dot(1, b.getSkewX(), b.getScaleY());
        fMat[ 6] = a.dot(2, b.getSkewX(), b.getScaleY());
        fMat[ 7] = a.dot(3, b.getSkewX(), b.getScaleY());

        fMat[ 8] = a.dot(0, b.getTranslateX(), b.getTranslateY()) + a.fMat[ 8];
        fMat[ 9] = a.dot(1, b.getTranslateX(), b.getTranslateY()) + a.fMat[ 9];
        fMat[10] = a.dot(2, b.getTranslateX(), b.getTranslateY()) + a.fMat[10];
        fMat[11] = a.dot(3, b.getTranslateX(), b.getTranslateY()) + a.fMat[11];
    }

private:
    float dot(int index, float x, float y) const {
        return fMat[index + 0] * x + fMat[index + 4] * y;
    }
};

static bool SK_WARN_UNUSED_RESULT
texture_to_matrix(const VertState& state, const SkPoint verts[], const SkPoint texs[],
                  SkMatrix* matrix) {
    SkPoint src[3], dst[3];

    src[0] = texs[state.f0];
    src[1] = texs[state.f1];
    src[2] = texs[state.f2];
    dst[0] = verts[state.f0];
    dst[1] = verts[state.f1];
    dst[2] = verts[state.f2];
    return matrix->setPolyToPoly(src, dst, 3);
}

class SkTriColorShader : public SkShaderBase {
public:
    SkTriColorShader(bool isOpaque, bool usePersp) : fIsOpaque(isOpaque), fUsePersp(usePersp) {}

    // This gets called for each triangle, without re-calling appendStages.
    bool update(const SkMatrix& ctmInv, const SkPoint pts[], const SkPMColor4f colors[],
                int index0, int index1, int index2);

protected:
    bool appendStages(const SkStageRec& rec, const MatrixRec&) const override {
        rec.fPipeline->append(SkRasterPipelineOp::seed_shader);
        if (fUsePersp) {
            rec.fPipeline->append(SkRasterPipelineOp::matrix_perspective, &fM33);
        }
        rec.fPipeline->append(SkRasterPipelineOp::matrix_4x3, &fM43);
        return true;
    }

    skvm::Color program(skvm::Builder*,
                        skvm::Coord,
                        skvm::Coord,
                        skvm::Color,
                        const MatrixRec&,
                        const SkColorInfo&,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;

private:
    bool isOpaque() const override { return fIsOpaque; }
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    // If fUsePersp, we need both of these matrices,
    // otherwise we can combine them, and only use fM43

    Matrix43 fM43;
    SkMatrix fM33;
    const bool fIsOpaque;
    const bool fUsePersp;   // controls our stages, and what we do in update()
    mutable skvm::Uniform fColorMatrix;
    mutable skvm::Uniform fCoordMatrix;

    using INHERITED = SkShaderBase;
};

skvm::Color SkTriColorShader::program(skvm::Builder* b,
                                      skvm::Coord device,
                                      skvm::Coord local,
                                      skvm::Color,
                                      const MatrixRec&,
                                      const SkColorInfo&,
                                      skvm::Uniforms* uniforms,
                                      SkArenaAlloc* alloc) const {
    fColorMatrix = uniforms->pushPtr(&fM43);

    skvm::F32 x = local.x,
              y = local.y;

    if (fUsePersp) {
        fCoordMatrix = uniforms->pushPtr(&fM33);
        auto dot = [&, x, y](int row) {
            return b->mad(x, b->arrayF(fCoordMatrix, row),
                             b->mad(y, b->arrayF(fCoordMatrix, row + 3),
                                       b->arrayF(fCoordMatrix, row + 6)));
        };

        x = dot(0);
        y = dot(1);
        x = x * (1.0f / dot(2));
        y = y * (1.0f / dot(2));
    }

    auto colorDot = [&, x, y](int row) {
        return b->mad(x, b->arrayF(fColorMatrix, row),
                         b->mad(y, b->arrayF(fColorMatrix, row + 4),
                                   b->arrayF(fColorMatrix, row + 8)));
    };

    skvm::Color color;
    color.r = colorDot(0);
    color.g = colorDot(1);
    color.b = colorDot(2);
    color.a = colorDot(3);
    return color;
}

bool SkTriColorShader::update(const SkMatrix& ctmInv, const SkPoint pts[],
                              const SkPMColor4f colors[], int index0, int index1, int index2) {
    SkMatrix m, im;
    m.reset();
    m.set(0, pts[index1].fX - pts[index0].fX);
    m.set(1, pts[index2].fX - pts[index0].fX);
    m.set(2, pts[index0].fX);
    m.set(3, pts[index1].fY - pts[index0].fY);
    m.set(4, pts[index2].fY - pts[index0].fY);
    m.set(5, pts[index0].fY);
    if (!m.invert(&im)) {
        return false;
    }

    fM33.setConcat(im, ctmInv);

    auto c0 = skvx::float4::Load(colors[index0].vec()),
         c1 = skvx::float4::Load(colors[index1].vec()),
         c2 = skvx::float4::Load(colors[index2].vec());

    (c1 - c0).store(&fM43.fMat[0]);
    (c2 - c0).store(&fM43.fMat[4]);
    c0.store(&fM43.fMat[8]);

    if (!fUsePersp) {
        fM43.setConcat(fM43, fM33);
    }
    return true;
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

extern bool gUseSkVMBlitter;

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

    SkMatrix ctm = fMatrixProvider->localToDevice();
    // Explicit texture coords can't contain perspective - only the CTM can.
    const bool usePerspective = ctm.hasPerspective();

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
    // each update and use an identity matrix provider.
    SkTransformShader* transformShader = nullptr;
    const SkMatrixProvider* matrixProvider = fMatrixProvider;
    SkTLazy<SkMatrixProvider> identityProvider;
    if (texCoords && texCoords != positions) {
        paintShader = transformShader = outerAlloc->make<SkTransformShader>(*as_SB(paintShader),
                                                                            usePerspective);
        matrixProvider = identityProvider.init(SkMatrix::I());
    }
    sk_sp<SkShader> blenderShader = applyShaderColorBlend(paintShader);

    SkPaint finalPaint{paint};
    finalPaint.setShader(std::move(blenderShader));

    auto rpblit = [&]() {
        VertState state(vertexCount, indices, indexCount);
        VertState::Proc vertProc = state.chooseProc(info.mode());
        SkSurfaceProps props = SkSurfacePropsCopyOrDefault(fProps);

        auto blitter = SkCreateRasterPipelineBlitter(fDst,
                                                     finalPaint,
                                                     matrixProvider->localToDevice(),
                                                     outerAlloc,
                                                     fRC->clipShader(),
                                                     props);
        if (!blitter) {
            return false;
        }
        while (vertProc(&state)) {
            if (triColorShader && !triColorShader->update(ctmInverse, positions, dstColors,
                                                          state.f0, state.f1, state.f2)) {
                continue;
            }

            SkMatrix localM;
            if (!transformShader || (texture_to_matrix(state, positions, texCoords, &localM) &&
                                     transformShader->update(SkMatrix::Concat(ctm, localM)))) {
                fill_triangle(state, blitter, *fRC, dev2, dev3);
            }
        }
        return true;
    };

    if (gUseSkVMBlitter || !rpblit()) {
        VertState state(vertexCount, indices, indexCount);
        VertState::Proc vertProc = state.chooseProc(info.mode());

        auto blitter = SkVMBlitter::Make(fDst,
                                         finalPaint,
                                         matrixProvider->localToDevice(),
                                         outerAlloc,
                                         this->fRC->clipShader());
        if (!blitter) {
            return;
        }
        while (vertProc(&state)) {
            SkMatrix localM;
            if (transformShader && !(texture_to_matrix(state, positions, texCoords, &localM) &&
                                     transformShader->update(SkMatrix::Concat(ctm, localM)))) {
                continue;
            }

            if (triColorShader && !triColorShader->update(ctmInverse, positions, dstColors,state.f0,
                                                          state.f1, state.f2)) {
                continue;
            }

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
    SkMatrix ctm = fMatrixProvider->localToDevice();
    SkMatrix ctmInv;
    if (!ctm.invert(&ctmInv)) {
        return;
    }

    constexpr size_t kDefVertexCount = 16;
    constexpr size_t kOuterSize = sizeof(SkTriColorShader) +
                                  (2 * sizeof(SkPoint) + sizeof(SkColor4f)) * kDefVertexCount;
    SkSTArenaAlloc<kOuterSize> outerAlloc;

    SkPoint*  dev2 = nullptr;
    SkPoint3* dev3 = nullptr;

    if (ctm.hasPerspective()) {
        dev3 = outerAlloc.makeArray<SkPoint3>(vertexCount);
        ctm.mapHomogeneousPoints(dev3, info.positions(), vertexCount);
        // similar to the bounds check for 2d points (below)
        if (!SkScalarsAreFinite((const SkScalar*)dev3, vertexCount * 3)) {
            return;
        }
    } else {
        dev2 = outerAlloc.makeArray<SkPoint>(vertexCount);
        ctm.mapPoints(dev2, info.positions(), vertexCount);

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
