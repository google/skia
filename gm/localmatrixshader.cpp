/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "ToolUtils.h"
#include "gm.h"

static sk_sp<SkImage> make_image(SkCanvas* rootCanvas) {
    static constexpr SkScalar kSize = 50;
    SkImageInfo info = SkImageInfo::MakeN32Premul(kSize, kSize);
    auto                      surface = ToolUtils::makeSurface(rootCanvas, info);

    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorGREEN);

    surface->getCanvas()->drawCircle(kSize / 2, kSize / 2, kSize / 2, p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorRED);
    surface->getCanvas()->drawLine(kSize * .25f, kSize * .50f, kSize * .75f, kSize * .50f, p);
    surface->getCanvas()->drawLine(kSize * .50f, kSize * .25f, kSize * .50f, kSize * .75f, p);

    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(localmatrixshader_nested, canvas, 450, 1200) {
    auto image = make_image(canvas);

    using FactoryT = sk_sp<SkShader> (*)(const sk_sp<SkImage>&,
                                         const SkMatrix& inner,
                                         const SkMatrix& outer);
    static const FactoryT gFactories[] = {
        // SkLocalMatrixShader(SkImageShader(inner), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return img->makeShader(&inner)->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkLocalMatrixShader(SkImageShader(I), inner), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return img->makeShader()->makeWithLocalMatrix(inner)->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkComposeShader(SkImageShader(inner)), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return SkShader::MakeCompose(SkShader::MakeColorShader(SK_ColorTRANSPARENT),
                                         img->makeShader(&inner),
                                         SkBlendMode::kSrcOver)
                   ->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkComposeShader(SkLocalMatrixShader(SkImageShader(I), inner)), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return SkShader::MakeCompose(SkShader::MakeColorShader(SK_ColorTRANSPARENT),
                                         img->makeShader()->makeWithLocalMatrix(inner),
                                         SkBlendMode::kSrcOver)
                   ->makeWithLocalMatrix(outer);
        },
    };

    static const auto inner = SkMatrix::MakeScale(2, 2),
                      outer = SkMatrix::MakeTrans(20, 20);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);

    auto rect = SkRect::Make(image->bounds());
    SkAssertResult(SkMatrix::Concat(inner, outer).mapRect(&rect));

    const auto drawColumn = [&]() {
        SkAutoCanvasRestore acr(canvas, true);
        for (const auto& f : gFactories) {
            SkPaint p;
            p.setShader(f(image, inner, outer));

            canvas->drawRect(rect, p);
            canvas->drawRect(rect, border);

            canvas->translate(0, rect.height() * 1.5f);
        }
    };

    drawColumn();

    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(0, rect.height() * SK_ARRAY_COUNT(gFactories) * 1.5f);
        drawColumn();
    }

    canvas->translate(rect.width() * 1.5f, 0);
    canvas->scale(2, 2);
    drawColumn();
}
