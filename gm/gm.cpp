/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkShader.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkTraceEvent.h"
#include "tools/ToolUtils.h"
using namespace skiagm;

constexpr char GM::kErrorMsg_DrawSkippedGpuOnly[];

static void draw_failure_message(SkCanvas* canvas, const char format[], ...)  {
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

static void draw_gpu_only_message(SkCanvas* canvas) {
    SkBitmap bmp;
    bmp.allocN32Pixels(128, 64);
    SkCanvas bmpCanvas(bmp);
    bmpCanvas.drawColor(SK_ColorWHITE);
    SkFont  font(ToolUtils::create_portable_typeface(), 20);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    bmpCanvas.drawString("GPU Only", 20, 40, font, paint);
    SkMatrix localM;
    localM.setRotate(35.f);
    localM.postTranslate(10.f, 0.f);
    paint.setShader(bmp.makeShader(SkTileMode::kMirror, SkTileMode::kMirror, &localM));
    paint.setFilterQuality(kMedium_SkFilterQuality);
    canvas->drawPaint(paint);
}

GM::GM(SkColor bgColor) {
    fMode = kGM_Mode;
    fBGColor = bgColor;
    fCanvasIsDeferred = false;
    fHaveCalledOnceBeforeDraw = false;
}

GM::~GM() {}

DrawResult GM::draw(SkCanvas* canvas, SkString* errorMsg) {
    TRACE_EVENT1("GM", TRACE_FUNC, "name", TRACE_STR_COPY(this->getName()));
    this->drawBackground(canvas);
    return this->drawContent(canvas, errorMsg);
}

DrawResult GM::drawContent(SkCanvas* canvas, SkString* errorMsg) {
    TRACE_EVENT0("GM", TRACE_FUNC);
    if (!fHaveCalledOnceBeforeDraw) {
        fHaveCalledOnceBeforeDraw = true;
        this->onOnceBeforeDraw();
    }
    SkAutoCanvasRestore acr(canvas, true);
    DrawResult drawResult = this->onDraw(canvas, errorMsg);
    if (DrawResult::kOk != drawResult) {
        if (DrawResult::kFail == drawResult) {
            draw_failure_message(canvas, "DRAW FAILED: %s", errorMsg->c_str());
        } else if (SkString(kErrorMsg_DrawSkippedGpuOnly) == *errorMsg) {
            draw_gpu_only_message(canvas);
        } else {
            draw_failure_message(canvas, "DRAW SKIPPED: %s", errorMsg->c_str());
        }
    }
    return drawResult;
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

DrawResult GM::onDraw(SkCanvas* canvas, SkString* errorMsg) {
    this->onDraw(canvas);
    return DrawResult::kOk;
}
void GM::onDraw(SkCanvas*) { SK_ABORT("Not implemented."); }


SkISize SimpleGM::onISize() { return fSize; }
SkString SimpleGM::onShortName() { return fName; }
DrawResult SimpleGM::onDraw(SkCanvas* canvas, SkString* errorMsg) {
    return fDrawProc(canvas, errorMsg);
}

SkISize SimpleGpuGM::onISize() { return fSize; }
SkString SimpleGpuGM::onShortName() { return fName; }
DrawResult SimpleGpuGM::onDraw(GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas,
                               SkString* errorMsg) {
    return fDrawProc(ctx, rtc, canvas, errorMsg);
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

bool GM::animate(const AnimTimer& timer) { return this->onAnimate(timer); }

bool GM::runAsBench() const { return false; }
void GM::modifyGrContextOptions(GrContextOptions* options) {}

void GM::onOnceBeforeDraw() {}

bool GM::onAnimate(const AnimTimer&) { return false; }
bool GM::onHandleKey(SkUnichar uni) { return false; }
bool GM::onGetControls(SkMetaData*) { return false; }
void GM::onSetControls(const SkMetaData&) {}

/////////////////////////////////////////////////////////////////////////////////////////////

void GM::drawSizeBounds(SkCanvas* canvas, SkColor color) {
    SkISize size = this->getISize();
    SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                              SkIntToScalar(size.height()));
    SkPaint paint;
    paint.setColor(color);
    canvas->drawRect(r, paint);
}

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* GMRegistry::gHead;

DrawResult GpuGM::onDraw(GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas,
                          SkString* errorMsg) {
    this->onDraw(ctx, rtc, canvas);
    return DrawResult::kOk;
}
void GpuGM::onDraw(GrContext*, GrRenderTargetContext*, SkCanvas*) {
    SK_ABORT("Not implemented.");
}

DrawResult GpuGM::onDraw(SkCanvas* canvas, SkString* errorMsg) {
    GrContext* ctx = canvas->getGrContext();
    GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
    if (!ctx || !rtc) {
        *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }
    if (ctx->abandoned()) {
        *errorMsg = "GrContext abandoned.";
        return DrawResult::kSkip;
    }
    return this->onDraw(ctx, rtc, canvas, errorMsg);
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
