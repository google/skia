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
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"
#include "tools/ToolUtils.h"

#include <utility>

class SkReadBuffer;

class FailImageFilter : public SkImageFilter_Base {
public:
    static sk_sp<SkImageFilter> Make() {
        return sk_sp<SkImageFilter>(new FailImageFilter);
    }

    SK_FLATTENABLE_HOOKS(FailImageFilter)
protected:
    FailImageFilter() : INHERITED(nullptr, 0, nullptr) {}

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override {
        return nullptr;
    }

private:

    typedef SkImageFilter_Base INHERITED;
};

sk_sp<SkFlattenable> FailImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    return FailImageFilter::Make();
}

class IdentityImageFilter : public SkImageFilter_Base {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> input) {
        return sk_sp<SkImageFilter>(new IdentityImageFilter(std::move(input)));
    }


    SK_FLATTENABLE_HOOKS(IdentityImageFilter)
protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context& ctx, SkIPoint* offset) const override {
        offset->set(0, 0);
        return sk_ref_sp<SkSpecialImage>(ctx.sourceImage());
    }

private:
    IdentityImageFilter(sk_sp<SkImageFilter> input) : INHERITED(&input, 1, nullptr) {}

    typedef SkImageFilter_Base INHERITED;
};

// Register these image filters as deserializable before main().
namespace {
    static struct Initializer {
        Initializer() {
            SK_REGISTER_FLATTENABLE(IdentityImageFilter);
            SK_REGISTER_FLATTENABLE(FailImageFilter);
        }
    } initializer;
}

sk_sp<SkFlattenable> IdentityImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return IdentityImageFilter::Make(common.getInput(0));
}

///////////////////////////////////////////////////////////////////////////////

static void draw_paint(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(std::move(imf));
    paint.setColor(SK_ColorGREEN);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawPaint(paint);
    canvas->restore();
}

static void draw_line(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setImageFilter(imf);
    paint.setStrokeWidth(r.width()/10);
    canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, paint);
}

static void draw_rect(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    paint.setImageFilter(imf);
    SkRect rr(r);
    rr.inset(r.width()/10, r.height()/10);
    canvas->drawRect(rr, paint);
}

static void draw_path(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    paint.setImageFilter(imf);
    paint.setAntiAlias(true);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
}

static void draw_text(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorCYAN);
    SkFont font(ToolUtils::create_portable_typeface(), r.height() / 2);
    SkTextUtils::DrawString(canvas, "Text", r.centerX(), r.centerY(), font, paint,
                            SkTextUtils::kCenter_Align);
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(std::move(imf));

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.allocN32Pixels(bounds.width(), bounds.height());
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, nullptr);

    canvas->drawBitmap(bm, 0, 0, &paint);
}

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersBaseGM : public skiagm::GM {
public:
    ImageFiltersBaseGM () {}

protected:
    SkString onShortName() override {
        return SkString("imagefiltersbase");
    }

    SkISize onISize() override { return SkISize::Make(700, 500); }

    void draw_frame(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        void (*drawProc[])(SkCanvas*, const SkRect&, sk_sp<SkImageFilter>) = {
            draw_paint,
            draw_line, draw_rect, draw_path, draw_text,
            draw_bitmap,
        };

        auto cf = SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcIn);
        sk_sp<SkImageFilter> filters[] = {
            nullptr,
            IdentityImageFilter::Make(nullptr),
            FailImageFilter::Make(),
            SkImageFilters::ColorFilter(std::move(cf), nullptr),
            // The strange 0.29 value tickles an edge case where crop rect calculates
            // a small border, but the blur really needs no border. This tickles
            // an msan uninitialized value bug.
            SkImageFilters::Blur(12.0f, 0.29f, nullptr),
            SkImageFilters::DropShadow(10.0f, 5.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(16);
        SkScalar DX = r.width() + MARGIN;
        SkScalar DY = r.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);
        for (size_t i = 0; i < SK_ARRAY_COUNT(drawProc); ++i) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(filters); ++j) {
                drawProc[i](canvas, r, filters[j]);

                draw_frame(canvas, r);
                canvas->translate(0, DY);
            }
            canvas->restore();
            canvas->translate(DX, 0);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new ImageFiltersBaseGM; )

///////////////////////////////////////////////////////////////////////////////

/*
 *  Want to test combos of filter and LCD text, to be sure we disable LCD in the presence of
 *  a filter.
 */
class ImageFiltersTextBaseGM : public skiagm::GM {
    SkString fSuffix;
public:
    ImageFiltersTextBaseGM(const char suffix[]) : fSuffix(suffix) {}

protected:
    SkString onShortName() override {
        SkString name;
        name.printf("%s_%s", "textfilter", fSuffix.c_str());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(512, 342); }

    void drawWaterfall(SkCanvas* canvas, const SkPaint& paint) {
        static const SkFont::Edging kEdgings[3] = {
            SkFont::Edging::kAlias,
            SkFont::Edging::kAntiAlias,
            SkFont::Edging::kSubpixelAntiAlias,
        };
        SkFont font(ToolUtils::create_portable_typeface(), 30);

        SkAutoCanvasRestore acr(canvas, true);
        for (SkFont::Edging edging : kEdgings) {
            font.setEdging(edging);
            canvas->drawString("Hamburgefon", 0, 0, font, paint);
            canvas->translate(0, 40);
        }
    }

    virtual void installFilter(SkPaint* paint) = 0;

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        canvas->translate(20, 40);

        for (int doSaveLayer = 0; doSaveLayer <= 1; ++doSaveLayer) {
            SkAutoCanvasRestore acr(canvas, true);
            for (int useFilter = 0; useFilter <= 1; ++useFilter) {
                SkAutoCanvasRestore acr2(canvas, true);

                SkPaint paint;
                if (useFilter) {
                    this->installFilter(&paint);
                }
                if (doSaveLayer) {
                    canvas->saveLayer(nullptr, &paint);
                    paint.setImageFilter(nullptr);
                }
                this->drawWaterfall(canvas, paint);

                acr2.restore();
                canvas->translate(250, 0);
            }
            acr.restore();
            canvas->translate(0, 200);
        }
    }

private:
    typedef GM INHERITED;
};

class ImageFiltersText_IF : public ImageFiltersTextBaseGM {
public:
    ImageFiltersText_IF() : ImageFiltersTextBaseGM("image") {}

    void installFilter(SkPaint* paint) override {
        paint->setImageFilter(SkImageFilters::Blur(1.5f, 1.5f, nullptr));
    }
};
DEF_GM( return new ImageFiltersText_IF; )

class ImageFiltersText_CF : public ImageFiltersTextBaseGM {
public:
    ImageFiltersText_CF() : ImageFiltersTextBaseGM("color") {}

    void installFilter(SkPaint* paint) override {
        paint->setColorFilter(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kSrcIn));
    }
};
DEF_GM( return new ImageFiltersText_CF; )
