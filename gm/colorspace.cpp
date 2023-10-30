/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

static const skcms_TransferFunction gTFs[] = {
    SkNamedTransferFn::kSRGB,
    SkNamedTransferFn::k2Dot2,
    SkNamedTransferFn::kLinear,
    SkNamedTransferFn::kRec2020,
    SkNamedTransferFn::kPQ,
    SkNamedTransferFn::kHLG,
    {-3.0f, 2.0f, 2.0f, 1/0.17883277f, 0.28466892f, 0.55991073f,  3.0f },   // HLG scaled 4x
};

static const skcms_Matrix3x3 gGamuts[] = {
    SkNamedGamut::kSRGB,
    SkNamedGamut::kAdobeRGB,
    SkNamedGamut::kDisplayP3,
    SkNamedGamut::kRec2020,
    SkNamedGamut::kXYZ,
};

static const int W = 128,
                 H = 128;

// These GMs demonstrate that our color space management is self-consistent.
// (Important to note, self-consistent, not necessarily correct in an objective sense.)
//
// Let's let,
//
//     SkColorSpace* imgCS = img->colorSpace();
//     SkColorSpace* dstCS = canvas->imageInfo().colorSpace();
//
// Ordinarily we'd just
//
//     canvas->drawImage(img, 0,0);
//
// which would convert that img's pixels from imgCS to dstCS while drawing.
//
// But before we draw in these GMs, we convert the image to an arbitrarily different color space,
// letting midCS range over the cross-product gTFs × gGamuts:
//
//     canvas->drawImage(img->makeColorSpace(midCS), 0,0);
//
// This converts img first from imgCS to midCS, treating midCS as a destination color space,
// and then draws that midCS image to the dstCS canvas, treating midCS as a source color space.
// This should draw a grid of images that look identical except for small precision loss.
//
// If instead of calling SkImage::makeColorSpace() we use SkCanvas::makeSurface() to create a
// midCS offscreen, we construct the same logical imgCS -> midCS -> dstCS transform chain while
// exercising different drawing code paths.  Both strategies should draw roughly the same.

namespace {
    enum Strategy { SkImage_makeColorSpace, SkCanvas_makeSurface };
}

static void draw_colorspace_gm(Strategy strategy, SkCanvas* canvas) {
    SkFont font = ToolUtils::DefaultPortableFont();
    if (!canvas->imageInfo().colorSpace()) {
        canvas->drawString("This GM only makes sense with color-managed drawing.",
                           W,H, font, SkPaint{});
        return;
    }

    sk_sp<SkImage> img = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    if (!img) {
        canvas->drawString("Could not load our test image!",
                           W,H, font, SkPaint{});
        return;
    }

    SkASSERT(img->width()  == W);
    SkASSERT(img->height() == H);
    SkASSERT(img->colorSpace());

    for (skcms_Matrix3x3 gamut : gGamuts) {
        canvas->save();
        for (skcms_TransferFunction tf : gTFs) {
            sk_sp<SkColorSpace> midCS = SkColorSpace::MakeRGB(tf, gamut);

            switch (strategy) {
                case SkImage_makeColorSpace: {
                    canvas->drawImage(img->makeColorSpace(nullptr, midCS), 0,0);
                } break;

                case SkCanvas_makeSurface: {
                    sk_sp<SkSurface> offscreen =
                        canvas->makeSurface(canvas->imageInfo().makeColorSpace(midCS));
                    if (!offscreen) {
                        canvas->drawString("Could not allocate offscreen surface!",
                                           W,H, font, SkPaint{});
                        return;
                    }
                    offscreen->getCanvas()->drawImage(img, 0,0);
                    canvas->drawImage(offscreen->makeImageSnapshot(), 0,0);
                } break;
            }

            canvas->translate(W, 0);
        }
        canvas->restore();
        canvas->translate(0, H);
    }
}

DEF_SIMPLE_GM(colorspace, canvas, W*std::size(gTFs), H*std::size(gGamuts)) {
    draw_colorspace_gm(SkImage_makeColorSpace, canvas);
}

DEF_SIMPLE_GM(colorspace2, canvas, W*std::size(gTFs), H*std::size(gGamuts)) {
    draw_colorspace_gm(SkCanvas_makeSurface, canvas);
}
