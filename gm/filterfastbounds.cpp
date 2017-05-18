/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurImageFilter.h"
#include "SkDropShadowImageFilter.h"
#include "SkImageSource.h"
#include "SkOffsetImageFilter.h"
#include "SkPath.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkTileImageFilter.h"

namespace skiagm {

// Each method of this type must draw its geometry inside 'r' using 'p'
typedef void(*drawMth)(SkCanvas* canvas, const SkRect& r, const SkPaint& p);

static void draw_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawRect(r, p);
}

static void draw_oval(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawOval(r, p);
}

static void draw_rrect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkScalar xRad = r.width() / 4.0f;
    SkScalar yRad = r.height() / 4.0f;

    SkRRect rr;
    rr.setRectXY(r, xRad, yRad);
    canvas->drawRRect(rr, p);
}

static void draw_drrect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkScalar xRad = r.width() / 4.0f;
    SkScalar yRad = r.height() / 4.0f;

    SkRRect outer;
    outer.setRectXY(r, xRad, yRad);
    SkRRect inner = outer;
    inner.inset(xRad, yRad);
    canvas->drawDRRect(outer, inner, p);
}

static void draw_path(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkPath path;

    path.moveTo(r.fLeft, r.fTop);
    path.lineTo(r.fLeft, r.fBottom);
    path.lineTo(r.fRight, r.fBottom);
    path.close();

    canvas->drawPath(path, p);
}

static void draw_points(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkPoint pts0[2] = { { r.fLeft, r.fTop }, { r.fRight, r.fBottom } };
    SkPoint pts1[2] = { { r.fLeft, r.fBottom }, { r.fRight, r.fTop } };

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts0, p);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts1, p);
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkBitmap bm;

    bm.allocN32Pixels(64, 64);
    SkCanvas temp(bm);
    temp.clear(SK_ColorMAGENTA);

    canvas->drawBitmapRect(bm, r, &p);
}

constexpr drawMth gDrawMthds[] = {
    draw_rect, draw_oval, draw_rrect, draw_drrect, draw_path, draw_points, draw_bitmap
};

static void add_paint(SkTArray<SkPaint>* paints, sk_sp<SkImageFilter> filter) {
    SkPaint& p = paints->push_back();
    p.setImageFilter(std::move(filter));
    SkASSERT(p.canComputeFastBounds());
}

// Create a selection of imagefilter-based paints to test
static void create_paints(SkTArray<SkPaint>* paints, sk_sp<SkImageFilter> source) {
    {
        SkMatrix scale;
        scale.setScale(2.0f, 2.0f);

        sk_sp<SkImageFilter> scaleMIF(
            SkImageFilter::MakeMatrixFilter(scale, kLow_SkFilterQuality, source));

        add_paint(paints, std::move(scaleMIF));
    }

    {
        SkMatrix rot;
        rot.setRotate(-33.3f);

        sk_sp<SkImageFilter> rotMIF(
            SkImageFilter::MakeMatrixFilter(rot, kLow_SkFilterQuality, source));

        add_paint(paints, std::move(rotMIF));
    }

    {
        SkRect src = SkRect::MakeXYWH(20, 20, 10, 10);
        SkRect dst = SkRect::MakeXYWH(30, 30, 30, 30);
        sk_sp<SkImageFilter> tileIF(SkTileImageFilter::Make(src, dst, nullptr));

        add_paint(paints, std::move(tileIF));
    }

    {
        constexpr SkDropShadowImageFilter::ShadowMode kBoth =
                    SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode;

        sk_sp<SkImageFilter> dsif(SkDropShadowImageFilter::Make(10.0f, 10.0f,
                                                                3.0f, 3.0f,
                                                                SK_ColorRED, kBoth,
                                                                source));

        add_paint(paints, std::move(dsif));
    }

    {
        sk_sp<SkImageFilter> dsif(
            SkDropShadowImageFilter::Make(27.0f, 27.0f,
                                            3.0f, 3.0f,
                                            SK_ColorRED,
                                            SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode,
                                            source));

        add_paint(paints, std::move(dsif));
    }

    add_paint(paints, SkBlurImageFilter::Make(3, 3, source));
    add_paint(paints, SkOffsetImageFilter::Make(15, 15, source));
}

// This GM visualizes the fast bounds for various combinations of geometry
// and image filter
class ImageFilterFastBoundGM : public GM {
public:
    ImageFilterFastBoundGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    static constexpr int kTileWidth = 100;
    static constexpr int kTileHeight = 100;
    static constexpr int kNumVertTiles = 7;
    static constexpr int kNumXtraCols = 2;

    SkString onShortName() override{ return SkString("filterfastbounds"); }

    SkISize onISize() override{
        return SkISize::Make((SK_ARRAY_COUNT(gDrawMthds) + kNumXtraCols) * kTileWidth,
                             kNumVertTiles * kTileHeight);
    }

    static void draw_geom_with_paint(drawMth draw, const SkIPoint& off,
                                     SkCanvas* canvas, const SkPaint& p) {
        SkPaint redStroked;
        redStroked.setColor(SK_ColorRED);
        redStroked.setStyle(SkPaint::kStroke_Style);

        SkPaint blueStroked;
        blueStroked.setColor(SK_ColorBLUE);
        blueStroked.setStyle(SkPaint::kStroke_Style);

        const SkRect r = SkRect::MakeLTRB(20, 20, 30, 30);
        SkRect storage;

        canvas->save();
            canvas->translate(SkIntToScalar(off.fX), SkIntToScalar(off.fY));
            canvas->scale(1.5f, 1.5f);

            const SkRect& fastBound = p.computeFastBounds(r, &storage);

            canvas->save();
                canvas->clipRect(fastBound);
                (*draw)(canvas, r, p);
            canvas->restore();

            canvas->drawRect(r, redStroked);
            canvas->drawRect(fastBound, blueStroked);
        canvas->restore();
    }

    static void draw_savelayer_with_paint(const SkIPoint& off,
                                          SkCanvas* canvas,
                                          const SkPaint& p) {
        SkPaint redStroked;
        redStroked.setColor(SK_ColorRED);
        redStroked.setStyle(SkPaint::kStroke_Style);

        SkPaint blueStroked;
        blueStroked.setColor(SK_ColorBLUE);
        blueStroked.setStyle(SkPaint::kStroke_Style);

        const SkRect bounds = SkRect::MakeWH(10, 10);
        SkRect storage;

        canvas->save();
            canvas->translate(30, 30);
            canvas->translate(SkIntToScalar(off.fX), SkIntToScalar(off.fY));
            canvas->scale(1.5f, 1.5f);

            const SkRect& fastBound = p.computeFastBounds(bounds, &storage);

            canvas->saveLayer(&fastBound, &p);
            canvas->restore();

            canvas->drawRect(bounds, redStroked);
            canvas->drawRect(fastBound, blueStroked);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override{

        SkPaint blackFill;

        //-----------
        // Normal paints (no source)
        SkTArray<SkPaint> paints;
        create_paints(&paints, nullptr);

        //-----------
        // Paints with a PictureImageFilter as a source
        sk_sp<SkPicture> pic;

        {
            SkPictureRecorder rec;

            SkCanvas* c = rec.beginRecording(10, 10);
            c->drawRect(SkRect::MakeWH(10, 10), blackFill);
            pic = rec.finishRecordingAsPicture();
        }

        SkTArray<SkPaint> pifPaints;
        create_paints(&pifPaints, SkPictureImageFilter::Make(pic));

        //-----------
        // Paints with a SkImageSource as a source

        auto surface(SkSurface::MakeRasterN32Premul(10, 10));
        {
            SkPaint p;
            SkCanvas* temp = surface->getCanvas();
            temp->clear(SK_ColorYELLOW);
            p.setColor(SK_ColorBLUE);
            temp->drawRect(SkRect::MakeLTRB(5, 5, 10, 10), p);
            p.setColor(SK_ColorGREEN);
            temp->drawRect(SkRect::MakeLTRB(5, 0, 10, 5), p);
        }

        sk_sp<SkImage> image(surface->makeImageSnapshot());
        sk_sp<SkImageFilter> imageSource(SkImageSource::Make(std::move(image)));
        SkTArray<SkPaint> bmsPaints;
        create_paints(&bmsPaints, std::move(imageSource));

        //-----------
        SkASSERT(paints.count() == kNumVertTiles);
        SkASSERT(paints.count() == pifPaints.count());
        SkASSERT(paints.count() == bmsPaints.count());

        // horizontal separators
        for (int i = 1; i < paints.count(); ++i) {
            canvas->drawLine(0,
                             i*SkIntToScalar(kTileHeight),
                             SkIntToScalar((SK_ARRAY_COUNT(gDrawMthds) + kNumXtraCols)*kTileWidth),
                             i*SkIntToScalar(kTileHeight),
                             blackFill);
        }
        // vertical separators
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gDrawMthds) + kNumXtraCols; ++i) {
            canvas->drawLine(SkIntToScalar(i * kTileWidth),
                             0,
                             SkIntToScalar(i * kTileWidth),
                             SkIntToScalar(paints.count() * kTileWidth),
                             blackFill);
        }

        // A column of saveLayers with PictureImageFilters
        for (int i = 0; i < pifPaints.count(); ++i) {
            draw_savelayer_with_paint(SkIPoint::Make(0, i*kTileHeight),
                                      canvas, pifPaints[i]);
        }

        // A column of saveLayers with BitmapSources
        for (int i = 0; i < pifPaints.count(); ++i) {
            draw_savelayer_with_paint(SkIPoint::Make(kTileWidth, i*kTileHeight),
                                      canvas, bmsPaints[i]);
        }

        // Multiple columns with different geometry
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gDrawMthds); ++i) {
            for (int j = 0; j < paints.count(); ++j) {
                draw_geom_with_paint(*gDrawMthds[i],
                                     SkIPoint::Make((i+kNumXtraCols) * kTileWidth, j*kTileHeight),
                                     canvas, paints[j]);
            }
        }

    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFilterFastBoundGM;)
}
