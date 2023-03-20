/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTArray.h"

#include <utility>

using namespace skia_private;

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
    canvas->drawPath(SkPath::Polygon({
        {r.fLeft, r.fTop},
        {r.fLeft, r.fBottom},
        {r.fRight, r.fBottom},
    }, true), p);
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

    canvas->drawImageRect(bm.asImage(), r, SkSamplingOptions(), &p);
}

constexpr drawMth gDrawMthds[] = {
    draw_rect, draw_oval, draw_rrect, draw_drrect, draw_path, draw_points, draw_bitmap
};

static void add_paint(TArray<SkPaint>* paints, sk_sp<SkImageFilter> filter) {
    SkPaint& p = paints->push_back();
    p.setImageFilter(std::move(filter));
    SkASSERT(p.canComputeFastBounds());
}

// Create a selection of imagefilter-based paints to test
static void create_paints(TArray<SkPaint>* paints, sk_sp<SkImageFilter> source) {
    {
        SkMatrix scale;
        scale.setScale(2.0f, 2.0f);

        sk_sp<SkImageFilter> scaleMIF(
            SkImageFilters::MatrixTransform(scale, SkSamplingOptions(SkFilterMode::kLinear),
                                            source));

        add_paint(paints, std::move(scaleMIF));
    }

    {
        SkMatrix rot;
        rot.setRotate(-33.3f);

        sk_sp<SkImageFilter> rotMIF(
            SkImageFilters::MatrixTransform(rot, SkSamplingOptions(SkFilterMode::kLinear), source));

        add_paint(paints, std::move(rotMIF));
    }

    {
        SkRect src = SkRect::MakeXYWH(20, 20, 10, 10);
        SkRect dst = SkRect::MakeXYWH(30, 30, 30, 30);
        sk_sp<SkImageFilter> tileIF(SkImageFilters::Tile(src, dst, nullptr));

        add_paint(paints, std::move(tileIF));
    }

    {
        sk_sp<SkImageFilter> dsif =
                SkImageFilters::DropShadow(10.0f, 10.0f, 3.0f, 3.0f, SK_ColorRED, source);

        add_paint(paints, std::move(dsif));
    }

    {
        sk_sp<SkImageFilter> dsif =
            SkImageFilters::DropShadowOnly(27.0f, 27.0f, 3.0f, 3.0f, SK_ColorRED, source);

        add_paint(paints, std::move(dsif));
    }

    add_paint(paints, SkImageFilters::Blur(3, 3, source));
    add_paint(paints, SkImageFilters::Offset(15, 15, source));
}

// This GM visualizes the fast bounds for various combinations of geometry
// and image filter
class ImageFilterFastBoundGM : public GM {
public:
    ImageFilterFastBoundGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    inline static constexpr int kTileWidth = 100;
    inline static constexpr int kTileHeight = 100;
    inline static constexpr int kNumVertTiles = 7;
    inline static constexpr int kNumXtraCols = 2;

    SkString onShortName() override { return SkString("filterfastbounds"); }

    SkISize onISize() override {
        return SkISize::Make((std::size(gDrawMthds) + kNumXtraCols) * kTileWidth,
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

    void onDraw(SkCanvas* canvas) override {

        SkPaint blackFill;

        //-----------
        // Normal paints (no source)
        TArray<SkPaint> paints;
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

        TArray<SkPaint> pifPaints;
        create_paints(&pifPaints, SkImageFilters::Picture(pic));

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
        sk_sp<SkImageFilter> imageSource(SkImageFilters::Image(std::move(image)));
        TArray<SkPaint> bmsPaints;
        create_paints(&bmsPaints, std::move(imageSource));

        //-----------
        SkASSERT(paints.size() == kNumVertTiles);
        SkASSERT(paints.size() == pifPaints.size());
        SkASSERT(paints.size() == bmsPaints.size());

        // horizontal separators
        for (int i = 1; i < paints.size(); ++i) {
            canvas->drawLine(0,
                             i*SkIntToScalar(kTileHeight),
                             SkIntToScalar((std::size(gDrawMthds) + kNumXtraCols)*kTileWidth),
                             i*SkIntToScalar(kTileHeight),
                             blackFill);
        }
        // vertical separators
        for (int i = 0; i < (int)std::size(gDrawMthds) + kNumXtraCols; ++i) {
            canvas->drawLine(SkIntToScalar(i * kTileWidth),
                             0,
                             SkIntToScalar(i * kTileWidth),
                             SkIntToScalar(paints.size() * kTileWidth),
                             blackFill);
        }

        // A column of saveLayers with PictureImageFilters
        for (int i = 0; i < pifPaints.size(); ++i) {
            draw_savelayer_with_paint(SkIPoint::Make(0, i*kTileHeight),
                                      canvas, pifPaints[i]);
        }

        // A column of saveLayers with BitmapSources
        for (int i = 0; i < pifPaints.size(); ++i) {
            draw_savelayer_with_paint(SkIPoint::Make(kTileWidth, i*kTileHeight),
                                      canvas, bmsPaints[i]);
        }

        // Multiple columns with different geometry
        for (int i = 0; i < (int)std::size(gDrawMthds); ++i) {
            for (int j = 0; j < paints.size(); ++j) {
                draw_geom_with_paint(*gDrawMthds[i],
                                     SkIPoint::Make((i+kNumXtraCols) * kTileWidth, j*kTileHeight),
                                     canvas, paints[j]);
            }
        }

    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFilterFastBoundGM;)
}  // namespace skiagm
