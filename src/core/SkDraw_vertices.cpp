/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkAutoBlitterChoose.h"
#include "SkComposeShader.h"
#include "SkDraw.h"
#include "SkNx.h"
#include "SkPM4fPriv.h"
#include "SkRasterClip.h"
#include "SkScan.h"
#include "SkShaderBase.h"
#include "SkString.h"
#include "SkVertState.h"

#include "SkArenaAlloc.h"
#include "SkCoreBlitters.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpace_Base.h"

struct Matrix43 {
    float fMat[12];    // column major

    Sk4f map(float x, float y) const {
        return Sk4f::Load(&fMat[0]) * x + Sk4f::Load(&fMat[4]) * y + Sk4f::Load(&fMat[8]);
    }

    void setConcat(const Matrix43& a, const SkMatrix& b) {
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

static SkScan::HairRCProc ChooseHairProc(bool doAntiAlias) {
    return doAntiAlias ? SkScan::AntiHairLine : SkScan::HairLine;
}

static bool texture_to_matrix(const VertState& state, const SkPoint verts[],
                              const SkPoint texs[], SkMatrix* matrix) {
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
    SkTriColorShader(bool isOpaque) : fIsOpaque(isOpaque) {}

    Matrix43* getMatrix43() { return &fM43; }

    bool isOpaque() const override { return fIsOpaque; }

    SK_TO_STRING_OVERRIDE()

    // For serialization.  This will never be called.
    Factory getFactory() const override { sk_throw(); return nullptr; }

protected:
    Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const override {
        return nullptr;
    }
    bool onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS, SkArenaAlloc* alloc,
                        const SkMatrix&, const SkPaint&, const SkMatrix*) const override {
        pipeline->append(SkRasterPipeline::seed_shader);
        pipeline->append(SkRasterPipeline::matrix_4x3, &fM43);
        // In theory we should never need to clamp. However, either due to imprecision in our
        // matrix43, or the scan converter passing us pixel centers that in fact are not within
        // the triangle, we do see occasional (slightly) out-of-range values, so we add these
        // clamp stages. It would be nice to find a way to detect when these are not needed.
        pipeline->append(SkRasterPipeline::clamp_0);
        pipeline->append(SkRasterPipeline::clamp_a);
        return true;
    }

private:
    Matrix43 fM43;
    const bool fIsOpaque;

    typedef SkShaderBase INHERITED;
};

#ifndef SK_IGNORE_TO_STRING
void SkTriColorShader::toString(SkString* str) const {
    str->append("SkTriColorShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

static bool update_tricolor_matrix(const SkMatrix& ctmInv,
                                   const SkPoint pts[], const SkPM4f colors[],
                                   int index0, int index1, int index2, Matrix43* result) {
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

    SkMatrix dstToUnit;
    dstToUnit.setConcat(im, ctmInv);

    Sk4f c0 = colors[index0].to4f(),
         c1 = colors[index1].to4f(),
         c2 = colors[index2].to4f();

    Matrix43 colorm;
    (c1 - c0).store(&colorm.fMat[0]);
    (c2 - c0).store(&colorm.fMat[4]);
    c0.store(&colorm.fMat[8]);
    result->setConcat(colorm, dstToUnit);
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
static SkPM4f* convert_colors(const SkColor src[], int count, SkColorSpace* deviceCS,
                              SkArenaAlloc* alloc) {
    SkPM4f* dst = alloc->makeArray<SkPM4f>(count);
    if (!deviceCS) {
        for (int i = 0; i < count; ++i) {
            dst[i] = SkPM4f_from_SkColor(src[i], nullptr);
        }
    } else {
        auto srcCS = SkColorSpace::MakeSRGB();
        auto dstCS = as_CSB(deviceCS)->makeLinearGamma();
        SkColorSpaceXform::Apply(dstCS.get(), SkColorSpaceXform::kRGBA_F32_ColorFormat, dst,
                                 srcCS.get(), SkColorSpaceXform::kBGRA_8888_ColorFormat, src,
                                 count, SkColorSpaceXform::kPremul_AlphaOp);
    }
    return dst;
}

static bool compute_is_opaque(const SkColor colors[], int count) {
    uint32_t c = ~0;
    for (int i = 0; i < count; ++i) {
        c &= colors[i];
    }
    return SkColorGetA(c) == 0xFF;
}

void SkDraw::drawVertices(SkVertices::VertexMode vmode, int count,
                          const SkPoint vertices[], const SkPoint textures[],
                          const SkColor colors[], SkBlendMode bmode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) const {
    SkASSERT(0 == count || vertices);

    // abort early if there is nothing to draw
    if (count < 3 || (indices && indexCount < 3) || fRC->isEmpty()) {
        return;
    }
    SkMatrix ctmInv;
    if (!fMatrix->invert(&ctmInv)) {
        return;
    }

    // make textures and shader mutually consistent
    SkShader* shader = paint.getShader();
    if (!(shader && textures)) {
        shader = nullptr;
        textures = nullptr;
    }

    // We can simplify things for certain blendmodes. This is for speed, and SkComposeShader
    // itself insists we don't pass kSrc or kDst to it.
    //
    if (colors && textures) {
        switch (bmode) {
            case SkBlendMode::kSrc:
                colors = nullptr;
                break;
            case SkBlendMode::kDst:
                textures = nullptr;
                break;
            default: break;
        }
    }

    // we don't use the shader if there are no textures
    if (!textures) {
        shader = nullptr;
    }

    constexpr size_t defCount = 16;
    constexpr size_t outerSize = sizeof(SkTriColorShader) +
                                 sizeof(SkComposeShader) +
                                 (sizeof(SkPoint) + sizeof(SkPM4f)) * defCount;
    SkSTArenaAlloc<outerSize> outerAlloc;

    SkPoint* devVerts = outerAlloc.makeArray<SkPoint>(count);
    fMatrix->mapPoints(devVerts, vertices, count);

    VertState       state(count, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    if (colors || textures) {
        SkPM4f*     dstColors = nullptr;
        Matrix43*   matrix43 = nullptr;

        if (colors) {
            dstColors = convert_colors(colors, count, fDst.colorSpace(), &outerAlloc);

            SkTriColorShader* triShader = outerAlloc.make<SkTriColorShader>(
                                                                compute_is_opaque(colors, count));
            matrix43 = triShader->getMatrix43();
            if (shader) {
                shader = outerAlloc.make<SkComposeShader>(sk_ref_sp(triShader), sk_ref_sp(shader),
                                                          bmode, 1);
            } else {
                shader = triShader;
            }
        }

        SkPaint p(paint);
        p.setShader(sk_ref_sp(shader));

        if (!textures) {    // only tricolor shader
            SkASSERT(matrix43);
            auto blitter = SkCreateRasterPipelineBlitter(fDst, p, *fMatrix, &outerAlloc);
            while (vertProc(&state)) {
                if (!update_tricolor_matrix(ctmInv, vertices, dstColors,
                                            state.f0, state.f1, state.f2,
                                            matrix43)) {
                    continue;
                }

                SkPoint tmp[] = {
                    devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
                };
                SkScan::FillTriangle(tmp, *fRC, blitter);
            }
        } else {
            while (vertProc(&state)) {
                SkSTArenaAlloc<2048> innerAlloc;

                const SkMatrix* ctm = fMatrix;
                SkMatrix tmpCtm;
                if (textures) {
                    SkMatrix localM;
                    texture_to_matrix(state, vertices, textures, &localM);
                    tmpCtm = SkMatrix::Concat(*fMatrix, localM);
                    ctm = &tmpCtm;
                }

                if (matrix43 && !update_tricolor_matrix(ctmInv, vertices, dstColors,
                                                        state.f0, state.f1, state.f2,
                                                        matrix43)) {
                    continue;
                }

                SkPoint tmp[] = {
                    devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
                };
                auto blitter = SkCreateRasterPipelineBlitter(fDst, p, *ctm, &innerAlloc);
                SkScan::FillTriangle(tmp, *fRC, blitter);
            }
        }
    } else {
        // no colors[] and no texture, stroke hairlines with paint's color.
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        SkAutoBlitterChoose blitter(fDst, *fMatrix, p);
        // Abort early if we failed to create a shader context.
        if (blitter->isNullBlitter()) {
            return;
        }
        SkScan::HairRCProc hairProc = ChooseHairProc(paint.isAntiAlias());
        const SkRasterClip& clip = *fRC;
        while (vertProc(&state)) {
            SkPoint array[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2], devVerts[state.f0]
            };
            hairProc(array, 4, clip, blitter.get());
        }
    }
}
