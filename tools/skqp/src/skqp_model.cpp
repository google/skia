/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skqp/src/skqp.h"
#include "tools/skqp/src/skqp_model.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "src/utils/SkOSPath.h"

#include <limits.h>

#ifndef SK_SKQP_GLOBAL_ERROR_TOLERANCE
#define SK_SKQP_GLOBAL_ERROR_TOLERANCE 0
#endif

////////////////////////////////////////////////////////////////////////////////

static inline uint32_t color(const SkPixmap& pm, SkIPoint p) {
    return *pm.addr32(p.x(), p.y());
}

static inline bool inside(SkIPoint point, SkISize dimensions) {
    return (unsigned)point.x() < (unsigned)dimensions.width() &&
           (unsigned)point.y() < (unsigned)dimensions.height();
}

SkQP::RenderOutcome skqp::Check(const SkPixmap& minImg,
                                const SkPixmap& maxImg,
                                const SkPixmap& img,
                                unsigned tolerance,
                                SkBitmap* errorOut) {
    SkQP::RenderOutcome result;
    SkISize dim = img.info().dimensions();
    SkASSERT(minImg.info().dimensions() == dim);
    SkASSERT(maxImg.info().dimensions() == dim);
    static const SkIPoint kNeighborhood[9] = {
        { 0,  0}, // ordered by closest pixels first.
        {-1,  0}, { 1,  0}, { 0, -1}, { 0,  1},
        {-1, -1}, { 1, -1}, {-1,  1}, { 1,  1},
    };
    for (int y = 0; y < dim.height(); ++y) {
        for (int x = 0; x < dim.width(); ++x) {
            const SkIPoint xy{x, y};
            const uint32_t c = color(img, xy);
            int error = INT_MAX;
            // loop over neighborhood (halo);
            for (SkIPoint delta : kNeighborhood) {
                SkIPoint point = xy + delta;
                if (inside(point, dim)) {  // skip out of pixmap bounds.
                    int err = 0;
                    // loop over four color channels.
                    // Return Manhattan distance in channel-space.
                    for (int component : {0, 8, 16, 24}) {
                        uint8_t v    = (c                    >> component) & 0xFF,
                                vmin = (color(minImg, point) >> component) & 0xFF,
                                vmax = (color(maxImg, point) >> component) & 0xFF;
                        err = SkMax32(err, SkMax32((int)v - (int)vmax, (int)vmin - (int)v));
                    }
                    error = SkMin32(error, err);
                }
            }
            if (error > (int)tolerance) {
                ++result.fBadPixelCount;
                result.fTotalError += error;
                result.fMaxError = SkMax32(error, result.fMaxError);
                if (errorOut) {
                    if (!errorOut->getPixels()) {
                        errorOut->allocPixels(SkImageInfo::Make(
                                    dim.width(), dim.height(),
                                    kBGRA_8888_SkColorType,
                                    kOpaque_SkAlphaType));
                        errorOut->eraseColor(SK_ColorWHITE);
                    }
                    SkASSERT((unsigned)error < 256);
                    *(errorOut->getAddr32(x, y)) = SkColorSetARGB(0xFF, (uint8_t)error, 0, 0);
                }
            }
        }
    }
    return result;
}

static SkBitmap decode(sk_sp<SkData> data) {
    SkBitmap bitmap;
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        SkISize size = codec->getInfo().dimensions();
        SkASSERT(!size.isEmpty());
        SkImageInfo info = SkImageInfo::Make(size, skqp::kColorType, skqp::kAlphaType);
        bitmap.allocPixels(info);
        if (SkCodec::kSuccess != codec->getPixels(bitmap.pixmap())) {
            bitmap.reset();
        }
    }
    return bitmap;
}

skqp::ModelResult skqp::CheckAgainstModel(const char* name,
                                          const SkPixmap& pm,
                                          SkQPAssetManager* mgr) {
    skqp::ModelResult result;
    if (pm.colorType() != kColorType || pm.alphaType() != kAlphaType) {
        result.fErrorString = "Model failed: source image format.";
        return result;
    }
    if (pm.info().isEmpty()) {
        result.fErrorString = "Model failed: empty source image";
        return result;
    }
    constexpr char PATH_ROOT[] = "gmkb";
    SkString img_path = SkOSPath::Join(PATH_ROOT, name);
    SkString max_path = SkOSPath::Join(img_path.c_str(), kMaxPngPath);
    SkString min_path = SkOSPath::Join(img_path.c_str(), kMinPngPath);

    result.fMaxPng = mgr->open(max_path.c_str());
    result.fMinPng = mgr->open(min_path.c_str());

    SkBitmap max_image = decode(result.fMaxPng);
    SkBitmap min_image = decode(result.fMinPng);

    if (max_image.isNull() || min_image.isNull()) {
        result.fErrorString = "Model missing";
        return result;
    }
    if (max_image.info().dimensions() != min_image.info().dimensions()) {
        result.fErrorString = "Model has mismatched data.";
        return result;
    }

    if (max_image.info().dimensions() != pm.info().dimensions()) {
        result.fErrorString = "Model data does not match source size.";
        return result;
    }
    result.fOutcome = Check(min_image.pixmap(),
                            max_image.pixmap(),
                            pm,
                            SK_SKQP_GLOBAL_ERROR_TOLERANCE,
                            &result.fErrors);
    return result;
}
