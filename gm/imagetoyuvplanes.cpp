/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <SkSurface.h>
#include "gm.h"
#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkImage.h"

static sk_sp<SkImage> create_image(GrContext* context, int width, int height) {
    sk_sp<SkSurface> surface;
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    if (context) {
        surface = SkSurface::MakeRenderTarget(context,  SkBudgeted::kYes, info);
    } else {
        surface = SkSurface::MakeRaster(info);
    }
    if (!surface) {
        return nullptr;
    }
    // Create an RGB image from which we will extract planes
    SkPaint paint;
    static const SkColor kColors[] =
            { SK_ColorBLUE, SK_ColorYELLOW, SK_ColorGREEN, SK_ColorWHITE };
    SkScalar r = (width + height) / 4.f;
    paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(0,0), r, kColors,
                                                 nullptr, SK_ARRAY_COUNT(kColors),
                                                 SkShader::kMirror_TileMode));

    surface->getCanvas()->drawPaint(paint);
    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(image_to_yuv_planes, canvas, 120, 525) {
    static const SkScalar kPad = 5.f;
    static const int kImageSize = 32;

    GrContext *context = canvas->getGrContext();
    sk_sp<SkImage> rgbImage(create_image(context, kImageSize, kImageSize));
    if (!rgbImage) {
        return;
    }

    canvas->drawImage(rgbImage.get(), kPad, kPad);
    // Test cases where all three planes are the same size, where just u and v are the same size,
    // and where all differ.
    static const SkISize kSizes[][3] = {
        {{kImageSize, kImageSize}, {kImageSize  , kImageSize  }, {kImageSize,   kImageSize  }},
        {{kImageSize, kImageSize}, {kImageSize/2, kImageSize/2}, {kImageSize/2, kImageSize/2}},
        {{kImageSize, kImageSize}, {kImageSize/2, kImageSize/2}, {kImageSize/3, kImageSize/3}}
    };

    // A mix of rowbytes triples to go with the above sizes.
    static const size_t kRowBytes[][3] {
        {0, 0, 0},
        {kImageSize, kImageSize/2 + 1, kImageSize},
        {kImageSize + 13, kImageSize, kImageSize/3 + 8}
    };


    SkScalar x = kPad;
    for (size_t s = 0; s < SK_ARRAY_COUNT(kSizes); ++s) {
        SkScalar y = rgbImage->height() + 2 * kPad;

        const SkISize *sizes = kSizes[s];
        size_t realRowBytes[3];
        for (int i = 0; i < 3; ++i) {
            realRowBytes[i] = kRowBytes[s][i] ? kRowBytes[s][i] : kSizes[s][i].fWidth;
        }
        SkAutoTDeleteArray<uint8_t> yPlane(new uint8_t[realRowBytes[0] * sizes[0].fHeight]);
        SkAutoTDeleteArray<uint8_t> uPlane(new uint8_t[realRowBytes[1] * sizes[1].fHeight]);
        SkAutoTDeleteArray<uint8_t> vPlane(new uint8_t[realRowBytes[2] * sizes[2].fHeight]);

        void *planes[3] = {yPlane.get(), uPlane.get(), vPlane.get()};

        // Convert the RGB image to YUV planes using each YUV color space and draw the YUV planes
        // to the canvas.
        SkBitmap yuvBmps[3];
        yuvBmps[0].setInfo(SkImageInfo::MakeA8(sizes[0].fWidth, sizes[0].fHeight), kRowBytes[s][0]);
        yuvBmps[1].setInfo(SkImageInfo::MakeA8(sizes[1].fWidth, sizes[1].fHeight), kRowBytes[s][1]);
        yuvBmps[2].setInfo(SkImageInfo::MakeA8(sizes[2].fWidth, sizes[2].fHeight), kRowBytes[s][2]);
        yuvBmps[0].setPixels(yPlane.get());
        yuvBmps[1].setPixels(uPlane.get());
        yuvBmps[2].setPixels(vPlane.get());

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            // Clear the planes so we don't accidentally see the old values if there is a bug in
            // readYUV8Planes().
            memset(yPlane.get(), 0, realRowBytes[0] * sizes[0].fHeight);
            memset(uPlane.get(), 0, realRowBytes[1] * sizes[1].fHeight);
            memset(vPlane.get(), 0, realRowBytes[2] * sizes[2].fHeight);
            if (rgbImage->readYUV8Planes(sizes, planes, kRowBytes[s],
                                         static_cast<SkYUVColorSpace>(space))) {
                yuvBmps[0].notifyPixelsChanged();
                yuvBmps[1].notifyPixelsChanged();
                yuvBmps[2].notifyPixelsChanged();
                for (int i = 0; i < 3; ++i) {
                    canvas->drawBitmap(yuvBmps[i], x, y);
                    y += kPad + yuvBmps[i].height();
                }
            }
        }

        x += rgbImage->width() + kPad;
    }
}
