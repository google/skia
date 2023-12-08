// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(BlendModes, 256, 4352, false, 0) {
void drawBG(SkCanvas* canvas) {
    SkColor radColors[] = {0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF};
    auto rad = SkGradientShader::MakeRadial(SkPoint::Make(128, 128), 128, radColors, nullptr, 3,
                                            SkTileMode::kClamp);

    SkMatrix rotMtx;
    rotMtx.setRotate(-90, 128, 128);
    SkColor sweepColors[] = {0xFFFF00FF, 0xFFFF0000, 0xFFFFFF00, 0xFF00FF00,
                             0xFF00FFFF, 0xFF0000FF, 0xFFFF00FF};
    auto sweep = SkGradientShader::MakeSweep(128, 128, sweepColors, nullptr, 7, 0, &rotMtx);

    auto comp = SkShaders::Blend(SkBlendMode::kModulate, std::move(rad), std::move(sweep));
    SkPaint p;
    p.setShader(std::move(comp));

    canvas->drawPaint(p);
}

void draw(SkCanvas* canvas) {
    SkBlendMode blendModes[] = {
        SkBlendMode::kDst,
        SkBlendMode::kSrc,
        SkBlendMode::kSrcOver,
        SkBlendMode::kDstOver,
        SkBlendMode::kSrcIn,
        SkBlendMode::kDstIn,
        SkBlendMode::kSrcOut,
        SkBlendMode::kDstOut,
        SkBlendMode::kSrcATop,
        SkBlendMode::kDstATop,
        SkBlendMode::kXor,
        SkBlendMode::kPlus,
        SkBlendMode::kModulate,
        SkBlendMode::kScreen,
        SkBlendMode::kOverlay,
        SkBlendMode::kDarken,
        SkBlendMode::kLighten,
    };

    SkPaint labelPaint;
    labelPaint.setAntiAlias(true);
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 12);

    for (auto mode : blendModes) {
        SkPaint layerPaint;
        layerPaint.setBlendMode(mode);

        canvas->save();
        canvas->clipRect(SkRect::MakeWH(256, 256));

        drawBG(canvas);

        canvas->saveLayer(nullptr, &layerPaint);
        const SkScalar r = 80;
        SkPaint discP;
        discP.setAntiAlias(true);
        discP.setBlendMode(SkBlendMode::kPlus);
        discP.setColor(SK_ColorGREEN); canvas->drawCircle(128, r, r, discP);
        discP.setColor(SK_ColorRED);   canvas->drawCircle(r, 256 - r, r, discP);
        discP.setColor(SK_ColorBLUE);  canvas->drawCircle(256 - r, 256 - r, r, discP);
        canvas->restore();

        canvas->drawSimpleText(SkBlendMode_Name(mode), strlen(SkBlendMode_Name(mode)),
                               SkTextEncoding::kUTF8, 10, 10, font, labelPaint);
        canvas->restore();
        canvas->translate(0, 256);
    }
}
}  // END FIDDLE
