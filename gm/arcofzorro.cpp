/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

#define CROSS_INNER_RADIUS 3
#define CROSS_SIZE 18

void write_to_png(GrDirectContext* dContext, const char* name, SkImage* image) {
    SkBitmap bm;
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    bm.allocPixels(info);
    image->readPixels(dContext, info, bm.getPixels(), bm.rowBytes(), 0, 0);

    char filename[256];
    _snprintf(filename, 256, "c:\\src\\bugs\\%s.png", name);
    filename[255] = '\0';

    SkFILEWStream file(filename);
    SkAssertResult(file.isValid());

    SkAssertResult(SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100));
}

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    enum class Mode {
        kOld,
        kNewer,
        kNewest
    };

    ArcOfZorroGM(Mode mode, bool rot) : fMode(mode), fRot(rot) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        SkString str;
        str.printf("arcofzorro_%s%s",
                   fMode == Mode::kOld ? "old" : (fMode == Mode::kNewer ? "newer" : "newest"),
                   fRot ? "_rot" : "");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(200, 200);
    }

    /*
        https://source.corp.google.com/piper///depot/google3/java/com/google/android/apps/camera/coach/PitchRollIndicatorDrawer.java
        Norm: 802
        Rot:  728
        --------------- as lines
        Norm: 3
        Rot:  3
     */
    void drawCross1(float x, float y, SkPaint crossPaint, SkCanvas* canvas, float rotDeg) {
//        crossPaint.setMaskFilter(nullptr);

        canvas->save();
            canvas->rotate(rotDeg);
            canvas->drawLine(x - CROSS_SIZE / 2, y, x - CROSS_INNER_RADIUS, y, crossPaint);
            canvas->drawLine(x + CROSS_SIZE / 2, y, x + CROSS_INNER_RADIUS, y, crossPaint);
            canvas->drawLine(x, y - CROSS_SIZE / 2, x, y - CROSS_INNER_RADIUS, crossPaint);
            canvas->drawLine(x, y + CROSS_SIZE / 2, x, y + CROSS_INNER_RADIUS, crossPaint);
        canvas->restore();
    }

    /*
        Norm: 18
        Rot:  190
     */
    void drawCross2(float x, float y, SkPaint crossPaint, SkCanvas* canvas, float rotDeg) {
//        crossPaint.setMaskFilter(nullptr);

        float quantX = 0.25f * SkScalarRoundToScalar(4 * x);
        float quantY = 0.25f * SkScalarRoundToScalar(4 * y);

        canvas->save();
           canvas->rotate(rotDeg);
           canvas->translate(quantX, quantY);
           canvas->drawPath(fPath, crossPaint);
        canvas->restore();
    }

    void prepForDrawCross3(SkCanvas* canvas) {
        if (fCross3Image) {
            return;
        }

        SkPath strokedPath;

        // resolve the stroking
        SkAssertResult(fNormalPaint.getFillPath(fPath, &strokedPath));

        SkPaint blurNoStrokePaint;
        blurNoStrokePaint.setColor(SK_ColorBLACK);
        blurNoStrokePaint.setAntiAlias(true);
        blurNoStrokePaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.85085f));

        SkASSERT(blurNoStrokePaint.canComputeFastBounds());

        // compute the bloat due to blurring
        SkRect result;
        blurNoStrokePaint.computeFastBounds(strokedPath.getBounds(), &result);

        // integerize to get the size of the holding image
        SkIRect ir = result.roundOut();

        SkImageInfo ii = SkImageInfo::Make({ ir.width(), ir.height() },
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        sk_sp<SkSurface> s = canvas->makeSurface(ii);

        SkCanvas* c = s->getCanvas();

//        blurNoStrokePaint.setMaskFilter(nullptr);

        fDelta = { strokedPath.getBounds().centerX() - ir.fLeft,
                   strokedPath.getBounds().centerY() - ir.fTop };
        c->clear(SkColors::kTransparent);
        c->save();
            c->translate(fDelta.fX, fDelta.fY);
            c->drawPath(strokedPath, blurNoStrokePaint);
        c->restore();

        fCross3Image = s->makeImageSnapshot();

//        write_to_png(canvas->recordingContext()->asDirectContext(), "foo", fCross3Image.get());
    }

    /*
        Norm: 4
        Rot:  4
    */
    void drawCross3(float x, float y, SkCanvas* canvas, float rotDeg) {
        canvas->save();
           canvas->rotate(rotDeg);
           canvas->translate(x-fDelta.fX, y-fDelta.fY);
           canvas->drawImage(fCross3Image, 0, 0, { SkFilterMode::kLinear, SkMipmapMode::kNone });
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fNormalPaint.setColor(SK_ColorYELLOW);
        fNormalPaint.setStrokeWidth(3);
        fNormalPaint.setAntiAlias(true);
        fNormalPaint.setStyle(SkPaint::kStroke_Style);
        fNormalPaint.setStrokeCap(SkPaint::kRound_Cap);

        fShadowPaint.setColor(SK_ColorBLACK);
        fShadowPaint.setStrokeWidth(3);
        fShadowPaint.setAntiAlias(true);
        fShadowPaint.setStyle(SkPaint::kStroke_Style);
        fShadowPaint.setStrokeCap(SkPaint::kRound_Cap);
        fShadowPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.85085f));

        fAltShadowPaint.setColor(SK_ColorBLACK);
        fAltShadowPaint.setStrokeWidth(3);
        fAltShadowPaint.setAntiAlias(true);
        fAltShadowPaint.setStyle(SkPaint::kStroke_Style);
        fAltShadowPaint.setStrokeCap(SkPaint::kSquare_Cap);
        fAltShadowPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.85085f));

        fPath.moveTo(-CROSS_SIZE / 2, 0);
        fPath.lineTo(-CROSS_INNER_RADIUS, 0);
        fPath.moveTo(CROSS_SIZE / 2, 0);
        fPath.lineTo(CROSS_INNER_RADIUS, 0);
        fPath.moveTo(0, -CROSS_SIZE / 2);
        fPath.lineTo(0, -CROSS_INNER_RADIUS);
        fPath.moveTo(0, CROSS_SIZE / 2);
        fPath.lineTo(0, CROSS_INNER_RADIUS);
        fPath.setIsVolatile(false);
    }

    /*
        w/ caching:
                            Entry Count: current 804 (804 budgeted, 0 wrapped, 800 locked, 804 scratch), high 804
                            Entry Bytes: current 3503936 (budgeted 3503936, 1.3% full, 0 unbudgeted) high 3503936
            Textures Created: 802
            Texture Uploads: 802

        w/o caching:
                            Entry Count: current 8 (8 budgeted, 0 wrapped, 0 locked, 8 scratch), high 8
                            Entry Bytes: current 243520 (budgeted 243520, 0.091% full, 0 unbudgeted) high 243520
            Textures Created: 6
            Texture Uploads: 802
    */
    void onDraw(SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());

        ToolUtils::draw_checkerboard(canvas, 0xffffffff, 0xffc6c3c6, 10);

#if 1
        SkPaint shadowP;
        shadowP.setColor(SK_ColorRED);
        shadowP.setStrokeWidth(5.0f);
        shadowP.setAntiAlias(true);
        shadowP.setAlpha(0xff);
        shadowP.setStyle(SkPaint::kStroke_Style);
        shadowP.setStrokeCap(SkPaint::kRound_Cap);
        shadowP.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));
#endif

        SkRandom random;
        for (int i = 0; i < 200; ++i) {
#if 1
            float centerX = random.nextRangeF(CROSS_SIZE/2.0f, 200-CROSS_SIZE/2.0f);
            float centerY = random.nextRangeF(CROSS_SIZE/2.0f, 200-CROSS_SIZE/2.0f);
            float startX = centerX - CROSS_SIZE/2.0f + random.nextF();
            float stopX = centerX + CROSS_SIZE/2.0f + random.nextF();
            float startY = centerY + random.nextF();
            float stopY = centerY + random.nextF();

            canvas->drawLine(startX, startY, stopX, stopY, shadowP);
#else
            float x = random.nextRangeF(CROSS_SIZE, 200-CROSS_SIZE);
            float y = random.nextRangeF(CROSS_SIZE, 200-CROSS_SIZE);
            float rotDeg = random.nextRangeF(-45, 45);
            if (!fRot) {
                rotDeg = 0.0f;
            }

            if (fMode == Mode::kOld) {
                this->drawCross1(x + 3, y + 3, /* fAltShadowPaint */ fShadowPaint, canvas, rotDeg);
                this->drawCross1(x, y, fNormalPaint, canvas, rotDeg);
            } else if (fMode == Mode::kNewer) {
                this->drawCross2(x + 3, y + 3, fShadowPaint, canvas, rotDeg);
                this->drawCross1(x, y, fNormalPaint, canvas, rotDeg);
            } else {
                this->prepForDrawCross3(canvas);

                this->drawCross3(x + 3, y + 3, canvas, rotDeg);
                this->drawCross1(x, y, fNormalPaint, canvas, rotDeg);
            }
#endif

            dContext->flush();
        }
    }

private:
    SkPaint        fAltShadowPaint;
    SkPaint        fShadowPaint;
    SkPaint        fNormalPaint;

    SkPath         fPath;
    Mode           fMode;
    bool           fRot;

    SkPoint        fDelta;
    sk_sp<SkImage> fCross3Image;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

//DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kOld, true);)
//DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kNewer, true);)
//DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kNewest, true);)
DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kOld, false);)
//DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kNewer, false);)
//DEF_GM(return new ArcOfZorroGM(ArcOfZorroGM::Mode::kNewest, false);)
}  // namespace skiagm
