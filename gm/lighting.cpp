/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkAnimTimer.h"
#include "SkLightingImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPoint3.h"

#define WIDTH 330
#define HEIGHT 660

namespace skiagm {

class ImageLightingGM : public GM {
public:
    ImageLightingGM()
        : fAzimuth(SkIntToScalar(kStartAzimuth)) {
        this->setBGColor(0xFF000000);
    }

protected:

    SkString onShortName() override {
        return SkString("lighting");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(
          SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fBitmap = sk_tool_utils::create_string_bitmap(100, 100, 0xFFFFFFFF, 20, 70, 96, "e");
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(sk_tool_utils::color_to_565(0xFF101010));
        SkPaint checkPaint;
        checkPaint.setColor(sk_tool_utils::color_to_565(0xFF202020));
        for (int y = 0; y < HEIGHT; y += 16) {
          for (int x = 0; x < WIDTH; x += 16) {
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), checkPaint);
            canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), checkPaint);
            canvas->restore();
          }
        }
        SkScalar cosAzimuth;
        SkScalar sinAzimuth = SkScalarSinCos(SkDegreesToRadians(fAzimuth), &cosAzimuth);

        SkPoint3 spotTarget = SkPoint3::Make(SkIntToScalar(40), SkIntToScalar(40), 0);
        SkPoint3 spotLocation = SkPoint3::Make(spotTarget.fX + 70.7214f * cosAzimuth,
                                               spotTarget.fY + 70.7214f * sinAzimuth,
                                               spotTarget.fZ + SkIntToScalar(20));
        SkScalar spotExponent = SK_Scalar1;

        SkPoint3 pointLocation = SkPoint3::Make(spotTarget.fX + 50 * cosAzimuth,
                                                spotTarget.fY + 50 * sinAzimuth,
                                                SkIntToScalar(10));
        SkScalar elevationRad = SkDegreesToRadians(SkIntToScalar(5));

        SkPoint3 distantDirection = SkPoint3::Make(cosAzimuth * SkScalarCos(elevationRad),
                                                   sinAzimuth * SkScalarCos(elevationRad),
                                                   SkScalarSin(elevationRad));
        SkScalar cutoffAngle = SkIntToScalar(15);
        SkScalar kd = SkIntToScalar(2);
        SkScalar ks = SkIntToScalar(1);
        SkScalar shininess = SkIntToScalar(8);
        SkScalar surfaceScale = SkIntToScalar(1);
        SkColor white(0xFFFFFFFF);
        SkPaint paint;

        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(20, 10, 60, 65));
        SkImageFilter::CropRect fullSizeCropRect(SkRect::MakeXYWH(0, 0, 100, 100));
        sk_sp<SkImageFilter> noopCropped(SkOffsetImageFilter::Make(0, 0, nullptr, &cropRect));

        int y = 0;
        for (int i = 0; i < 3; i++) {
            const SkImageFilter::CropRect* cr = (i == 1) ? &cropRect : (i == 2) ? &fullSizeCropRect : nullptr;
            sk_sp<SkImageFilter> input = (i == 2) ? noopCropped : nullptr;
            paint.setImageFilter(SkLightingImageFilter::MakePointLitDiffuse(pointLocation,
                                                                            white,
                                                                            surfaceScale,
                                                                            kd,
                                                                            input,
                                                                            cr));
            drawClippedBitmap(canvas, paint, 0, y);

            paint.setImageFilter(SkLightingImageFilter::MakeDistantLitDiffuse(distantDirection,
                                                                              white,
                                                                              surfaceScale,
                                                                              kd,
                                                                              input,
                                                                              cr));
            drawClippedBitmap(canvas, paint, 110, y);

            paint.setImageFilter(SkLightingImageFilter::MakeSpotLitDiffuse(spotLocation,
                                                                           spotTarget,
                                                                           spotExponent,
                                                                           cutoffAngle,
                                                                           white,
                                                                           surfaceScale,
                                                                           kd,
                                                                           input,
                                                                           cr));
            drawClippedBitmap(canvas, paint, 220, y);

            y += 110;

            paint.setImageFilter(SkLightingImageFilter::MakePointLitSpecular(pointLocation,
                                                                             white,
                                                                             surfaceScale,
                                                                             ks,
                                                                             shininess,
                                                                             input,
                                                                             cr));
            drawClippedBitmap(canvas, paint, 0, y);

            paint.setImageFilter(SkLightingImageFilter::MakeDistantLitSpecular(distantDirection,
                                                                               white,
                                                                               surfaceScale,
                                                                               ks,
                                                                               shininess,
                                                                               input,
                                                                               cr));
            drawClippedBitmap(canvas, paint, 110, y);

            paint.setImageFilter(SkLightingImageFilter::MakeSpotLitSpecular(spotLocation,
                                                                            spotTarget,
                                                                            spotExponent,
                                                                            cutoffAngle,
                                                                            white,
                                                                            surfaceScale,
                                                                            ks,
                                                                            shininess,
                                                                            input,
                                                                            cr));
            drawClippedBitmap(canvas, paint, 220, y);

            y += 110;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        constexpr SkScalar kDesiredDurationSecs = 15.0f;

        fAzimuth = kStartAzimuth + timer.scaled(360.0f/kDesiredDurationSecs, 360.0f);
        return true;
    }

private:
    static constexpr int kStartAzimuth = 225;

    SkBitmap fBitmap;
    SkScalar fAzimuth;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageLightingGM;)
}
