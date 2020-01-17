/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkTraceEvent.h"
#include "tools/ToolUtils.h"

#include <stdarg.h>

class GrRenderTargetContext;

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
    font.measureText(failureMsg.c_str(), failureMsg.size(), SkTextEncoding::kUTF8, &bounds);
    SkPaint textPaint(SkColors::kWhite);
    canvas->drawString(failureMsg, kOffset, bounds.height() + kOffset, font, textPaint);
}

static void draw_gpu_only_message(SkCanvas* canvas) {
    SkBitmap bmp;
    bmp.allocN32Pixels(128, 64);
    SkCanvas bmpCanvas(bmp);
    bmpCanvas.drawColor(SK_ColorWHITE);
    SkFont  font(ToolUtils::create_portable_typeface(), 20);
    SkPaint paint(SkColors::kRed);
    bmpCanvas.drawString("GPU Only", 20, 40, font, paint);
    SkMatrix localM;
    localM.setRotate(35.f);
    localM.postTranslate(10.f, 0.f);
    paint.setShader(bmp.makeShader(SkTileMode::kMirror, SkTileMode::kMirror, &localM));
    paint.setFilterQuality(kMedium_SkFilterQuality);
    canvas->drawPaint(paint);
}


// ~~~~~~~~~~~~~~~~~~ Verifier ~~~~~~~~~~~~~~~~~~
GMVerifier::~GMVerifier() {}

VerifierResult GMVerifier::verify(const SkBitmap& gold, const SkBitmap& actual) {
    return defaultVerify(gold, actual);
}

const SkBitmap* GMVerifier::goldStageImg() const {
    return fGoldStageImg.get();
}

const SkBitmap* GMVerifier::actualStageImg() const {
    return fActualStageImg.get();
}

VerifierResult GMVerifier::defaultVerify(const SkBitmap& gold, const SkBitmap& actual) {
    fGoldStageImg = mask(gold);
    fActualStageImg = duplicate(actual);
    if (!allPixelsAreInMask(*fActualStageImg, *fGoldStageImg))
        return VerifierResult::kFail;

    return VerifierResult::kOk;
}

std::unique_ptr<SkBitmap> GMVerifier::duplicate(const SkBitmap& bmp) {
    const SkISize input_size = bmp.info().bounds().size();
    const SkImageInfo image_info = bmp.info();

    std::unique_ptr<SkBitmap> result(new SkBitmap);
    result->allocPixelsFlags(image_info, SkBitmap::kZeroPixels_AllocFlag);

    const SkPixmap& pixmap = bmp.pixmap();
    for (int y = 0; y < input_size.fHeight; y++) {
        for (int x = 0; x < input_size.fWidth; x++) {
            *result->getAddr32(x, y) = *pixmap.addr32(x, y);
        }
    }

    return result;
}

std::unique_ptr<SkBitmap> GMVerifier::mask(const SkBitmap& bmp) {
    const SkISize input_size = bmp.info().bounds().size();
    const SkImageInfo
        image_info = SkImageInfo::Make(input_size, bmp.colorType(), bmp.alphaType(), nullptr);

    std::unique_ptr<SkBitmap> result(new SkBitmap);
    result->allocPixelsFlags(image_info, SkBitmap::kZeroPixels_AllocFlag);

    SkCanvas canvas(*result);
    canvas.clear(SK_ColorWHITE);

    const SkPixmap& pixmap = bmp.pixmap();
    for (int y = 0; y < input_size.fHeight; y++) {
        for (int x = 0; x < input_size.fWidth; x++) {
            SkColor c = pixmap.getColor(x, y);
            if (c != SK_ColorWHITE && SkColorGetA(c) > 0) {
                *result->getAddr32(x, y) = SK_ColorBLACK;
            }
        }
    }

    return result;
}

bool GMVerifier::bitmapsEqual(const SkBitmap& a, const SkBitmap& b) {
    const SkISize a_size = a.info().bounds().size();
    const SkISize b_size = b.info().bounds().size();

    if (!a_size.equals(b_size.fWidth, b_size.fHeight))
        return false;

    const SkPixmap& a_pixmap = a.pixmap();
    const SkPixmap& b_pixmap = b.pixmap();
    for (int y = 0; y < b_size.fHeight; y++) {
        for (int x = 0; x < b_size.fWidth; x++) {
            if (b_pixmap.getColor(x, y) != a_pixmap.getColor(x, y)) {
                return false;
            }
        }
    }

    return true;
}

bool GMVerifier::allPixelsAreInMask(SkBitmap& a, SkBitmap& mask) {
    const SkColor background_color = SK_ColorWHITE;
    const SkISize a_size = a.info().bounds().size();
    const SkISize mask_size = mask.info().bounds().size();

    const uint8_t color_dist_threshold = 16;
    const uint32_t failed_pixel_threshold = static_cast<uint32_t>(0.01 * a_size.area());

    if (!a_size.equals(mask_size.fWidth, mask_size.fHeight)) {
        return false;
    }

    uint32_t num_failed_pixels = 0;
    for (int y = 0; y < mask_size.fHeight; y++) {
        for (int x = 0; x < mask_size.fWidth; x++) {
            const SkColor c = a.getColor(x, y);
            const uint32_t distance_from_bg = colorDist(c, background_color);
            if (distance_from_bg > color_dist_threshold &&
                !colorInNeighborhood(mask, x, y, SK_ColorBLACK)) {
                // Test image drew a pixel that gold image did not (approximately).
                num_failed_pixels++;
                a.erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    return num_failed_pixels < failed_pixel_threshold;
}

bool GMVerifier::colorInNeighborhood(const SkBitmap& bitmap, int x, int y, SkColor color, int n) {
    const SkIRect bounds = bitmap.bounds();
    const int minX = std::max(x - n, bounds.fLeft),
        maxX = std::min(x + n, bounds.fRight - 1),
        minY = std::max(y - n, bounds.fTop),
        maxY = std::min(y + n, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (color == bitmap.getColor(j, i)) {
                return true;
            }
        }
    }

    return false;
}

uint32_t GMVerifier::colorDist(SkColor a, SkColor b) {
    const auto abs_delta = [](uint8_t x, uint8_t y) {
        return x < y ? y - x : x - y;
    };

    const uint8_t a_a = SkColorGetA(a),
        a_r = SkColorGetR(a),
        a_g = SkColorGetG(a),
        a_b = SkColorGetB(a);
    const uint8_t b_a = SkColorGetA(b),
        b_r = SkColorGetR(b),
        b_g = SkColorGetG(b),
        b_b = SkColorGetB(b);

    const uint32_t da = abs_delta(a_a, b_a);
    const uint32_t dr = abs_delta(a_r, b_r);
    const uint32_t dg = abs_delta(a_g, b_g);
    const uint32_t db = abs_delta(a_b, b_b);

    return da + dr + dg + db;
}

// ~~~~~~~~~~~~~~~~~~ End Verifier ~~~~~~~~~~~~~~~~~~

GM::GM(SkColor bgColor) {
    fMode = kGM_Mode;
    fBGColor = bgColor;
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

bool GM::animate(double nanos) { return this->onAnimate(nanos); }

bool GM::runAsBench() const { return false; }
void GM::modifyGrContextOptions(GrContextOptions* options) {}

std::unique_ptr<GMVerifier> GM::getVerifier() {
    return std::unique_ptr<GMVerifier>(new GMVerifier);
}

void GM::onOnceBeforeDraw() {}

bool GM::onAnimate(double /*nanos*/) { return false; }

bool GM::onChar(SkUnichar uni) { return false; }

bool GM::onGetControls(SkMetaData*) { return false; }

void GM::onSetControls(const SkMetaData&) {}

/////////////////////////////////////////////////////////////////////////////////////////////

void GM::drawSizeBounds(SkCanvas* canvas, SkColor color) {
    canvas->drawRect(SkRect::Make(this->getISize()), SkPaint(SkColor4f::FromColor(color)));
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
        // A green circle.
        canvas->drawCircle(0, 0, 12, SkPaint(SkColor4f::FromColor(SkColorSetRGB(27, 158, 119))));

        // Cut out a check mark.
        SkPaint paint(SkColors::kTransparent);
        paint.setBlendMode(SkBlendMode::kSrc);
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
        // A red circle.
        canvas->drawCircle(0,0, 12, SkPaint(SkColor4f::FromColor(SkColorSetRGB(231, 41, 138))));

        // Cut out an 'X'.
        SkPaint paint(SkColors::kTransparent);
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(-5,-5,
                         +5,+5, paint);
        canvas->drawLine(+5,-5,
                         -5,+5, paint);
    });
}
