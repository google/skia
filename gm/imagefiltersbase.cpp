/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkDropShadowImageFilter.h"
#include "SkSpecialImage.h"

class FailImageFilter : public SkImageFilter {
public:
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("FailImageFilter",
                                    FailImageFilter::CreateProc,
                                    FailImageFilter::GetFlattenableType());
        }
    };
    static sk_sp<SkImageFilter> Make() {
        return sk_sp<SkImageFilter>(new FailImageFilter);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(FailImageFilter)

protected:
    FailImageFilter() : INHERITED(nullptr, 0, nullptr) {}

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override {
        return nullptr;
    }
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override {
        return nullptr;
    }

private:
    typedef SkImageFilter INHERITED;
};

static FailImageFilter::Registrar gReg0;

sk_sp<SkFlattenable> FailImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    return FailImageFilter::Make();
}

#ifndef SK_IGNORE_TO_STRING
void FailImageFilter::toString(SkString* str) const {
    str->appendf("FailImageFilter: (");
    str->append(")");
}
#endif

class IdentityImageFilter : public SkImageFilter {
public:
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("IdentityImageFilter",
                                    IdentityImageFilter::CreateProc,
                                    IdentityImageFilter::GetFlattenableType());
        }
    };
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> input) {
        return sk_sp<SkImageFilter>(new IdentityImageFilter(std::move(input)));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(IdentityImageFilter)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override {
        offset->set(0, 0);
        return sk_ref_sp<SkSpecialImage>(source);
    }
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override {
        return sk_ref_sp(const_cast<IdentityImageFilter*>(this));
    }

private:
    IdentityImageFilter(sk_sp<SkImageFilter> input) : INHERITED(&input, 1, nullptr) {}

    typedef SkImageFilter INHERITED;
};

static IdentityImageFilter::Registrar gReg1;

sk_sp<SkFlattenable> IdentityImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return IdentityImageFilter::Make(common.getInput(0));
}

#ifndef SK_IGNORE_TO_STRING
void IdentityImageFilter::toString(SkString* str) const {
    str->appendf("IdentityImageFilter: (");
    str->append(")");
}
#endif

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
    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setTextSize(r.height()/2);
    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas->drawString("Text", r.centerX(), r.centerY(), paint);
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

        auto cf = SkColorFilter::MakeModeFilter(SK_ColorRED, SkBlendMode::kSrcIn);
        sk_sp<SkImageFilter> filters[] = {
            nullptr,
            IdentityImageFilter::Make(nullptr),
            FailImageFilter::Make(),
            SkColorFilterImageFilter::Make(std::move(cf), nullptr),
            SkBlurImageFilter::Make(12.0f, 0.0f, nullptr),
            SkDropShadowImageFilter::Make(
                                    10.0f, 5.0f, 3.0f, 3.0f, SK_ColorBLUE,
                                    SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                                    nullptr),
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

    void drawWaterfall(SkCanvas* canvas, const SkPaint& origPaint) {
        const uint32_t flags[] = {
            0,
            SkPaint::kAntiAlias_Flag,
            SkPaint::kAntiAlias_Flag | SkPaint::kLCDRenderText_Flag,
        };
        SkPaint paint(origPaint);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(30);

        SkAutoCanvasRestore acr(canvas, true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(flags); ++i) {
            paint.setFlags(flags[i]);
            canvas->drawString("Hamburgefon", 0, 0, paint);
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
        paint->setImageFilter(SkBlurImageFilter::Make(1.5f, 1.5f, nullptr));
    }
};
DEF_GM( return new ImageFiltersText_IF; )

class ImageFiltersText_CF : public ImageFiltersTextBaseGM {
public:
    ImageFiltersText_CF() : ImageFiltersTextBaseGM("color") {}

    void installFilter(SkPaint* paint) override {
        paint->setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorBLUE, SkBlendMode::kSrcIn));
    }
};
DEF_GM( return new ImageFiltersText_CF; )
