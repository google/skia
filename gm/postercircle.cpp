/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/timer/AnimTimer.h"

// Mimics https://output.jsbin.com/falefice/1/quiet?CC_POSTER_CIRCLE, which can't be captured as
// an SKP due to many 3D layers being composited post-SKP capture.
// See skbug.com/9028
class PosterCircleGM : public skiagm::GM {
public:
    PosterCircleGM() : fTime(0.f) {}

protected:

    SkString onShortName() override {
        return SkString("poster_circle");
    }

    SkISize onISize() override {
        return SkISize::Make(kStageWidth, kStageHeight + 50);
    }

    bool onAnimate(const AnimTimer& timer) override {
#if 0
        if (timer.isRunning()) {
            // Use a fixed timestep
            fTime += 5e-4f;
        }
#else
        fTime = timer.scaled(0.5f);
#endif
        return true;
    }

    void onOnceBeforeDraw() override {
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setEmbolden(true);
        font.setSize(24.f);

        sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(kPosterSize, kPosterSize);
        for (int i = 0; i < kNumAngles; ++i) {
            SkCanvas* canvas = surface->getCanvas();

            SkPaint fillPaint;
            fillPaint.setAntiAlias(true);
            fillPaint.setColor(i % 2 == 0 ? SkColorSetRGB(0x99, 0x5C, 0x7F)
                                          : SkColorSetRGB(0x83, 0x5A, 0x99));
            canvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(kPosterSize, kPosterSize),
                                                  10.f, 10.f), fillPaint);

            SkString label;
            label.printf("%d", i);
            SkRect labelBounds;
            font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &labelBounds);
            SkScalar labelX = 0.5f * kPosterSize - 0.5f * labelBounds.width();
            SkScalar labelY = 0.5f * kPosterSize + 0.5f * labelBounds.height();


            SkPaint labelPaint;
            labelPaint.setAntiAlias(true);
            canvas->drawString(label, labelX, labelY, font, labelPaint);

            fPosterImages[i] = surface->makeImageSnapshot();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // See https://developer.mozilla.org/en-US/docs/Web/CSS/transform-function/perspective
        // for projection matrix when --webkit-perspective: 800px is used.
        SkMatrix44 proj(SkMatrix44::kIdentity_Constructor);
        proj.set(3, 2, -1.f / 800.f);

        for (int pass = 0; pass < 2; ++pass) {
            // Want to draw 90 to 270 first (the back), then 270 to 90 (the front), but do all 3
            // rings backsides, then their frontsides since the front projections overlap across
            // rings. Note: we skip the poster circle's x axis rotation because that complicates the
            // back-to-front drawing order and it isn't necessary to trigger draws aligned with Z.
            bool drawFront = pass > 0;

            for (int y = 0; y < 3; ++y) {
                float ringY = (y - 1) * (kPosterSize + 10.f);
                for (int i = 0; i < kNumAngles; ++i) {
                    // Add an extra 45 degree rotation, which triggers the bug by aligning some of
                    // the posters with the z axis.
                    SkScalar yDuration = 5.f - y;
                    SkScalar yRotation = SkScalarMod(kAngleStep * i +
                            360.f * SkScalarMod(fTime / yDuration, yDuration), 360.f);
                    // These rotation limits were chosen manually to line up with current projection
                    static constexpr SkScalar kBackMinAngle = 70.f;
                    static constexpr SkScalar kBackMaxAngle = 290.f;
                    if (drawFront) {
                        if (yRotation >= kBackMinAngle && yRotation <= kBackMaxAngle) {
                            // Back portion during a front draw
                            continue;
                        }
                    } else {
                        if (yRotation < kBackMinAngle || yRotation > kBackMaxAngle) {
                            // Front portion during a back draw
                            continue;
                        }
                    }

                    canvas->save();

                    // Matrix matches transform: rotateY(<angle>deg) translateZ(200px); nested in an
                    // element with the perspective projection matrix above.
                    SkMatrix44 model;
                    // No post/preRotate, so start with rotation matrix and adjust from there
                    model.setRotateAboutUnit(0.f, 1.f, 0.f, SkDegreesToRadians(yRotation));
                    model.preTranslate(0.f, 0.f, kRingRadius); // *before* rotation
                    model.postTranslate(0.f, ringY, 0.f);      // *after* rotation
                    model.postConcat(proj);
                    model.postTranslate(0.5f * kStageWidth, 0.5f * kStageHeight + 25, 0.f);

                    // Flatten the 4x4 matrix by discarding the 3rd row and column
                    canvas->concat(SkMatrix::MakeAll(
                            model.get(0, 0), model.get(0, 1), model.get(0, 3),
                            model.get(1, 0), model.get(1, 1), model.get(1, 3),
                            model.get(3, 0), model.get(3, 1), model.get(3, 3)));

                    SkRect poster = SkRect::MakeLTRB(-0.5f * kPosterSize, -0.5f * kPosterSize,
                                                      0.5f * kPosterSize,  0.5f * kPosterSize);
                    SkPaint fillPaint;
                    fillPaint.setAntiAlias(true);
                    fillPaint.setAlphaf(0.7f);
                    fillPaint.setFilterQuality(kLow_SkFilterQuality);
                    canvas->drawImageRect(fPosterImages[i], poster, &fillPaint);

                    canvas->restore();
                }
            }
        }
    }

private:
    static const int kAngleStep = 30;
    static const int kNumAngles = 12; // 0 through 330 degrees

    static const int kStageWidth = 600;
    static const int kStageHeight = 400;
    static const int kRingRadius = 200;
    static const int kPosterSize = 100;

    sk_sp<SkImage> fPosterImages[kNumAngles];
    SkScalar fTime;
};

DEF_GM(return new PosterCircleGM();)
