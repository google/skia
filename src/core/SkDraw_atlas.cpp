/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkRSXform.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkScan.h"
#include "src/shaders/SkShaderBase.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkScan.h"

static void fill_rect(const SkMatrix& ctm, const SkRasterClip& rc,
                      const SkRect& r, SkBlitter* blitter) {
    if (ctm.rectStaysRect()) {
        SkRect dr;
        ctm.mapRect(&dr, r);
        SkScan::FillRect(dr, rc, blitter);
    } else {
        SkPath path;
        path.addRect(r);
        path.transform(ctm);
        SkScan::FillPath(path, rc, blitter);
    }
}

static void load_color(SkRasterPipeline_UniformColorCtx* ctx, SkColor c) {
    const float premulScale = SkColorGetA(c) * (1.0f/(255*255));
    SkPMColor pm = SkPreMultiplyColor(c);
    // only need one of these. can I query the pipeline to know if its lowp or highp?
    ctx->rgba[0] = SkGetPackedR32(pm); ctx->r = SkColorGetR(c) * premulScale;
    ctx->rgba[1] = SkGetPackedG32(pm); ctx->g = SkColorGetG(c) * premulScale;
    ctx->rgba[2] = SkGetPackedB32(pm); ctx->b = SkColorGetB(c) * premulScale;
    ctx->rgba[3] = SkGetPackedA32(pm); ctx->a = SkColorGetA(c) * (1.0f/255);
}

void SkDraw::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect textures[],
                       const SkColor colors[], int count, SkBlendMode bmode, const SkPaint& paint) {
    sk_sp<SkShader> atlasShader = atlas->makeShader();
    if (!atlasShader) {
        return;
    }

    SkMatrix xf;

    SkPaint p(paint);
    p.setAntiAlias(false);  // we never respect this for drawAtlas(or drawVertices)
    p.setStyle(SkPaint::kFill_Style);
    p.setShader(nullptr);
    p.setMaskFilter(nullptr);

    if (false) {
        SkDraw draw(*this);

        p.setShader(atlasShader);

        for (int i = 0; i < count; ++i) {
            if (colors) {
                p.setShader(SkShaders::Blend(bmode, SkShaders::Color(colors[i]), atlasShader));
            }
            xf.setRSXform(xform[i]);
            xf.preTranslate(-textures[i].fLeft, -textures[i].fTop);
            xf.postConcat(*fMatrix);
            draw.fMatrix = &xf;
            draw.drawRect(textures[i], p);
        }
        return;
    }

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline pipeline(&alloc);
    SkStageRec rec = {
        &pipeline, &alloc, fDst.colorType(), fDst.colorSpace(), p, nullptr, *fMatrix
    };
    if (!as_SB(atlasShader.get())->appendStages(rec)) {
        return;
    }

    SkRasterPipeline_UniformColorCtx* uniformCtx = nullptr;

    if (colors) {
        // we will late-bind the values in ctx, once for each color in the loop
        uniformCtx = alloc.make<SkRasterPipeline_UniformColorCtx>();

        rec.fPipeline->append(SkRasterPipeline::uniform_color_dst, uniformCtx);
        SkBlendMode_AppendStages(bmode, rec.fPipeline);
    }

    bool isOpaque = !colors && atlasShader->isOpaque();
    if (p.getAlphaf() != 1) {
        rec.fPipeline->append(SkRasterPipeline::scale_1_float, alloc.make<float>(p.getAlphaf()));
    }

    auto blitter = SkCreateRasterPipelineBlitter(fDst, p, pipeline, isOpaque, &alloc);

    for (int i = 0; i < count; ++i) {
        if (colors) {
            load_color(uniformCtx, colors[i]);
        }
        xf.setRSXform(xform[i]);
        xf.preTranslate(-textures[i].fLeft, -textures[i].fTop);
        xf.postConcat(*fMatrix);
        // todo: update the imageshader's pipeline to know about the new matrix

        fill_rect(xf, *fRC, textures[i], blitter);
    }
}
