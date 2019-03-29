/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkMixer.h"
#include "SkScan.h"
#include "SkShaderBase.h"
#include "SkRSXform.h"

void SkDraw::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect textures[],
                       const SkColor colors[], int count, SkBlendMode bmode, const SkPaint& paint) {
    SkDraw draw(*this);
    SkPaint p(paint);

    p.setAntiAlias(false);  // we never respect this for drawAtlas(or drawVertices)
    p.setStyle(SkPaint::kFill_Style);
    p.setShader(nullptr);
    p.setMaskFilter(nullptr);

    sk_sp<SkShader> atlasShader;
    if (atlas) {
        atlasShader = atlas->makeShader();
    }

    // The fast trick is to make a mixer-shader once, and then pipe-in the blend color per-draw
    // via the paint (p).
    // This allows us to only make the shader once. The limitation is that the rasterpipeline
    // *also* pays attention to the paint's color (its alpha actually), so we can only play this
    // game for opaque colors.
    //
    // If we can build the pipeline ourselves, we can take care of this final alpha-modulate step
    // ourselves, so we could always take the one-shader route (and ideally share the same pipeline
    // for all draws).

    auto mixer = SkMixer::MakeBlend(bmode);
    sk_sp<SkShader> mixerShader;
    if (atlasShader && colors && p.getAlpha() == 0xFF) {
        mixerShader = SkShader::MakeMixer(nullptr, atlasShader, mixer);
    }

    SkMatrix xf;
    for (int i = 0; i < count; ++i) {
        if (colors) {
            if (atlasShader) {
                if (mixerShader && SkColorGetA(colors[i]) == 0xFF) {
                    p.setShader(mixerShader);
                    p.setColor(colors[i]);
                } else {
                    p.setShader(SkShader::MakeMixer(SkShader::MakeColorShader(colors[i]),
                                                    atlasShader, mixer));
                    p.setColor4f(paint.getColor4f(), nullptr);
                }
            } else {
                p.setColor(colors[i]);
            }
        } else {
            p.setShader(atlasShader);
        }
        xf.setRSXform(xform[i]);
        xf.preTranslate(-textures[i].fLeft, -textures[i].fTop);
        xf.postConcat(*fMatrix);
        draw.fMatrix = &xf;
        draw.drawRect(textures[i], p);
    }
}
