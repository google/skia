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
    // only need one of these. can I query the pipeline to know if its lowp or highp?
    ctx->rgba[0] = SkColorGetR(c); ctx->r = SkColorGetR(c) * (1.0f/255);
    ctx->rgba[1] = SkColorGetG(c); ctx->g = SkColorGetG(c) * (1.0f/255);
    ctx->rgba[2] = SkColorGetB(c); ctx->b = SkColorGetB(c) * (1.0f/255);
    ctx->rgba[3] = SkColorGetA(c); ctx->a = SkColorGetA(c) * (1.0f/255);
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
    SkRasterPipeline_<256> pipeline;
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

        rec.fPipeline->append(SkRasterPipeline::move_src_dst);
        rec.fPipeline->unchecked_append(SkRasterPipeline::uniform_color, uniformCtx);
        rec.fPipeline->append(SkRasterPipeline::move_dst_src);

        SkBlendMode_AppendStages(bmode, rec.fPipeline);
    }

    bool isOpaque = !colors && p.getAlphaf() == 1 && atlasShader->isOpaque();
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
