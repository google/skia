/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkAutoBlitterChoose.h"
#include "SkColorShader.h"
#include "SkDraw.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkRasterClip.h"
#include "SkScan.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkVertState.h"

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

class SkTriColorShader : public SkShader {
public:
    SkTriColorShader();

    class TriColorShaderContext : public SkShader::Context {
    public:
        TriColorShaderContext(const SkTriColorShader& shader, const ContextRec&);
        ~TriColorShaderContext() override;
        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f dstC[], int count) override;

    private:
        bool setup(const SkPoint pts[], const SkColor colors[], int, int, int);

        SkMatrix    fDstToUnit;
        SkPMColor   fColors[3];
        bool fSetup;

        Matrix43 fM43;

        typedef SkShader::Context INHERITED;
    };

    struct TriColorShaderData {
        const SkPoint* pts;
        const SkColor* colors;
        const VertState *state;
    };

    SK_TO_STRING_OVERRIDE()

    // For serialization.  This will never be called.
    Factory getFactory() const override { sk_throw(); return nullptr; }

    // Supply setup data to context from drawing setup
    void bindSetupData(TriColorShaderData* setupData) { fSetupData = setupData; }

    // Take the setup data from context when needed.
    TriColorShaderData* takeSetupData() {
        TriColorShaderData *data = fSetupData;
        fSetupData = NULL;
        return data;
    }

protected:
    Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const override {
        return alloc->make<TriColorShaderContext>(*this, rec);
    }

private:
    TriColorShaderData *fSetupData;

    typedef SkShader INHERITED;
};

bool SkTriColorShader::TriColorShaderContext::setup(const SkPoint pts[], const SkColor colors[],
                                                    int index0, int index1, int index2) {

    fColors[0] = SkPreMultiplyColor(colors[index0]);
    fColors[1] = SkPreMultiplyColor(colors[index1]);
    fColors[2] = SkPreMultiplyColor(colors[index2]);

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
    // We can't call getTotalInverse(), because we explicitly don't want to look at the localmatrix
    // as our interators are intrinsically tied to the vertices, and nothing else.
    SkMatrix ctmInv;
    if (!this->getCTM().invert(&ctmInv)) {
        return false;
    }
    // TODO replace INV(m) * INV(ctm) with INV(ctm * m)
    fDstToUnit.setConcat(im, ctmInv);

    Sk4f alpha(this->getPaintAlpha() * (1.0f / 255)),
    c0 = SkPM4f::FromPMColor(fColors[0]).to4f() * alpha,
    c1 = SkPM4f::FromPMColor(fColors[1]).to4f() * alpha,
    c2 = SkPM4f::FromPMColor(fColors[2]).to4f() * alpha;

    Matrix43 colorm;
    (c1 - c0).store(&colorm.fMat[0]);
    (c2 - c0).store(&colorm.fMat[4]);
    c0.store(&colorm.fMat[8]);
    fM43.setConcat(colorm, fDstToUnit);

    return true;
}

#include "SkColorPriv.h"
#include "SkComposeShader.h"

static int ScalarTo256(SkScalar v) {
    return static_cast<int>(SkScalarPin(v, 0, 1) * 256 + 0.5);
}

SkTriColorShader::SkTriColorShader()
: INHERITED(NULL)
, fSetupData(NULL) {}

SkTriColorShader::TriColorShaderContext::TriColorShaderContext(const SkTriColorShader& shader,
                                                               const ContextRec& rec)
: INHERITED(shader, rec)
, fSetup(false) {}

SkTriColorShader::TriColorShaderContext::~TriColorShaderContext() {}

void SkTriColorShader::TriColorShaderContext::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkTriColorShader* parent = static_cast<SkTriColorShader*>(const_cast<SkShader*>(&fShader));
    TriColorShaderData* set = parent->takeSetupData();
    if (set) {
        fSetup = setup(set->pts, set->colors, set->state->f0, set->state->f1, set->state->f2);
    }

    if (!fSetup) {
        // Invalid matrices. Not checked before so no need to assert.
        return;
    }

    const int alphaScale = Sk255To256(this->getPaintAlpha());

    SkPoint src;

    fDstToUnit.mapXY(SkIntToScalar(x) + 0.5, SkIntToScalar(y) + 0.5, &src);
    for (int i = 0; i < count; i++) {
        int scale1 = ScalarTo256(src.fX);
        int scale2 = ScalarTo256(src.fY);
        int scale0 = 256 - scale1 - scale2;
        if (scale0 < 0) {
            if (scale1 > scale2) {
                scale2 = 256 - scale1;
            } else {
                scale1 = 256 - scale2;
            }
            scale0 = 0;
        }

        if (256 != alphaScale) {
            scale0 = SkAlphaMul(scale0, alphaScale);
            scale1 = SkAlphaMul(scale1, alphaScale);
            scale2 = SkAlphaMul(scale2, alphaScale);
        }

        dstC[i] = SkAlphaMulQ(fColors[0], scale0) +
        SkAlphaMulQ(fColors[1], scale1) +
        SkAlphaMulQ(fColors[2], scale2);

        src.fX += fDstToUnit.getScaleX();
        src.fY += fDstToUnit.getSkewY();
    }
}

void SkTriColorShader::TriColorShaderContext::shadeSpan4f(int x, int y, SkPM4f dstC[], int count) {
    SkTriColorShader* parent = static_cast<SkTriColorShader*>(const_cast<SkShader*>(&fShader));
    TriColorShaderData* set = parent->takeSetupData();
    if (set) {
        fSetup = setup(set->pts, set->colors, set->state->f0, set->state->f1, set->state->f2);
    }

    if (!fSetup) {
        // Invalid matrices. Not checked before so no need to assert.
        return;
    }

    Sk4f c  = fM43.map(SkIntToScalar(x) + 0.5, SkIntToScalar(y) + 0.5),
    dc = Sk4f::Load(&fM43.fMat[0]),
    zero(0.0f),
    one(1.0f);

    for (int i = 0; i < count; i++) {
        // We don't expect to be wildly out of 0...1, but we pin just because of minor
        // numerical imprecision.
        Sk4f::Min(Sk4f::Max(c, zero), Sk4f::Min(c[3], one)).store(dstC + i);
        c += dc;
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkTriColorShader::toString(SkString* str) const {
    str->append("SkTriColorShader: (");
    
    this->INHERITED::toString(str);
    
    str->append(")");
}
#endif


namespace {

    // Similar to SkLocalMatrixShader, but composes the local matrix with the CTM (instead
    // of composing with the inherited local matrix):
    //
    //   rec' = {rec.ctm x localMatrix, rec.localMatrix}
    //
    // (as opposed to rec' = {rec.ctm, rec.localMatrix x localMatrix})
    //
    class SkLocalInnerMatrixShader final : public SkShader {
    public:
        SkLocalInnerMatrixShader(sk_sp<SkShader> proxy, const SkMatrix& localMatrix)
        : INHERITED(&localMatrix)
        , fProxyShader(std::move(proxy)) {}

        Factory getFactory() const override {
            SkASSERT(false);
            return nullptr;
        }

    protected:
        void flatten(SkWriteBuffer&) const override {
            SkASSERT(false);
        }

        Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const override {
            SkMatrix adjustedCTM = SkMatrix::Concat(*rec.fMatrix, this->getLocalMatrix());
            ContextRec newRec(rec);
            newRec.fMatrix = &adjustedCTM;
            return fProxyShader->makeContext(newRec, alloc);
        }

        bool onAppendStages(SkRasterPipeline* p, SkColorSpace* cs, SkArenaAlloc* alloc,
                            const SkMatrix& ctm, const SkPaint& paint,
                            const SkMatrix* localM) const override {
            // We control the shader graph ancestors, so we know there's no local matrix being
            // injected before this.
            SkASSERT(!localM);

            SkMatrix adjustedCTM = SkMatrix::Concat(ctm, this->getLocalMatrix());
            return fProxyShader->appendStages(p, cs, alloc, adjustedCTM, paint);
        }

    private:
        sk_sp<SkShader> fProxyShader;

        typedef SkShader INHERITED;
    };

    sk_sp<SkShader> MakeTextureShader(const VertState& state, const SkPoint verts[],
                                      const SkPoint texs[], const SkPaint& paint,
                                      SkColorSpace* dstColorSpace,
                                      SkArenaAlloc* alloc) {
        SkASSERT(paint.getShader());

        const auto& p0 = texs[state.f0],
        p1 = texs[state.f1],
        p2 = texs[state.f2];

        if (p0 != p1 || p0 != p2) {
            // Common case (non-collapsed texture coordinates).
            // Map the texture to vertices using a local transform.

            // We cannot use a plain SkLocalMatrix shader, because we need the texture matrix
            // to compose next to the CTM.
            SkMatrix localMatrix;
            return texture_to_matrix(state, verts, texs, &localMatrix)
            ? alloc->makeSkSp<SkLocalInnerMatrixShader>(paint.refShader(), localMatrix)
            : nullptr;
        }

        // Collapsed texture coordinates special case.
        // The texture is a solid color, sampled at the given point.
        SkMatrix shaderInvLocalMatrix;
        SkAssertResult(paint.getShader()->getLocalMatrix().invert(&shaderInvLocalMatrix));

        const auto sample       = SkPoint::Make(0.5f, 0.5f);
        const auto mappedSample = shaderInvLocalMatrix.mapXY(sample.x(), sample.y()),
        mappedPoint  = shaderInvLocalMatrix.mapXY(p0.x(), p0.y());
        const auto localMatrix  = SkMatrix::MakeTrans(mappedSample.x() - mappedPoint.x(),
                                                      mappedSample.y() - mappedPoint.y());

        SkShader::ContextRec rec(paint, SkMatrix::I(), &localMatrix,
                                 SkShader::ContextRec::kPMColor_DstType, dstColorSpace);
        auto* ctx = paint.getShader()->makeContext(rec, alloc);
        if (!ctx) {
            return nullptr;
        }

        SkPMColor pmColor;
        ctx->shadeSpan(SkScalarFloorToInt(sample.x()), SkScalarFloorToInt(sample.y()), &pmColor, 1);

        // no need to keep this temp context around.
        alloc->reset();

        return alloc->makeSkSp<SkColorShader>(SkUnPreMultiply::PMColorToColor(pmColor));
    }

} // anonymous ns

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

    // transform out vertices into device coordinates
    SkAutoSTMalloc<16, SkPoint> storage(count);
    SkPoint* devVerts = storage.get();
    fMatrix->mapPoints(devVerts, vertices, count);

    /*
     We can draw the vertices in 1 of 4 ways:

     - solid color (no shader/texture[], no colors[])
     - just colors (no shader/texture[], has colors[])
     - just texture (has shader/texture[], no colors[])
     - colors * texture (has shader/texture[], has colors[])

     Thus for texture drawing, we need both texture[] and a shader.
     */

    auto triShader = sk_make_sp<SkTriColorShader>();
    SkPaint p(paint);

    SkShader* shader = p.getShader();
    if (nullptr == shader) {
        // if we have no shader, we ignore the texture coordinates
        textures = nullptr;
    } else if (nullptr == textures) {
        // if we don't have texture coordinates, ignore the shader
        p.setShader(nullptr);
        shader = nullptr;
    }

    // setup the custom shader (if needed)
    if (colors) {
        if (nullptr == textures) {
            // just colors (no texture)
            p.setShader(triShader);
        } else {
            // colors * texture
            SkASSERT(shader);
            p.setShader(SkShader::MakeComposeShader(triShader, sk_ref_sp(shader), bmode));
        }
    }

    SkAutoBlitterChoose blitter(fDst, *fMatrix, p);
    // Abort early if we failed to create a shader context.
    if (blitter->isNullBlitter()) {
        return;
    }

    // setup our state and function pointer for iterating triangles
    VertState       state(count, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    if (textures || colors) {
        SkTriColorShader::TriColorShaderData verticesSetup = { vertices, colors, &state };

        while (vertProc(&state)) {
            auto* blitterPtr = blitter.get();

            // We're going to allocate at most
            //
            //   * one SkLocalMatrixShader OR one SkColorShader
            //   * one SkComposeShader
            //   * one SkAutoBlitterChoose
            //
            static constexpr size_t kAllocSize =
            sizeof(SkAutoBlitterChoose) + sizeof(SkComposeShader) +
            SkTMax(sizeof(SkLocalInnerMatrixShader), sizeof(SkColorShader));
            char allocBuffer[kAllocSize];
            SkArenaAlloc alloc(allocBuffer);

            if (textures) {
                sk_sp<SkShader> texShader = MakeTextureShader(state, vertices, textures, paint,
                                                              fDst.colorSpace(), &alloc);
                if (texShader) {
                    SkPaint localPaint(p);
                    localPaint.setShader(colors
                                         ? alloc.makeSkSp<SkComposeShader>(triShader, std::move(texShader), bmode)
                                         : std::move(texShader));

                    blitterPtr = alloc.make<SkAutoBlitterChoose>(fDst, *fMatrix, localPaint)->get();
                    if (blitterPtr->isNullBlitter()) {
                        continue;
                    }
                }
            }
            if (colors) {
                triShader->bindSetupData(&verticesSetup);
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            SkScan::FillTriangle(tmp, *fRC, blitterPtr);
            triShader->bindSetupData(nullptr);
        }
    } else {
        // no colors[] and no texture, stroke hairlines with paint's color.
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
