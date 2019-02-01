/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkShader.h"
#include "SkTraceEvent.h"
using namespace skiagm;

GM::GM() {
    fMode = kGM_Mode;
    fBGColor = SK_ColorWHITE;
    fCanvasIsDeferred = false;
    fHaveCalledOnceBeforeDraw = false;
}

GM::~GM() {}

void GM::draw(SkCanvas* canvas) {
    TRACE_EVENT1("GM", TRACE_FUNC, "name", TRACE_STR_COPY(this->getName()));
    this->drawBackground(canvas);
    this->drawContent(canvas);
}

void GM::drawContent(SkCanvas* canvas) {
    TRACE_EVENT0("GM", TRACE_FUNC);
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    SkAutoCanvasRestore acr(canvas, true);
    this->onDraw(canvas);
}

void GM::drawBackground(SkCanvas* canvas) {
    TRACE_EVENT0("GM", TRACE_FUNC);
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    SkAutoCanvasRestore acr(canvas, true);
    canvas->drawColor(fBGColor, SkBlendMode::kSrc);
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
    SkFont font(sk_tool_utils::create_portable_typeface(), 20);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    bmpCanvas.drawString("GPU Only", 20, 40, font, paint);
    SkMatrix localM;
    localM.setRotate(35.f);
    localM.postTranslate(10.f, 0.f);
    paint.setShader(SkShader::MakeBitmapShader(bmp, SkShader::kMirror_TileMode,
                                               SkShader::kMirror_TileMode,
                                               &localM));
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

template <typename Fn>
static void mark(SkCanvas* canvas, SkScalar x, SkScalar y, Fn&& fn) {
    SkPaint alpha;
    alpha.setAlpha(0x50);
    canvas->saveLayer(nullptr, &alpha);
        canvas->translate(x,y);
        canvas->scale(2,2);
        fn();
    canvas->restore();
}

void MarkGMGood(SkCanvas* canvas, SkScalar x, SkScalar y) {
    mark(canvas, x,y, [&]{
        SkPaint paint;

        // A green circle.
        paint.setColor(SkColorSetRGB(27, 158, 119));
        canvas->drawCircle(0,0, 12, paint);

        // Cut out a check mark.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(-6, 0,
                         -1, 5, paint);
        canvas->drawLine(-1, +5,
                         +7, -5, paint);
    });
}

void MarkGMBad(SkCanvas* canvas, SkScalar x, SkScalar y) {
    mark(canvas, x,y, [&] {
        SkPaint paint;

        // A red circle.
        paint.setColor(SkColorSetRGB(231, 41, 138));
        canvas->drawCircle(0,0, 12, paint);

        // Cut out an 'X'.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(-5,-5,
                         +5,+5, paint);
        canvas->drawLine(+5,-5,
                         -5,+5, paint);
    });
}
