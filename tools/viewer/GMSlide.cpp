/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GMSlide.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "sk_tool_utils.h"

GMSlide::GMSlide(skiagm::GM* gm) : fGM(gm) {
    fName.printf("GM_%s", gm->getName());
}

GMSlide::~GMSlide() { delete fGM; }

static void draw_gpu_only_message(SkCanvas* canvas) {
    SkBitmap bmp;
    bmp.allocN32Pixels(128, 64);
    SkCanvas bmpCanvas(bmp);
    bmpCanvas.drawColor(SK_ColorWHITE);
    SkFont font(sk_tool_utils::create_portable_typeface(), 20);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    bmpCanvas.drawString("GPU Only", 20, 40, font, paint);
    SkMatrix localM;
    localM.setRotate(35.f);
    localM.postTranslate(10.f, 0.f);
    paint.setShader(SkShader::MakeBitmapShader(
            bmp, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode, &localM));
    paint.setFilterQuality(kMedium_SkFilterQuality);
    canvas->drawPaint(paint);
}

void GMSlide::draw(SkCanvas* canvas) {
    // Do we care about timing the draw of the background (once)?
    // Does the GM ever rely on drawBackground to lazily compute something?
    fGM->drawBackground(canvas);
    if (const char* skipReason = fGM->drawContent(canvas)) {
        // Draw the skip reason for viewer. GMs will just not produce an image at all.
        if (skiagm::GM::kDrawSkippedGPUOnly == skipReason) {
            draw_gpu_only_message(canvas);
        } else {
            skiagm::GM::DrawFailureMessage(canvas, "DRAW SKIPPED: %s", skipReason);
        }
    }
}

bool GMSlide::animate(const SkAnimTimer& timer) {
    return fGM->animate(timer);
}

bool GMSlide::onChar(SkUnichar c) {
    return fGM->handleKey(c);
}

bool GMSlide::onGetControls(SkMetaData* controls) {
    return fGM->getControls(controls);
}

void GMSlide::onSetControls(const SkMetaData& controls) {
    fGM->setControls(controls);
}

