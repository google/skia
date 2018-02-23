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

static int get_error_with_nearby(int x, int y, const SkPixmap& pm,
                                 const SkPixmap& pm_max, const SkPixmap& pm_min) {
    struct NearbyPixels {
        const int x, y, w, h;
        struct Iter {
            const int x, y, w, h;
            int8_t curr;
            SkIPoint operator*() const { return this->get(); }
            SkIPoint get() const {
                switch (curr) {
                    case 0: return {x - 1, y - 1};
                    case 1: return {x    , y - 1};
                    case 2: return {x + 1, y - 1};
                    case 3: return {x - 1, y    };
                    case 4: return {x + 1, y    };
                    case 5: return {x - 1, y + 1};
                    case 6: return {x    , y + 1};
                    case 7: return {x + 1, y + 1};
                    default: SkASSERT(false); return {0, 0};
                }
            }
            void skipBad() {
                while (curr < 8) {
                    SkIPoint p = this->get();
                    if (p.x() >= 0 && p.y() >= 0 && p.x() < w && p.y() < h) {
                        return;
                    }
                    ++curr;
                }
                curr = -1;
            }
            void operator++() {
                if (-1 == curr) { return; }
                ++curr;
                this->skipBad();
            }
            bool operator!=(const Iter& other) const { return curr != other.curr; }
        };
        Iter begin() const { Iter i{x, y, w, h, 0}; i.skipBad(); return i; }
        Iter end() const { return Iter{x, y, w, h, -1}; }
    };

    uint32_t c = *pm.addr32(x, y);
    int error = get_error(c, *pm_max.addr32(x, y), *pm_min.addr32(x, y));
    for (SkIPoint p : NearbyPixels{x, y, pm.width(), pm.height()}) {
        if (error == 0) {
            return 0;
        }
        error = SkTMin(error, get_error(
                    c, *pm_max.addr32(p.x(), p.y()), *pm_min.addr32(p.x(), p.y())));
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
