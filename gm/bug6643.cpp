/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShader.h"
#include "SkPictureRecorder.h"
#include "gm.h"

DEF_SIMPLE_GM(bug6643, canvas, 200, 200) {
    SkColor colors[] = { SK_ColorTRANSPARENT, SK_ColorGREEN, SK_ColorTRANSPARENT };

    SkPaint p;
    p.setAntiAlias(true);
    p.setShader(SkGradientShader::MakeSweep(100, 100, colors, nullptr, SK_ARRAY_COUNT(colors),
                                            SkGradientShader::kInterpolateColorsInPremul_Flag,
                                            nullptr));

    SkPictureRecorder recorder;
    recorder.beginRecording(200, 200)->drawPaint(p);

    p.setShader(SkShader::MakePictureShader(recorder.finishRecordingAsPicture(),
                                            SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode,
                                            nullptr, nullptr));
    canvas->drawColor(SK_ColorWHITE);
    canvas->drawPaint(p);
}

#include "SkTextBlob.h"

static sk_sp<SkTextBlob> makeBlob() {
    SkTextBlobBuilder textBlobBuilder;
    const char bunny[] = "/(^x^)\\";
    const int len = sizeof(bunny) - 1;
    uint16_t glyphs[len];
    SkPaint paint;
    paint.textToGlyphs(bunny, len, glyphs);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    int runs[] = { 3, 1, 3 };
    SkPoint textPos = { 20, 100 };
    int glyphIndex = 0;
    for (auto runLen : runs) {
        paint.setTextSize(1 == runLen ? 20 : 50);
        const SkTextBlobBuilder::RunBuffer& run = 
                textBlobBuilder.allocRun(paint, runLen, textPos.fX, textPos.fY);
        memcpy(run.glyphs, &glyphs[glyphIndex], sizeof(glyphs[0]) * runLen);
        textPos.fX += paint.measureText(&glyphs[glyphIndex], sizeof(glyphs[0]) * runLen, nullptr);
        glyphIndex += runLen;
    }
    return textBlobBuilder.make();
}

static void drawBlob(SkCanvas* canvas, sk_sp<SkTextBlob> blob) {
    SkPaint paint;
    canvas->drawTextBlob(blob.get(), 10, 10, paint);
}

static void drawXform(SkCanvas*canvas) {
    sk_sp<SkTextBlob> blob = makeBlob();
    drawBlob(canvas, blob);
    canvas->save();
    canvas->scale(1.2f, 1.2f);
    canvas->translate(150, 0);
    drawBlob(canvas, blob);
    canvas->save();
    canvas->scale(1.2f, 1.2f);
    canvas->translate(150, 0);
    drawBlob(canvas, blob);
    canvas->restore();
    canvas->restore();
}

DEF_SIMPLE_GM(flutterbug, canvas, 200, 200) {
    drawXform(canvas);
    SkPictureRecorder recorder;
    SkCanvas* rCanvas = recorder.beginRecording(200, 200);
    SkPaint p;
    rCanvas->scale(1.2f, 1.2f);
    drawXform(rCanvas);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    canvas->translate(0, 100);
    canvas->drawPicture(picture);
    canvas->translate(0, 120);
    canvas->scale(1.2f, 1.2f);
    drawXform(canvas);
    canvas->scale(1.2f, 1.2f);
    rCanvas->scale(1.2f, 1.2f);
    canvas->translate(0, 120);
    drawXform(canvas);
    canvas->translate(0, 120);
    canvas->drawPicture(picture);
}
