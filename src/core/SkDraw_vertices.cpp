/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkNx.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkScan.h"
#include "src/core/SkVertState.h"
#include "src/shaders/SkComposeShader.h"
#include "src/shaders/SkShaderBase.h"

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
    SkTriColorShader(bool isOpaque) : fIsOpaque(isOpaque) {}

    bool update(const SkMatrix& ctmInv, const SkPoint pts[], const SkPMColor4f colors[],
                int index0, int index1, int index2);

protected:
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const override {
        return nullptr;
    }
#endif
    bool onAppendStages(const SkStageRec& rec) const override {
        rec.fPipeline->append(SkRasterPipeline::seed_shader);
        rec.fPipeline->append(SkRasterPipeline::matrix_4x3, &fM43);
        return true;
    }

private:
    bool isOpaque() const override { return fIsOpaque; }
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    Matrix43 fM43;  // we overwrite this for each triangle
    const bool fIsOpaque;

    typedef SkShaderBase INHERITED;
};

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

    SkMatrix dstToUnit;
    dstToUnit.setConcat(im, ctmInv);

    Sk4f c0 = Sk4f::Load(colors[index0].vec()),
         c1 = Sk4f::Load(colors[index1].vec()),
         c2 = Sk4f::Load(colors[index2].vec());

    Matrix43 colorm;
    (c1 - c0).store(&colorm.fMat[0]);
    (c2 - c0).store(&colorm.fMat[4]);
    c0.store(&colorm.fMat[8]);
    fM43.setConcat(colorm, dstToUnit);
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
static SkPMColor4f* convert_colors(const SkColor src[], int count, SkColorSpace* deviceCS,
                                   SkArenaAlloc* alloc) {
    SkPMColor4f* dst = alloc->makeArray<SkPMColor4f>(count);
    SkImageInfo srcInfo = SkImageInfo::Make(count, 1, kBGRA_8888_SkColorType,
                                            kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkImageInfo dstInfo = SkImageInfo::Make(count, 1, kRGBA_F32_SkColorType,
                                            kPremul_SkAlphaType, sk_ref_sp(deviceCS));
    SkConvertPixels(dstInfo, dst, 0, srcInfo, src, 0);
    return dst;
}

static bool compute_is_opaque(const SkColor colors[], int count) {
    uint32_t c = ~0;
    for (int i = 0; i < count; ++i) {
        c &= colors[i];
    }
    return SkColorGetA(c) == 0xFF;
}

void SkDraw::drawVertices(SkVertices::VertexMode vmode, int vertexCount,
                          const SkPoint vertices[], const SkPoint textures[],
                          const SkColor colors[], const SkVertices::BoneIndices boneIndices[],
                          const SkVertices::BoneWeights boneWeights[], SkBlendMode bmode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint, const SkVertices::Bone bones[],
                          int boneCount) const {
    SkASSERT(0 == vertexCount || vertices);

    // abort early if there is nothing to draw
    if (vertexCount < 3 || (indices && indexCount < 3) || fRC->isEmpty()) {
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

    constexpr size_t kDefVertexCount = 16;
    constexpr size_t kOuterSize = sizeof(SkTriColorShader) +
                                 sizeof(SkShader_Blend) +
                                 (2 * sizeof(SkPoint) + sizeof(SkColor4f)) * kDefVertexCount;
    SkSTArenaAlloc<kOuterSize> outerAlloc;

    // deform vertices using the skeleton if it is passed in
    if (bones && boneCount) {
        // allocate space for the deformed vertices
        SkPoint* deformed = outerAlloc.makeArray<SkPoint>(vertexCount);

        // deform the vertices
        if (boneIndices && boneWeights) {
            for (int i = 0; i < vertexCount; i ++) {
                const SkVertices::BoneIndices& indices = boneIndices[i];
                const SkVertices::BoneWeights& weights = boneWeights[i];

                // apply the world transform
                SkPoint worldPoint = bones[0].mapPoint(vertices[i]);

                // apply bone deformations
                deformed[i] = SkPoint::Make(0.0f, 0.0f);
                for (uint32_t j = 0; j < 4; j ++) {
                    // get the attachment data
                    uint32_t index = indices[j];
                    float weight = weights[j];

                    // skip the bone if there is no weight
                    if (weight == 0.0f) {
                        continue;
                    }
                    SkASSERT(index != 0);

                    // deformed += M * v * w
                    deformed[i] += bones[index].mapPoint(worldPoint) * weight;
                }
            }
        } else {
            // no bones, so only apply world transform
            SkMatrix worldTransform = SkMatrix::I();
            worldTransform.setAffine(bones[0].values);
            worldTransform.mapPoints(deformed, vertices, vertexCount);
        }

        // change the vertices to point to deformed
        vertices = deformed;
    }

    SkPoint* devVerts = outerAlloc.makeArray<SkPoint>(vertexCount);
    fMatrix->mapPoints(devVerts, vertices, vertexCount);

    {
        SkRect bounds;
        // this also sets bounds to empty if we see a non-finite value
        bounds.set(devVerts, vertexCount);
        if (bounds.isEmpty()) {
            return;
        }
    }

    VertState       state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    if (!(colors || textures)) {
        // no colors[] and no texture, stroke hairlines with paint's color.
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        SkAutoBlitterChoose blitter(*this, nullptr, p);
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
        return;
    }

    SkTriColorShader* triShader = nullptr;
    SkPMColor4f*  dstColors = nullptr;

    if (colors) {
        dstColors = convert_colors(colors, vertexCount, fDst.colorSpace(), &outerAlloc);
        triShader = outerAlloc.make<SkTriColorShader>(compute_is_opaque(colors, vertexCount));
        if (shader) {
            shader = outerAlloc.make<SkShader_Blend>(bmode,
                                                     sk_ref_sp(triShader), sk_ref_sp(shader),
                                                     nullptr);
        } else {
            shader = triShader;
        }
    }

    SkPaint p(paint);
    p.setShader(sk_ref_sp(shader));

    if (!textures) {    // only tricolor shader
        auto blitter = SkCreateRasterPipelineBlitter(fDst, p, *fMatrix, &outerAlloc);
        while (vertProc(&state)) {
            if (!triShader->update(ctmInv, vertices, dstColors, state.f0, state.f1, state.f2)) {
                continue;
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            SkScan::FillTriangle(tmp, *fRC, blitter);
        }
        return;
    }

    SkRasterPipeline pipeline(&outerAlloc);
    SkStageRec rec = {
        &pipeline, &outerAlloc, fDst.colorType(), fDst.colorSpace(), p, nullptr, *fMatrix
    };
    if (auto updater = as_SB(shader)->appendUpdatableStages(rec)) {
        bool isOpaque = shader->isOpaque();
        if (triShader) {
            isOpaque = false;   // unless we want to walk all the colors, and see if they are
                                // all opaque (and the blendmode will keep them that way
        }

        auto blitter = SkCreateRasterPipelineBlitter(fDst, p, pipeline, isOpaque, &outerAlloc);
        while (vertProc(&state)) {
            if (triShader && !triShader->update(ctmInv, vertices, dstColors,
                                                state.f0, state.f1, state.f2)) {
                continue;
            }

            SkMatrix localM;
            if (!texture_to_matrix(state, vertices, textures, &localM) ||
                !updater->update(*fMatrix, &localM)) {
                continue;
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            SkScan::FillTriangle(tmp, *fRC, blitter);
        }
    } else {
        // must rebuild pipeline for each triangle, to pass in the computed ctm
        while (vertProc(&state)) {
            if (triShader && !triShader->update(ctmInv, vertices, dstColors,
                                                state.f0, state.f1, state.f2)) {
                continue;
            }

            SkSTArenaAlloc<2048> innerAlloc;

            const SkMatrix* ctm = fMatrix;
            SkMatrix tmpCtm;
            if (textures) {
                SkMatrix localM;
                if (!texture_to_matrix(state, vertices, textures, &localM)) {
                    continue;
                }
                tmpCtm = SkMatrix::Concat(*fMatrix, localM);
                ctm = &tmpCtm;
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            auto blitter = SkCreateRasterPipelineBlitter(fDst, p, *ctm, &innerAlloc);
            SkScan::FillTriangle(tmp, *fRC, blitter);
        }
    }
}
