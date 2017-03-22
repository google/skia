/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkImageSource.h"
#include "SkMagnifierImageFilter.h"
#include "SkSurface.h"

static sk_sp<SkImage> make_image(GrContext* context, int size, GrSurfaceOrigin origin) {
    if (context) {
        SkImageInfo ii = SkImageInfo::Make(size, size, kN32_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> surf(SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, 0,
                                                          origin, nullptr));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();

        canvas->clear(SK_ColorRED);
        const struct {
            SkPoint fPt;
            SkColor fColor;
        } rec[] = {
            { { 1.5f, 1.5f }, SK_ColorGREEN },
            { { 2.5f, 1.5f }, SK_ColorBLUE },
            { { 1.5f, 2.5f }, SK_ColorCYAN },
            { { 2.5f, 2.5f }, SK_ColorGRAY },
        };
        SkPaint paint;
        for (const auto& r : rec) {
            paint.setColor(r.fColor);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, 1, &r.fPt, paint);
        }
        return surf->makeImageSnapshot();
    } else {
        SkBitmap bm;
        bm.allocN32Pixels(size, size);
        bm.eraseColor(SK_ColorRED);
        *bm.getAddr32(1, 1) = SkPackARGB32(0xFF, 0x00, 0xFF, 0x00);
        *bm.getAddr32(2, 1) = SkPackARGB32(0xFF, 0x00, 0x00, 0xFF);
        *bm.getAddr32(1, 2) = SkPackARGB32(0xFF, 0x00, 0xFF, 0xFF);
        *bm.getAddr32(2, 2) = SkPackARGB32(0xFF, 0x88, 0x88, 0x88);

        return SkImage::MakeFromBitmap(bm);
    }
}

/*
 * This GM creates an image with a 2x2:
 *    Green | Blue
 *    ------------
 *    Cyan  | Gray
 * block of pixels in one corner of a 33x33 field. The 'srcRect' feature of the
 * SkMagnifierImageFilter is then used to blow it up with different inset border widths.
 *
 * In GPU-mode we wind up drawing 4 rects:
 *
 *     BottomLeft origin + 1-wide inset | TopLeft origin + 1-wide inset
 *     ----------------------------------------------------------------
 *     BottomLeft origin + 7-wide inset | TopLeft origin + 7-wide inset
 *
 * In Raster-mode the source origin isn't used.
 */
class SimpleMagnificationGM : public skiagm::GM {
public:
    SimpleMagnificationGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    SkString onShortName() override {
        return SkString("simple-magnification");
    }

    SkISize onISize() override {
        return SkISize::Make(3*kPad+2*kImgSize, 3*kPad+2*kImgSize);
    }

    void draw(SkCanvas* canvas, sk_sp<SkImage> image, const SkIPoint& offset, int inset) {
        sk_sp<SkImageFilter> imgSrc(SkImageSource::Make(std::move(image)));

        SkRect srcRect = SkRect::MakeXYWH(1.0f, 1.0f, 2.0f, 2.0f);
        sk_sp<SkImageFilter> magFilter(SkMagnifierImageFilter::Make(srcRect, inset, imgSrc));

        SkPaint paint;
        paint.setImageFilter(std::move(magFilter));

        canvas->save();
        canvas->translate(offset.fX, offset.fY);
        SkRect rect = SkRect::MakeWH(kImgSize, kImgSize);
        canvas->drawRect(rect, paint);

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        sk_sp<SkImage> bottomLImg = make_image(context, kImgSize, kBottomLeft_GrSurfaceOrigin);
        sk_sp<SkImage> topLImg = make_image(context, kImgSize, kTopLeft_GrSurfaceOrigin);
        if (!bottomLImg || !topLImg) {
            return;
        }

        int bigOffset = 2 * kPad + kImgSize;

        this->draw(canvas, bottomLImg, SkIPoint::Make(kPad, kPad), 1);
        this->draw(canvas, topLImg, SkIPoint::Make(bigOffset, kPad), 1);
        this->draw(canvas, bottomLImg, SkIPoint::Make(kPad, bigOffset), 7);
        this->draw(canvas, topLImg, SkIPoint::Make(bigOffset, bigOffset), 7);
    }

private:
    static const int kImgSize = 33;
    static const int kPad = 2;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SimpleMagnificationGM;)
