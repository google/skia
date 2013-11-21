/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
using namespace skiagm;

SkString GM::gResourcePath;

GM::GM() {
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

const char* GM::shortName() {
    if (fShortName.size() == 0) {
        fShortName = this->onShortName();
    }
    return fShortName.c_str();
}

void GM::setBGColor(SkColor color) {
    fBGColor = color;
}

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

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* SkTRegistry<GM*(*)(void*)>::gHead;
