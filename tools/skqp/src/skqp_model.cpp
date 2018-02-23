/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skqp_model.h"
#include "skqp.h"

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkOSPath.h"
#include "SkStream.h"

#ifndef SK_SKQP_GLOBAL_ERROR_TOLERANCE
#define SK_SKQP_GLOBAL_ERROR_TOLERANCE 0
#endif

constexpr SkColorType kColorType = kRGBA_8888_SkColorType;
constexpr SkAlphaType kAlphaType = kUnpremul_SkAlphaType;

#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"

////////////////////////////////////////////////////////////////////////////////

static int get_error(uint32_t value, uint32_t value_max, uint32_t value_min) {
    int error = 0;
    for (int j : {0, 8, 16, 24}) {
        uint8_t    v = (value     >> j) & 0xFF,
                vmin = (value_min >> j) & 0xFF,
                vmax = (value_max >> j) & 0xFF;
        if (v > vmax) {
            error = std::max(v - vmax, error);
        } else if (v < vmin) {
            error = std::max(vmin - v, error);
        }
    }
    return std::max(0, error - SK_SKQP_GLOBAL_ERROR_TOLERANCE);
}

static uint32_t color(const SkPixmap& pm, SkIPoint p) {
    return *pm.addr32(p.x(), p.y());  // SkASSERT(kColorType == colorType());
}

static bool inside(SkIPoint point, SkISize dimensions) {
    return (unsigned)point.x() < (unsigned)dimensions.width() &&
           (unsigned)point.y() < (unsigned)dimensions.height();
}

static int get_error_with_nearby(int x, int y, const SkPixmap& pm,
                                 const SkPixmap& pm_max, const SkPixmap& pm_min) {
    SkIPoint neighborhood[9] = {
        {x    , y    },
        {x - 1, y    },
        {x + 1, y    },
        {x    , y - 1},
        {x    , y + 1},
        {x - 1, y - 1},
        {x + 1, y - 1},
        {x - 1, y + 1},
        {x + 1, y + 1},
    };
    SkISize dimensions = pm_max.info().dimensions();
    SkASSERT(!dimensions.isEmpty() && dimensions == pm_min.info().dimensions());

    uint32_t c = color(pm, {x, y});
    int error = INT32_MAX;

    for (SkIPoint point : neighborhood) {
        if (inside(point, dimensions)) {
            int err = get_error(c, color(pm_max, point), color(pm_min, point));
            if (err == 0) {
                return 0;
            }
            error = std::min(error, err);
        }
    }
    return error;
}

static SkBitmap decode(sk_sp<SkData> data) {
    SkBitmap bitmap;
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        SkISize size = codec->getInfo().dimensions();
        SkASSERT(!size.isEmpty());
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), kColorType, kAlphaType);
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
    SkString max_path = SkOSPath::Join(img_path.c_str(), PATH_MAX_PNG);
    SkString min_path = SkOSPath::Join(img_path.c_str(), PATH_MIN_PNG);
    result.fMaxPng = mgr->open(max_path.c_str());
    result.fMinPng = mgr->open(min_path.c_str());
    SkBitmap max_image = decode(result.fMaxPng);
    SkBitmap min_image = decode(result.fMinPng);
    if (max_image.isNull() || min_image.isNull()) {
        result.fErrorString = "Model missing";
        return result;
    }
    if (max_image.width()  != min_image.width() ||
        max_image.height() != min_image.height())
    {
        result.fErrorString = "Model has mismatched data.";
        return result;
    }
    if (max_image.width() != pm.width() || max_image.height() != pm.height()) {
        result.fErrorString = "Model data does not match source size.";
        return result;
    }
    int badness = 0;
    int badPixelCount = 0;
    SkPixmap pm_max = max_image.pixmap();
    SkPixmap pm_min = min_image.pixmap();
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            int error = get_error_with_nearby(x, y, pm, pm_max, pm_min) ;
            if (error > 0) {
                badness = SkTMax(error, badness);
                ++badPixelCount;
                if (result.fErrors.isNull()) {
                    result.fErrors.allocPixels(
                            SkImageInfo::Make(pm.width(), pm.height(), kColorType, kAlphaType));
                    result.fErrors.eraseColor(0xFFFFFFFF);
                }
                *result.fErrors.getAddr32(x, y) = 0xFF000000 + (unsigned)error;
            }
        }
    }
    if (badness == 0) {
        SkASSERT_RELEASE(0 == badPixelCount);
        result.fMaxPng = nullptr;
        result.fMinPng = nullptr;
    }
    result.fMaxError = badness;
    result.fBadPixelCount = badPixelCount;
    return result;
}
