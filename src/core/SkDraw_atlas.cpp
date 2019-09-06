/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkRSXform.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkScan.h"
#include "src/shaders/SkShaderBase.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkScan.h"

static void fill_rect(const SkMatrix& ctm, const SkRasterClip& rc,
                      const SkRect& r, SkBlitter* blitter, SkPath* scratchPath) {
    if (ctm.rectStaysRect()) {
        SkRect dr;
        ctm.mapRect(&dr, r);
        SkScan::FillRect(dr, rc, blitter);
    } else {
        scratchPath->rewind();
        scratchPath->addRect(r);
        scratchPath->transform(ctm);
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

void SkDraw::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect textures[],
                       const SkColor colors[], int count, SkBlendMode bmode, const SkPaint& paint) {
    sk_sp<SkShader> atlasShader = atlas->makeShader();
    if (!atlasShader) {
        return;
    }

    SkPaint p(paint);
    p.setAntiAlias(false);  // we never respect this for drawAtlas(or drawVertices)
    p.setStyle(SkPaint::kFill_Style);
    p.setShader(nullptr);
    p.setMaskFilter(nullptr);

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline pipeline(&alloc);
    SkStageRec rec = {
        &pipeline, &alloc, fDst.colorType(), fDst.colorSpace(), p, nullptr, *fMatrix
    };

    SkStageUpdater* updator = as_SB(atlasShader.get())->appendUpdatableStages(rec);
    if (!updator) {
        SkDraw draw(*this);

        p.setShader(atlasShader);
        for (int i = 0; i < count; ++i) {
            if (colors) {
                p.setShader(SkShaders::Blend(bmode, SkShaders::Color(colors[i]), atlasShader));
            }
            SkMatrix mx;
            mx.setRSXform(xform[i]);
            mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
            mx.postConcat(*fMatrix);
            draw.fMatrix = &mx;
            draw.drawRect(textures[i], p);
        }
        return;
    }

    SkRasterPipeline_UniformColorCtx* uniformCtx = nullptr;
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 rec.fDstCS,          kUnpremul_SkAlphaType);

    if (colors) {
        // we will late-bind the values in ctx, once for each color in the loop
        uniformCtx = alloc.make<SkRasterPipeline_UniformColorCtx>();
        rec.fPipeline->append(SkRasterPipeline::uniform_color_dst, uniformCtx);
        SkBlendMode_AppendStages(bmode, rec.fPipeline);
    }

    bool isOpaque = !colors && atlasShader->isOpaque();
    if (p.getAlphaf() != 1) {
        rec.fPipeline->append(SkRasterPipeline::scale_1_float, alloc.make<float>(p.getAlphaf()));
        isOpaque = false;
    }

    auto blitter = SkCreateRasterPipelineBlitter(fDst, p, pipeline, isOpaque, &alloc);
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
        mx.postConcat(*fMatrix);

        updator->update(mx, nullptr);
        fill_rect(mx, *fRC, textures[i], blitter, &scratchPath);
    }
}
