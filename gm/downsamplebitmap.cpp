/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "ToolUtils.h"
#include "gm.h"

static const char* kFilterQualityNames[] = { "none", "low", "medium", "high" };

struct DownsampleBitmapGM : public skiagm::GM {
    SkBitmap      (*fMakeBitmap)(SkImageInfo);
    SkString        fName;
    SkFilterQuality fFilterQuality;

    DownsampleBitmapGM(SkBitmap (*fn)(SkImageInfo), const char* kind, SkFilterQuality fq)
        : fMakeBitmap(fn)
        , fName(SkStringPrintf("downsamplebitmap_%s_%s", kind, kFilterQualityNames[fq]))
        , fFilterQuality(fq)
    {
        this->setBGColor(0xFFDDDDDD);
    }

    SkString onShortName() override { return fName; }

    SkISize onISize() override {
        SkBitmap bm = fMakeBitmap(SkImageInfo::MakeN32Premul(1,1)/*whatever*/);
        return SkISize::Make(bm.width(), 4 * bm.height());
    }

    void onDraw(SkCanvas* canvas) override {
        SkImageInfo info = canvas->imageInfo();
        if (!info.colorType()) { info = info.makeColorType(   kN32_SkColorType); }
        if (!info.alphaType()) { info = info.makeAlphaType(kPremul_SkAlphaType); }

        SkBitmap bm = fMakeBitmap(info);

        int curY = 0;
        int curHeight;
        float curScale = 1;
        do {

            SkMatrix matrix;
            matrix.setScale( curScale, curScale );

            SkPaint paint;
            paint.setFilterQuality(fFilterQuality);

            canvas->save();
                canvas->translate(0, (SkScalar)curY);
                canvas->concat(matrix);
                canvas->drawBitmap(bm, 0, 0, &paint);
            canvas->restore();

            curHeight = (int) (bm.height() * curScale + 2);
            curY += curHeight;
            curScale *= 0.75f;
        } while (curHeight >= 2 && curY < 4*bm.height());
    }
};

static SkBitmap convert_bitmap_format(SkBitmap src, SkImageInfo info) {
    SkBitmap dst;
    dst.allocPixels(info.makeWH(src.width(), src.height()));

    SkPixmap pm;
    SkAssertResult(dst.peekPixels(&pm));
    SkAssertResult(src.readPixels(pm));

    return dst;
}


static SkBitmap make_text(SkImageInfo info) {
    const SkScalar textSize = 72;

    SkBitmap bm;
    bm.allocPixels(info.makeWH(int(textSize * 8), int(textSize * 6)));
    SkCanvas canvas(bm);
    canvas.drawColor(SK_ColorWHITE);

    SkPaint paint;
    SkFont font;
    font.setSubpixel(true);
    font.setSize(textSize);

    font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle()));
    canvas.drawString("Hamburgefons", textSize/2, 1.2f*textSize, font, paint);
    font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle::Bold()));
    canvas.drawString("Hamburgefons", textSize/2, 2.4f*textSize, font, paint);
    font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()));
    canvas.drawString("Hamburgefons", textSize/2, 3.6f*textSize, font, paint);
    font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle::BoldItalic()));
    canvas.drawString("Hamburgefons", textSize/2, 4.8f*textSize, font, paint);

    return bm;
}
DEF_GM( return new DownsampleBitmapGM(make_text, "text",   kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_text, "text", kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_text, "text",    kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_text, "text",   kNone_SkFilterQuality); )


static SkBitmap make_checkerboard(SkImageInfo info) {
    const auto size      = 512;
    const auto numChecks = 256;

    SkBitmap bm;
    bm.allocN32Pixels(size,size);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            SkPMColor* s = bm.getAddr32(x, y);
            int cx = (x * numChecks) / size;
            int cy = (y * numChecks) / size;
            if ((cx+cy)%2) {
                *s = 0xFFFFFFFF;
            } else {
                *s = 0xFF000000;
            }
        }
    }
    return convert_bitmap_format(bm, info);
}
DEF_GM( return new DownsampleBitmapGM(make_checkerboard, "checkerboard",   kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_checkerboard, "checkerboard", kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_checkerboard, "checkerboard",    kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_checkerboard, "checkerboard",   kNone_SkFilterQuality); )


static SkBitmap make_image(SkImageInfo info) {
    SkBitmap bm;
    if (!GetResourceAsBitmap("images/mandrill_512.png", &bm)) {
        bm.allocN32Pixels(1, 1);
        bm.eraseARGB(255, 255, 0 , 0); // red == bad
    }
    return convert_bitmap_format(bm, info);
}
DEF_GM( return new DownsampleBitmapGM(make_image, "image",   kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_image, "image", kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_image, "image",    kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapGM(make_image, "image",   kNone_SkFilterQuality); )
