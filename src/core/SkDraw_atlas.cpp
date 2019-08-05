/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkRSXform.h"
#include "src/core/SkDraw.h"
#include "src/core/SkScan.h"
#include "src/shaders/SkShaderBase.h"

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

    SkMatrix xf;
    for (int i = 0; i < count; ++i) {
        if (colors) {
            if (atlasShader) {
                p.setShader(SkShaders::Blend(bmode,
                                             SkShaders::Color(colors[i]),
                                             atlasShader));
                p.setColor4f(paint.getColor4f(), nullptr);
            } else {
                p.setColor(colors[i]);  // modulate with paint.getAlpha()?
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
