/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkImage.h"
#include "SkImageFilter.h"
#include "SkSurface.h"
#include "ToolUtils.h"
#include "gm.h"

/**
 *  Test drawing a primitive w/ an imagefilter (in this case, just matrix w/ identity) to see
 *  that we apply the xfermode *after* the image has been created and filtered, and not during
 *  the creation step (i.e. before it is filtered).
 *
 *  see https://bug.skia.org/3741
 */
static void do_draw(SkCanvas* canvas, SkBlendMode mode, sk_sp<SkImageFilter> imf) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(220, 220));

        // want to force a layer, so modes like DstIn can combine meaningfully, but the final
        // image can still be shown against our default (opaque) background. non-opaque GMs
        // are a lot more trouble to compare/triage.
        canvas->saveLayer(nullptr, nullptr);
        canvas->drawColor(SK_ColorGREEN);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkRect r0 = SkRect::MakeXYWH(10, 60, 200, 100);
        SkRect r1 = SkRect::MakeXYWH(60, 10, 100, 200);

        paint.setColor(SK_ColorRED);
        canvas->drawOval(r0, paint);

        paint.setColor(0x660000FF);
        paint.setImageFilter(std::move(imf));
        paint.setBlendMode(mode);
        canvas->drawOval(r1, paint);
}

DEF_SIMPLE_GM(imagefilters_xfermodes, canvas, 480, 480) {
        canvas->translate(10, 10);

        // just need an imagefilter to trigger the code-path (which creates a tmp layer)
        sk_sp<SkImageFilter> imf(SkImageFilter::MakeMatrixFilter(SkMatrix::I(),
                                                                 kNone_SkFilterQuality,
                                                                 nullptr));

        const SkBlendMode modes[] = {
            SkBlendMode::kSrcATop, SkBlendMode::kDstIn
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(modes); ++i) {
            canvas->save();
            do_draw(canvas, modes[i], nullptr);
            canvas->translate(240, 0);
            do_draw(canvas, modes[i], imf);
            canvas->restore();

            canvas->translate(0, 240);
        }
}

static sk_sp<SkImage> make_image(SkCanvas* canvas) {
    const SkImageInfo info = SkImageInfo::MakeS32(100, 100, kPremul_SkAlphaType);
    auto              surface(ToolUtils::makeSurface(canvas, info));
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), SkPaint());
    return surface->makeImageSnapshot();
}

// Compare blurs when we're tightly clipped (fast) and not as tightly (slower)
//
// Expect the two to draw the same (modulo the extra border of pixels when the clip is larger)
//
DEF_SIMPLE_GM(fast_slow_blurimagefilter, canvas, 620, 260) {
    sk_sp<SkImage> image(make_image(canvas));
    const SkRect r = SkRect::MakeIWH(image->width(), image->height());

    canvas->translate(10, 10);
    for (SkScalar sigma = 8; sigma <= 128; sigma *= 2) {
        SkPaint paint;
        paint.setImageFilter(SkBlurImageFilter::Make(sigma, sigma, nullptr));

        canvas->save();
        // we outset the clip by 1, to fall out of the fast-case in drawImage
        // i.e. the clip is larger than the image
        for (SkScalar outset = 0; outset <= 1; ++outset) {
            canvas->save();
            canvas->clipRect(r.makeOutset(outset, outset));
            canvas->drawImage(image, 0, 0, &paint);
            canvas->restore();
            canvas->translate(0, r.height() + 20);
        }
        canvas->restore();
        canvas->translate(r.width() + 20, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Resources.h"
#include "SkBlurImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkRRect.h"

static void draw_set(SkCanvas* canvas, sk_sp<SkImageFilter> filters[], int count) {
    const SkRect r = SkRect::MakeXYWH(30, 30, 200, 200);
    const SkScalar offset = 250;
    SkScalar dx = 0, dy = 0;

    for (int i = 0; i < count; ++i) {
        canvas->save();
        SkRRect rr = SkRRect::MakeRectXY(r.makeOffset(dx, dy), 20, 20);
        canvas->clipRRect(rr, true);
        canvas->saveLayer({ &rr.getBounds(), nullptr, filters[i].get(), nullptr, nullptr, 0 });
        canvas->drawColor(0x40FFFFFF);
        canvas->restore();
        canvas->restore();

        if (0 == dx) {
            dx = offset;
        } else {
            dx = 0;
            dy = offset;
        }
    }
}

DEF_SIMPLE_GM(savelayer_with_backdrop, canvas, 830, 550) {
    SkColorMatrix cm;
    cm.setSaturation(10);
    sk_sp<SkColorFilter> cf(SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat));
    const SkScalar kernel[] = { 4, 0, 4, 0, -15, 0, 4, 0, 4 };
    sk_sp<SkImageFilter> filters[] = {
        SkBlurImageFilter::Make(10, 10, nullptr),
        SkDilateImageFilter::Make(8, 8, nullptr),
        SkMatrixConvolutionImageFilter::Make(
                                           { 3, 3 }, kernel, 1, 0, { 0, 0 },
                                           SkMatrixConvolutionImageFilter::kClampToBlack_TileMode,
                                           true, nullptr),
        SkColorFilterImageFilter::Make(std::move(cf), nullptr),
    };

    const struct {
        SkScalar    fSx, fSy, fTx, fTy;
    } xforms[] = {
        { 1, 1, 0, 0 },
        { 0.5f, 0.5f, 530, 0 },
        { 0.25f, 0.25f, 530, 275 },
        { 0.125f, 0.125f, 530, 420 },
    };

    SkPaint paint;
    paint.setFilterQuality(kMedium_SkFilterQuality);
    sk_sp<SkImage> image(GetResourceAsImage("images/mandrill_512.png"));

    canvas->translate(20, 20);
    for (const auto& xform : xforms) {
        canvas->save();
        canvas->translate(xform.fTx, xform.fTy);
        canvas->scale(xform.fSx, xform.fSy);
        canvas->drawImage(image, 0, 0, &paint);
        draw_set(canvas, filters, SK_ARRAY_COUNT(filters));
        canvas->restore();
    }
}
