/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRSXform.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkScan.h"
#include "src/core/SkScan.h"
#include "src/core/SkVM.h"
#include "src/core/SkVMBlitter.h"
#include "src/shaders/SkComposeShader.h"
#include "src/shaders/SkShaderBase.h"

static void fill_rect(const SkMatrix& ctm, const SkRasterClip& rc,
                      const SkRect& r, SkBlitter* blitter, SkPath* scratchPath) {
    if (ctm.rectStaysRect()) {
        SkRect dr;
        ctm.mapRect(&dr, r);
        SkScan::FillRect(dr, rc, blitter);
    } else {
        SkPoint pts[4];
        r.toQuad(pts);
        ctm.mapPoints(pts, pts, 4);

        scratchPath->rewind();
        scratchPath->addPoly(pts, 4, true);
        SkScan::FillPath(*scratchPath, rc, blitter);
    }
}

static void load_color(SkRasterPipeline_UniformColorCtx* ctx, const float rgba[]) {
    // only need one of these. can I query the pipeline to know if its lowp or highp?
    ctx->rgba[0] = SkScalarRoundToInt(rgba[0]*255); ctx->r = rgba[0];
    ctx->rgba[1] = SkScalarRoundToInt(rgba[1]*255); ctx->g = rgba[1];
    ctx->rgba[2] = SkScalarRoundToInt(rgba[2]*255); ctx->b = rgba[2];
    ctx->rgba[3] = SkScalarRoundToInt(rgba[3]*255); ctx->a = rgba[3];
}

extern bool gUseSkVMBlitter;

class UpdatableColorShader : public SkShaderBase {
public:
    explicit UpdatableColorShader(SkColorSpace* cs)
        : fSteps{sk_srgb_singleton(), kUnpremul_SkAlphaType, cs, kUnpremul_SkAlphaType} {}
    skvm::Color onProgram(
            skvm::Builder* builder, skvm::Coord device, skvm::Coord local, skvm::Color paint,
            const SkMatrixProvider& provider, const SkMatrix* localM, const SkColorInfo& dst,
            skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        skvm::Uniform color = uniforms->pushPtr(fValues);
        skvm::F32 r = builder->arrayF(color, 0);
        skvm::F32 g = builder->arrayF(color, 1);
        skvm::F32 b = builder->arrayF(color, 2);
        skvm::F32 a = builder->arrayF(color, 3);

        return {r, g, b, a};
    }

    void updateColor(SkColor c) const {
        SkColor4f c4 = SkColor4f::FromColor(c);
        fSteps.apply(c4.vec());
        auto cp4 = c4.premul();
        fValues[0] = cp4.fR;
        fValues[1] = cp4.fG;
        fValues[2] = cp4.fB;
        fValues[3] = cp4.fA;
    }

private:
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    SkColorSpaceXformSteps fSteps;
    mutable float fValues[4];
};

void SkDraw::drawAtlas(const SkRSXform xform[],
                       const SkRect textures[],
                       const SkColor colors[],
                       int count,
                       sk_sp<SkBlender> blender,
                       const SkPaint& paint) {
    sk_sp<SkShader> atlasShader = paint.refShader();
    if (!atlasShader) {
        return;
    }

    SkSTArenaAlloc<256> alloc;

    SkPaint p(paint);
    p.setAntiAlias(false);  // we never respect this for drawAtlas(or drawVertices)
    p.setStyle(SkPaint::kFill_Style);
    p.setShader(nullptr);
    p.setMaskFilter(nullptr);

    auto rpblit = [&]() {
        SkRasterPipeline pipeline(&alloc);
        SkStageRec rec = {&pipeline,
                          &alloc,
                          fDst.colorType(),
                          fDst.colorSpace(),
                          p,
                          nullptr,
                          *fMatrixProvider};

        SkStageUpdater* updator = as_SB(atlasShader.get())->appendUpdatableStages(rec);
        if (!updator) {
            SkDraw draw(*this);

            p.setShader(atlasShader);
            for (int i = 0; i < count; ++i) {
                if (colors) {
                    p.setShader(
                            SkShaders::Blend(blender, SkShaders::Color(colors[i]), atlasShader));
                }
                SkMatrix mx;
                mx.setRSXform(xform[i]);
                mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
                SkPreConcatMatrixProvider matrixProvider(*fMatrixProvider, mx);
                draw.fMatrixProvider = &matrixProvider;
                draw.drawRect(textures[i], p);
            }
            return true;
        }

        SkRasterPipeline_UniformColorCtx* uniformCtx = nullptr;
        SkColorSpaceXformSteps steps(
                sk_srgb_singleton(), kUnpremul_SkAlphaType, rec.fDstCS, kUnpremul_SkAlphaType);

        if (colors) {
            // we will late-bind the values in ctx, once for each color in the loop
            uniformCtx = alloc.make<SkRasterPipeline_UniformColorCtx>();
            rec.fPipeline->append(SkRasterPipeline::uniform_color_dst, uniformCtx);
            if (skstd::optional<SkBlendMode> bm = as_BB(blender)->asBlendMode(); bm.has_value()) {
                SkBlendMode_AppendStages(*bm, rec.fPipeline);
            } else {
                return false;
            }
        }

        bool isOpaque = !colors && atlasShader->isOpaque();
        if (p.getAlphaf() != 1) {
            rec.fPipeline->append(SkRasterPipeline::scale_1_float,
                                  alloc.make<float>(p.getAlphaf()));
            isOpaque = false;
        }

        auto blitter = SkCreateRasterPipelineBlitter(
                fDst, p, pipeline, isOpaque, &alloc, fRC->clipShader());
        if (!blitter) {
            return false;
        }
        SkPath scratchPath;

        for (int i = 0; i < count; ++i) {
            if (colors) {
                SkColor4f c4 = SkColor4f::FromColor(colors[i]);
                steps.apply(c4.vec());
                load_color(uniformCtx, c4.premul().vec());
            }

            SkMatrix mx;
            mx.setRSXform(xform[i]);
            mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
            mx.postConcat(fMatrixProvider->localToDevice());

            if (updator->update(mx)) {
                fill_rect(mx, *fRC, textures[i], blitter, &scratchPath);
            }
        }
        return true;
    };

    if (gUseSkVMBlitter || !rpblit()) {
        auto updateShader = as_SB(atlasShader)->updatableShader(&alloc);
        UpdatableColorShader* colorShader = nullptr;
        SkShaderBase* shader = nullptr;
        if (colors) {
            colorShader = alloc.make<UpdatableColorShader>(fDst.colorSpace());
            shader = alloc.make<SkShader_Blend>(
                    std::move(blender), sk_ref_sp(colorShader), sk_ref_sp(updateShader));
        } else {
            shader = as_SB(updateShader);
        }
        p.setShader(sk_ref_sp(shader));
        if (auto blitter = SkVMBlitter::Make(fDst, p, *fMatrixProvider, &alloc,
                                             fRC->clipShader())) {
            SkPath scratchPath;
            for (int i = 0; i < count; ++i) {
                if (colorShader) {
                    colorShader->updateColor(colors[i]);
                }

                SkMatrix mx;
                mx.setRSXform(xform[i]);
                mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
                mx.postConcat(fMatrixProvider->localToDevice());
                if (updateShader->update(mx)) {
                    fill_rect(mx, *fRC, textures[i], blitter, &scratchPath);
                }
            }
        }
    }
}
