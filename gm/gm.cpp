/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkFont.h"
#include "SkTraceEvent.h"
using namespace skiagm;

constexpr char GM::kDrawSkippedGPUOnly[];

GM::GM() {
    fMode = kGM_Mode;
    fBGColor = SK_ColorWHITE;
    fCanvasIsDeferred = false;
    fHaveCalledOnceBeforeDraw = false;
}

GM::~GM() {}

const char* GM::draw(SkCanvas* canvas) {
    TRACE_EVENT1("GM", TRACE_FUNC, "name", TRACE_STR_COPY(this->getName()));
    this->drawBackground(canvas);
    return this->drawContent(canvas);
}

const char* GM::drawContent(SkCanvas* canvas) {
    TRACE_EVENT0("GM", TRACE_FUNC);
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    SkAutoCanvasRestore acr(canvas, true);
    return this->onDraw(canvas);
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

void GM::DrawFailureMessage(SkCanvas* canvas, const char format[], ...)  {
    SkString failureMsg;

    va_list argp;
    va_start(argp, format);
    failureMsg.appendVAList(format, argp);
    va_end(argp);

    constexpr SkScalar kOffset = 5.0f;
    canvas->drawColor(SkColorSetRGB(200,0,0));
    SkFont font;
    SkRect bounds;
    font.measureText(failureMsg.c_str(), failureMsg.size(), kUTF8_SkTextEncoding, &bounds);
    SkPaint textPaint;
    textPaint.setColor(SK_ColorWHITE);
    canvas->drawString(failureMsg, kOffset, bounds.height() + kOffset, font, textPaint);
}

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* GMRegistry::gHead;

SimpleGM* SimpleGM::Create(
        const SkString& name, std::function<void(SkCanvas*)> simpleDrawProc, const SkISize& size,
        SkColor backgroundColor) {
    auto drawProc = [simpleDrawProc](SkCanvas* canvas) {
        simpleDrawProc(canvas);
        return kDrawComplete;
    };
    return new SimpleGM(name, std::move(drawProc), size, backgroundColor);
}

SimpleGM* SimpleGM::CreateSkippable(
        const SkString& name, std::function<const char*(SkCanvas*)> drawProc, const SkISize& size,
        SkColor backgroundColor) {
    return new SimpleGM(name, drawProc, size, backgroundColor);
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
