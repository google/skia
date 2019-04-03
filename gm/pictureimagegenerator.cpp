/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkFontPriv.h"
#include "SkGradientShader.h"
#include "SkImageGenerator.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkTextUtils.h"
#include "ToolUtils.h"
#include "gm.h"

static void draw_vector_logo(SkCanvas* canvas, const SkRect& viewBox) {
    constexpr char kSkiaStr[] = "SKIA";
    constexpr SkScalar kGradientPad = .1f;
    constexpr SkScalar kVerticalSpacing = 0.25f;
    constexpr SkScalar kAccentScale = 1.20f;

    SkPaint paint;
    paint.setAntiAlias(true);

    SkFont font(ToolUtils::create_portable_typeface());
    font.setSubpixel(true);
    font.setEmbolden(true);

    SkPath path;
    SkRect iBox, skiBox, skiaBox;
    SkTextUtils::GetPath("SKI", 3, kUTF8_SkTextEncoding, 0, 0, font, &path);
    TightBounds(path, &skiBox);
    SkTextUtils::GetPath("I", 1, kUTF8_SkTextEncoding, 0, 0, font, &path);
    TightBounds(path, &iBox);
    iBox.offsetTo(skiBox.fRight - iBox.width(), iBox.fTop);

    const size_t textLen = strlen(kSkiaStr);
    SkTextUtils::GetPath(kSkiaStr, textLen, kUTF8_SkTextEncoding, 0, 0, font, &path);
    TightBounds(path, &skiaBox);
    skiaBox.outset(0, 2 * iBox.width() * (kVerticalSpacing + 1));

    const SkScalar accentSize = iBox.width() * kAccentScale;
    const SkScalar underlineY = iBox.bottom() +
        (kVerticalSpacing + SkScalarSqrt(3) / 2) * accentSize;
    SkMatrix m;
    m.setRectToRect(skiaBox, viewBox, SkMatrix::kFill_ScaleToFit);
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(m);

    canvas->drawCircle(iBox.centerX(),
                       iBox.y() - (0.5f + kVerticalSpacing) * accentSize,
                       accentSize / 2,
                       paint);

    path.reset();
    path.moveTo(iBox.centerX() - accentSize / 2, iBox.bottom() + kVerticalSpacing * accentSize);
    path.rLineTo(accentSize, 0);
    path.lineTo(iBox.centerX(), underlineY);
    canvas->drawPath(path, paint);

    SkRect underlineRect = SkRect::MakeLTRB(iBox.centerX() - iBox.width() * accentSize * 3,
                                            underlineY,
                                            iBox.centerX(),
                                            underlineY + accentSize / 10);
    const SkPoint pts1[] = { SkPoint::Make(underlineRect.x(), 0),
                             SkPoint::Make(iBox.centerX(), 0) };
    const SkScalar pos1[] = { 0, 0.75f };
    const SkColor colors1[] = { SK_ColorTRANSPARENT, SK_ColorBLACK };
    SkASSERT(SK_ARRAY_COUNT(pos1) == SK_ARRAY_COUNT(colors1));
    paint.setShader(SkGradientShader::MakeLinear(pts1, colors1, pos1, SK_ARRAY_COUNT(pos1),
                                                 SkTileMode::kClamp));
    canvas->drawRect(underlineRect, paint);

    const SkPoint pts2[] = { SkPoint::Make(iBox.x() - iBox.width() * kGradientPad, 0),
                             SkPoint::Make(iBox.right() + iBox.width() * kGradientPad, 0) };
    const SkScalar pos2[] = { 0, .01f, 1.0f/3, 1.0f/3, 2.0f/3, 2.0f/3, .99f, 1 };
    const SkColor colors2[] = {
        SK_ColorBLACK,
        0xffca5139,
        0xffca5139,
        0xff8dbd53,
        0xff8dbd53,
        0xff5460a5,
        0xff5460a5,
        SK_ColorBLACK
    };
    SkASSERT(SK_ARRAY_COUNT(pos2) == SK_ARRAY_COUNT(colors2));
    paint.setShader(SkGradientShader::MakeLinear(pts2, colors2, pos2, SK_ARRAY_COUNT(pos2),
                                                 SkTileMode::kClamp));
    canvas->drawSimpleText(kSkiaStr, textLen, kUTF8_SkTextEncoding, 0, 0, font, paint);
}

// This GM exercises SkPictureImageGenerator features
// (in particular its matrix vs. bounds semantics).
class PictureGeneratorGM : public skiagm::GM {
protected:
    SkString onShortName() override {
        return SkString("pictureimagegenerator");
    }

    SkISize onISize() override {
        return SkISize::Make(1160, 860);
    }

    void onOnceBeforeDraw() override {
        const SkRect rect = SkRect::MakeWH(kPictureWidth, kPictureHeight);
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(rect);
        draw_vector_logo(canvas, rect);
        fPicture = recorder.finishRecordingAsPicture();
    }

    void onDraw(SkCanvas* canvas) override {
        const struct {
            SkISize  size;
            SkScalar scaleX, scaleY;
            SkScalar opacity;
        } configs[] = {
            { SkISize::Make(200, 100), 1, 1, 1 },
            { SkISize::Make(200, 200), 1, 1, 1 },
            { SkISize::Make(200, 200), 1, 2, 1 },
            { SkISize::Make(400, 200), 2, 2, 1 },

            { SkISize::Make(200, 100), 1, 1, 0.9f  },
            { SkISize::Make(200, 200), 1, 1, 0.75f },
            { SkISize::Make(200, 200), 1, 2, 0.5f  },
            { SkISize::Make(400, 200), 2, 2, 0.25f },

            { SkISize::Make(200, 200), 0.5f, 1,    1 },
            { SkISize::Make(200, 200), 1,    0.5f, 1 },
            { SkISize::Make(200, 200), 0.5f, 0.5f, 1 },
            { SkISize::Make(200, 200), 2,    2,    1 },

            { SkISize::Make(200, 100), -1,  1, 1    },
            { SkISize::Make(200, 100),  1, -1, 1    },
            { SkISize::Make(200, 100), -1, -1, 1    },
            { SkISize::Make(200, 100), -1, -1, 0.5f },
        };

        auto srgbColorSpace = SkColorSpace::MakeSRGB();
        const unsigned kDrawsPerRow = 4;
        const SkScalar kDrawSize = 250;

        for (size_t i = 0; i < SK_ARRAY_COUNT(configs); ++i) {
            SkPaint p;
            p.setAlphaf(configs[i].opacity);

            SkMatrix m = SkMatrix::MakeScale(configs[i].scaleX, configs[i].scaleY);
            if (configs[i].scaleX < 0) {
                m.postTranslate(SkIntToScalar(configs[i].size.width()), 0);
            }
            if (configs[i].scaleY < 0) {
                m.postTranslate(0, SkIntToScalar(configs[i].size.height()));
            }
            std::unique_ptr<SkImageGenerator> gen =
                SkImageGenerator::MakeFromPicture(configs[i].size, fPicture, &m,
                                                 p.getAlpha() != 255 ? &p : nullptr,
                                                 SkImage::BitDepth::kU8, srgbColorSpace);

            SkImageInfo bmInfo = gen->getInfo().makeColorSpace(canvas->imageInfo().refColorSpace());

            SkBitmap bm;
            bm.allocPixels(bmInfo);
            SkAssertResult(gen->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));

            const SkScalar x = kDrawSize * (i % kDrawsPerRow);
            const SkScalar y = kDrawSize * (i / kDrawsPerRow);

            p.setColor(0xfff0f0f0);
            p.setAlphaf(1.0f);
            canvas->drawRect(SkRect::MakeXYWH(x, y,
                                              SkIntToScalar(bm.width()),
                                              SkIntToScalar(bm.height())), p);
            canvas->drawBitmap(bm, x, y);
        }
    }

private:
    sk_sp<SkPicture> fPicture;

    const SkScalar kPictureWidth = 200;
    const SkScalar kPictureHeight = 100;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new PictureGeneratorGM;)
