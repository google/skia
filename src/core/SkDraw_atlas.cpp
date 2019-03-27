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

static SkBlendMode reverse_blendmode(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrc: mode = SkBlendMode::kDst; break;
        case SkBlendMode::kDst: mode = SkBlendMode::kSrc; break;

        case SkBlendMode::kSrcOver: mode = SkBlendMode::kDstOver; break;
        case SkBlendMode::kDstOver: mode = SkBlendMode::kSrcOver; break;

        case SkBlendMode::kSrcIn: mode = SkBlendMode::kDstIn; break;
        case SkBlendMode::kDstIn: mode = SkBlendMode::kSrcIn; break;

        case SkBlendMode::kSrcOut: mode = SkBlendMode::kDstOut; break;
        case SkBlendMode::kDstOut: mode = SkBlendMode::kSrcOut; break;

        case SkBlendMode::kSrcATop: mode = SkBlendMode::kDstATop; break;
        case SkBlendMode::kDstATop: mode = SkBlendMode::kSrcATop; break;

        default: break;
    }
    return mode;
}

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

    sk_sp<SkShader> mixerShader;
    if (atlasShader && colors && p.getAlpha() == 0xFF) {
        mixerShader = SkShader::MakeMixer(nullptr, atlasShader, SkMixer::MakeBlend(bmode));
    }

    // Need an option in modecolorfilter do perform this reverse
    bmode = reverse_blendmode(bmode);

    SkMatrix xf;
    for (int i = 0; i < count; ++i) {
        if (colors) {
            if (atlasShader) {
                if (mixerShader && SkColorGetA(colors[i]) == 0xFF) {
                    p.setShader(mixerShader);
                    p.setColor(colors[i]);
                } else {
                    auto cf = SkColorFilter::MakeModeFilter(colors[i], bmode);
                    p.setShader(atlasShader->makeWithColorFilter(cf));
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
