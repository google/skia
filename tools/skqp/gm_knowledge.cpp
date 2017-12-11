/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_knowledge.h"

#include <cfloat>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/core/SkStreamPriv.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkOSFile.h"
#include "SkPngEncoder.h"
#include "SkStream.h"

#include "skqp_asset_manager.h"

#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"
#define PATH_IMG_PNG "image.png"
#define PATH_ERR_PNG "errors.png"
#define PATH_REPORT  "report.html"

////////////////////////////////////////////////////////////////////////////////
inline void path_join_append(std::ostringstream* o) { }

template<class... Types>
void path_join_append(std::ostringstream* o, const char* v, Types... args) {
    constexpr char kPathSeparator[] = "/";
    *o << kPathSeparator << v;
    path_join_append(o, args...);
}

template<class... Types>
std::string path_join(const char* v, Types... args) {
    std::ostringstream o;
    o << v;
    path_join_append(&o, args...);
    return o.str();
}
template<class... Types>
std::string path_join(const std::string& v, Types... args) {
    return path_join(v.c_str(), args...);
}
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
    return error;
}

static float set_error_code(gmkb::Error* error_out, gmkb::Error error) {
    SkASSERT(error != gmkb::Error::kNone);
    if (error_out) {
        *error_out = error;
    }
    return FLT_MAX;
}

static SkPixmap to_pixmap(const SkBitmap& bitmap) {
    SkPixmap pixmap;
    SkAssertResult(bitmap.peekPixels(&pixmap));
    return pixmap;
}


static bool WritePixmapToFile(const SkPixmap& pixmap, const char* path) {
    SkFILEWStream wStream(path);
    SkPngEncoder::Options options;
    options.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return wStream.isValid() && SkPngEncoder::Encode(&wStream, pixmap, options);
}

constexpr SkColorType kColorType = kRGBA_8888_SkColorType;
constexpr SkAlphaType kAlphaType = kUnpremul_SkAlphaType;

static SkPixmap rgba8888_to_pixmap(const uint32_t* pixels, int width, int height) {
    SkImageInfo info = SkImageInfo::Make(width, height, kColorType, kAlphaType);
    return SkPixmap(info, pixels, width * sizeof(uint32_t));
}

static bool asset_exists(skqp::AssetManager* mgr, const char* path) {
    return mgr && nullptr != mgr->open(path);
}

static bool copy(skqp::AssetManager* mgr, const char* path, const char* dst) {
    if (mgr) {
        if (auto stream = mgr->open(path)) {
            SkFILEWStream wStream(dst);
            return wStream.isValid() && SkStreamCopy(&wStream, stream.get());
        }
    }
    return false;
}

static SkBitmap ReadPngRgba8888FromFile(skqp::AssetManager* assetManager, const char* path) {
    SkBitmap bitmap;
    if (auto codec = SkCodec::MakeFromStream(assetManager->open(path))) {
        SkISize size = codec->getInfo().dimensions();
        SkASSERT(!size.isEmpty());
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), kColorType, kAlphaType);
        bitmap.allocPixels(info);
        SkASSERT(bitmap.rowBytes() == (unsigned)bitmap.width() * sizeof(uint32_t));
        if (SkCodec::kSuccess != codec->getPixels(to_pixmap(bitmap))) {
            bitmap.reset();
        }
    }
    return bitmap;
}

namespace gmkb {
// Assumes that for each GM foo, asset_manager has files foo/{max,min}.png
float Check(const uint32_t* pixels,
            int width,
            int height,
            const char* name,
            const char* backend,
            skqp::AssetManager* assetManager,
            const char* report_directory_path,
            Error* error_out) {
    using std::string;
    if (width <= 0 || height <= 0) {
        return set_error_code(error_out, Error::kBadInput);
    }
    size_t N = (unsigned)width * (unsigned)height;
    string max_path = path_join(name, PATH_MAX_PNG);
    string min_path = path_join(name, PATH_MIN_PNG);
    SkBitmap max_image = ReadPngRgba8888FromFile(assetManager, max_path.c_str());
    if (max_image.isNull()) {
        return set_error_code(error_out, Error::kBadData);
    }
    SkBitmap min_image = ReadPngRgba8888FromFile(assetManager, min_path.c_str());
    if (min_image.isNull()) {
        return set_error_code(error_out, Error::kBadData);
    }
    if (max_image.width()  != min_image.width() ||
        max_image.height() != min_image.height())
    {
        return set_error_code(error_out, Error::kBadData);
    }
    if (max_image.width() != width || max_image.height() != height) {
        return set_error_code(error_out, Error::kBadInput);
    }
    int badness = 0;
    int badPixelCount = 0;
    const uint32_t* max_pixels = (uint32_t*)max_image.getPixels();
    const uint32_t* min_pixels = (uint32_t*)min_image.getPixels();

    for (size_t i = 0; i < N; ++i) {
        int error = get_error(pixels[i], max_pixels[i], min_pixels[i]);
        if (error > 0) {
            badness = SkTMax(error, badness);
            ++badPixelCount;
        }
    }
    if (report_directory_path && badness > 0 && report_directory_path[0] != '\0') {
        sk_mkdir(report_directory_path);
        if (!backend) {
            backend = "skia";
        }
        string report_directory = path_join(report_directory_path, backend);
        sk_mkdir(report_directory.c_str());
        string report_subdirectory = path_join(report_directory, name);
        sk_mkdir(report_subdirectory.c_str());
        string error_path = path_join(report_subdirectory, PATH_IMG_PNG);
        SkAssertResult(WritePixmapToFile(rgba8888_to_pixmap(pixels, width, height),
                                         error_path.c_str()));
        SkBitmap errorBitmap;
        errorBitmap.allocPixels(SkImageInfo::Make(width, height, kColorType, kAlphaType));
        uint32_t* errors = (uint32_t*)errorBitmap.getPixels();
        for (size_t i = 0; i < N; ++i) {
            int error = get_error(pixels[i], max_pixels[i], min_pixels[i]);
            errors[i] = error > 0 ? 0xFF000000 + (unsigned)error : 0x00000000;
        }
        error_path = path_join(report_subdirectory, PATH_ERR_PNG);
        SkAssertResult(WritePixmapToFile(to_pixmap(errorBitmap), error_path.c_str()));

        auto report_path = path_join(report_subdirectory, PATH_REPORT);
        auto rdir = path_join("..", "..", backend, name);

        auto max_path_out = path_join(report_subdirectory, PATH_MAX_PNG);
        auto min_path_out = path_join(report_subdirectory, PATH_MIN_PNG);
        (void)copy(assetManager, max_path.c_str(), max_path_out.c_str());
        (void)copy(assetManager, min_path.c_str(), min_path_out.c_str());

        SkString text = SkStringPrintf(
            "backend: %s\n<br>\n"
            "gm name: %s\n<br>\n"
            "maximum error: %d\n<br>\n"
            "bad pixel counts: %d\n<br>\n"
            "<a  href=\"%s/" PATH_IMG_PNG "\">"
            "<img src=\"%s/" PATH_IMG_PNG "\" alt='img'></a>\n"
            "<a  href=\"%s/" PATH_ERR_PNG "\">"
            "<img src=\"%s/" PATH_ERR_PNG "\" alt='err'></a>\n<br>\n"
            "<a  href=\"%s/" PATH_MAX_PNG "\">max</a>\n<br>\n"
            "<a  href=\"%s/" PATH_MIN_PNG "\">min</a>\n<hr>\n",
            backend, name, badness, badPixelCount,
            rdir.c_str(), rdir.c_str(), rdir.c_str(), rdir.c_str(), rdir.c_str(), rdir.c_str());
        SkFILEWStream(report_path.c_str()).write(text.c_str(), text.size());
    }
    if (error_out) {
        *error_out = Error::kNone;
    }
    return (float)badness;
}

bool IsGoodGM(const char* name, skqp::AssetManager* assetManager) {
    std::string max_path = path_join(name, PATH_MAX_PNG);
    std::string min_path = path_join(name, PATH_MIN_PNG);
    return asset_exists(assetManager, max_path.c_str())
        && asset_exists(assetManager, min_path.c_str());
}
}  // namespace gmkb
