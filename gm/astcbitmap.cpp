/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkTextureCompressor.h"

static const char *kASTCFilenames[] = {
    "mandrill_128x128_4x4.astc",    // kASTC_4x4_Format
    "mandrill_130x128_5x4.astc",    // kASTC_5x4_Format
    "mandrill_130x130_5x5.astc",    // kASTC_5x5_Format
    "mandrill_132x130_6x5.astc",    // kASTC_6x5_Format
    "mandrill_132x132_6x6.astc",    // kASTC_6x6_Format
    "mandrill_128x130_8x5.astc",    // kASTC_8x5_Format
    "mandrill_128x132_8x6.astc",    // kASTC_8x6_Format
    "mandrill_128x128_8x8.astc",    // kASTC_8x8_Format
    "mandrill_130x130_10x5.astc",   // kASTC_10x5_Format
    "mandrill_130x132_10x6.astc",   // kASTC_10x6_Format
    "mandrill_130x128_10x8.astc",   // kASTC_10x8_Format
    "mandrill_130x130_10x10.astc",  // kASTC_10x10_Format
    "mandrill_132x130_12x10.astc",  // kASTC_12x10_Format
    "mandrill_132x132_12x12.astc",  // kASTC_12x12_Format
};

static const int kNumASTCFilenames = SK_ARRAY_COUNT(kASTCFilenames);

static inline const char *get_astc_filename(int idx) {
    if (idx < 0 || kNumASTCFilenames <= idx) {
        return "";
    }
    return kASTCFilenames[idx];
}

namespace {
const int kGMDimension = 600;
const int kBitmapDimension = kGMDimension / 4;
}  // namespace

DEF_SIMPLE_GM(astcbitmap, canvas, kGMDimension, kGMDimension) {
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            SkBitmap bm;
            if (GetResourceAsBitmap(get_astc_filename(j*4+i), &bm)) {
                const SkScalar bmX = static_cast<SkScalar>(i*kBitmapDimension);
                const SkScalar bmY = static_cast<SkScalar>(j*kBitmapDimension);
                canvas->drawBitmap(bm, bmX, bmY);
            }
        }
    }
}

DEF_SIMPLE_GM(astc_image, canvas, kGMDimension, kGMDimension) {
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            SkAutoTUnref<SkImage> image(GetResourceAsImage(get_astc_filename(j*4+i)));
            if (image) {
                const SkScalar bmX = static_cast<SkScalar>(i*kBitmapDimension);
                const SkScalar bmY = static_cast<SkScalar>(j*kBitmapDimension);
                canvas->drawImage(image, bmX, bmY);
            }
        }
    }
}
