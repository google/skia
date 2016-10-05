/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRGBAToYUV.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilterRowMajor255.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkSurface.h"

bool SkRGBAToYUV(const SkImage* image, const SkISize sizes[3], void* const planes[3],
                 const size_t rowBytes[3], SkYUVColorSpace colorSpace) {
    // Matrices that go from RGBA to YUV.
    static const SkScalar kYUVColorSpaceInvMatrices[][15] = {
        // kJPEG_SkYUVColorSpace
        { 0.299001f,  0.586998f,   0.114001f,  0.f, 0.0000821798f * 255.f,
         -0.168736f, -0.331263f,   0.499999f,  0.f, 0.499954f * 255.f,
          0.499999f, -0.418686f,  -0.0813131f, 0.f, 0.499941f * 255.f},

        // kRec601_SkYUVColorSpace
        { 0.256951f,  0.504421f,   0.0977346f, 0.f, 0.0625f * 255.f,
         -0.148212f, -0.290954f,   0.439166f,  0.f, 0.5f * 255.f,
          0.439166f,  -0.367886f, -0.0712802f, 0.f, 0.5f * 255.f},

        // kRec709_SkYUVColorSpace
        { 0.182663f,  0.614473f,  0.061971f,  0.f, 0.0625f * 255.f,
         -0.100672f, -0.338658f,  0.43933f,   0.f, 0.5f * 255.f,
          0.439142f, -0.39891f,  -0.040231f,  0.f, 0.5f * 255.f},
    };
    static_assert(kLastEnum_SkYUVColorSpace == 2, "yuv color matrix array problem");
    static_assert(kJPEG_SkYUVColorSpace     == 0, "yuv color matrix array problem");
    static_assert(kRec601_SkYUVColorSpace   == 1, "yuv color matrix array problem");
    static_assert(kRec709_SkYUVColorSpace   == 2, "yuv color matrix array problem");

    for (int i = 0; i < 3; ++i) {
        size_t rb = rowBytes[i] ? rowBytes[i] : sizes[i].fWidth;
        auto surface(SkSurface::MakeRasterDirect(
                SkImageInfo::MakeA8(sizes[i].fWidth, sizes[i].fHeight), planes[i], rb));
        if (!surface) {
            return false;
        }
        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        int rowStartIdx = 5 * i;
        const SkScalar* row = kYUVColorSpaceInvMatrices[colorSpace] + rowStartIdx;
        paint.setColorFilter(
                SkColorMatrixFilterRowMajor255::MakeSingleChannelOutput(row));
        surface->getCanvas()->drawImageRect(image, SkIRect::MakeWH(image->width(), image->height()),
                                            SkRect::MakeIWH(surface->width(), surface->height()),
                                            &paint);
    }
    return true;
}
