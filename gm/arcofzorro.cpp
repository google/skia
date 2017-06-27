/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkModeColorFilter.h"
#include "SkImage.h"
#include "SkPath.h"

#include "Resources.h"

static const int kSkipped = 0;
static const int kNumSteps = 3;
static const int kSize = 128 / kNumSteps;
static const int kPad = 3;
static const int kSmSize = kSize - 2 * kPad;

static SkPath draw_path() {
    SkPath p;
    p.moveTo(kSmSize/2.0f, 2.0f*kPad);
    p.lineTo(kSmSize-2.0f*kPad, kSmSize-2.0f*kPad);
    p.lineTo(2.0f*kPad, kSmSize-2.0f*kPad);
    p.close();    
    return p;
}

static SkPath clip_path() {
    SkPath p;
    p.moveTo(kSmSize/2.0f, kPad);
    p.lineTo(kSmSize-kPad, kSmSize-kPad);
    p.lineTo(kPad, kSmSize-kPad);
    p.close();

    p.moveTo(kSmSize/2.0f+kPad, kPad-kPad);
    p.lineTo(kSmSize-kPad+kPad, kSmSize-kPad-kPad);
    p.lineTo(kPad+kPad, kSmSize-kPad-kPad);
    p.close();

    return p;
}

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
       return SkISize::Make(128, 128);
    }

    void onOnceBeforeDraw() override {
#if 0
        for (int y = 0; y < kNumSteps; ++y) {
            for (int x = 0; x < kNumSteps; ++x) {
                fImages1[x][y] = GetResourceAsImage("mandrill_128.png");
            }
        }
#endif

        fImage = GetResourceAsImage("mandrill_128.png");
        fDrawPath = draw_path();
        fClipPath = clip_path();
    }

    void draw(SkCanvas* canvas, sk_sp<SkImage> img, const SkIPoint& xlate, const SkIRect& src, bool drawPaths) {
        const SkRect dst = SkRect::MakeWH(src.width(), src.height());

        canvas->save();
            canvas->translate(xlate.fX, xlate.fY);
            canvas->clipRect(dst, false);
            if (drawPaths) {
                SkPaint paint;
                paint.setColor(0x61000000);
                paint.setAntiAlias(true);
                canvas->drawPath(fDrawPath, paint);
                canvas->clipPath(fClipPath, kDifference_SkClipOp, false);

                SkPaint imagePaint;
//                imagePaint.setColorFilter(SkModeColorFilter::Make(0x61000000, SkBlendMode::kSrcIn));
                imagePaint.setAntiAlias(false);
                canvas->drawImageRect(img, src, dst, &imagePaint);
            } else {
                canvas->drawImageRect(img, src, dst, nullptr);
            }
        canvas->restore();    
    }

    void onDraw(SkCanvas* canvas) override {

        int count = 0;
        for (int y = 0; y < 1; ++y) {// kNumSteps; ++y) {
            for (int x = 0; x < 2; ++x) { //kNumSteps; ++x) {
                SkIRect src = SkIRect::MakeXYWH(x*kSize, y*kSize, kSize, kSize);
                src.inset(kPad, kPad);

                this->draw(canvas, fImage, SkIPoint::Make(x*kSize+kPad, y*kSize+kPad), src, true); //kSkipped != count);
                ++count;
            }
        }
    }

private:
#if 0
    sk_sp<SkImage> fImages1[kNumSteps][kNumSteps];
#endif

    sk_sp<SkImage> fImage;
    SkPath fDrawPath;
    SkPath fClipPath;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
