/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkShader.h"
using namespace skiagm;

GM::GM() {
    fMode = kGM_Mode;
    fBGColor = SK_ColorWHITE;
    fCanvasIsDeferred = false;
    fHaveCalledOnceBeforeDraw = false;
    fStarterMatrix.reset();
}

GM::~GM() {}

void GM::draw(SkCanvas* canvas) {
    this->drawBackground(canvas);
    this->drawContent(canvas);
}

void GM::drawContent(SkCanvas* canvas) {
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    this->onDraw(canvas);
}

void GM::drawBackground(SkCanvas* canvas) {
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    this->onDrawBackground(canvas);
}

const char* GM::getName() {
    if (fShortName.size() == 0) {
        fShortName = this->onShortName();
    }
    return fShortName.c_str();
}

void GM::setBGColor(SkColor color) {
    fBGColor = color;
}

bool GM::animate(const SkAnimTimer& timer) {
    return this->onAnimate(timer);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GM::onDrawBackground(SkCanvas* canvas) {
    canvas->drawColor(fBGColor, SkXfermode::kSrc_Mode);
}

void GM::drawSizeBounds(SkCanvas* canvas, SkColor color) {
    SkISize size = this->getISize();
    SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                              SkIntToScalar(size.height()));
    SkPaint paint;
    paint.setColor(color);
    canvas->drawRect(r, paint);
}

void GM::DrawGpuOnlyMessage(SkCanvas* canvas) {
    SkBitmap bmp;
    bmp.allocN32Pixels(128, 64);
    SkCanvas bmpCanvas(bmp);
    bmpCanvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(20);
    paint.setColor(SK_ColorRED);
    sk_tool_utils::set_portable_typeface(&paint);
    static const char kTxt[] = "GPU Only";
    bmpCanvas.drawText(kTxt, strlen(kTxt), 20, 40, paint);
    SkMatrix localM;
    localM.setRotate(35.f);
    localM.postTranslate(10.f, 0.f);
    SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(bmp, SkShader::kMirror_TileMode,
                                                               SkShader::kMirror_TileMode,
                                                               &localM));
    paint.setShader(shader);
    paint.setFilterQuality(kMedium_SkFilterQuality);
    canvas->drawPaint(paint);
    return;
}

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* GMRegistry::gHead;

void skiagm::SimpleGM::onDraw(SkCanvas* canvas) {
    fDrawProc(canvas);
}

SkISize skiagm::SimpleGM::onISize() {
    return fSize;
}

SkString skiagm::SimpleGM::onShortName() {
    return fName;
}

