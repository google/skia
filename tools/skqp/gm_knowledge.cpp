/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_knowledge.h"

#include <cfloat>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/core/SkStreamPriv.h"
#include "../../src/core/SkTSort.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPngEncoder.h"
#include "SkStream.h"

#include "skqp_asset_manager.h"

#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"
#define PATH_IMG_PNG "image.png"
#define PATH_ERR_PNG "errors.png"
#define PATH_REPORT  "report.html"

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

namespace {
struct Run {
    SkString fBackend;
    SkString fGM;
    int fMaxerror;
    int fBadpixels;
};
}  // namespace

static std::vector<Run> gErrors;
static std::mutex gMutex;

namespace gmkb {
bool IsGoodGM(const char* name, skqp::AssetManager* assetManager) {
    return asset_exists(assetManager, SkOSPath::Join(name, PATH_MAX_PNG).c_str())
        && asset_exists(assetManager, SkOSPath::Join(name, PATH_MIN_PNG).c_str());
}

// Assumes that for each GM foo, asset_manager has files foo/{max,min}.png and
// that the report_directory_path already exists on disk.
float Check(const uint32_t* pixels,
            int width,
            int height,
            const char* name,
            const char* backend,
            skqp::AssetManager* assetManager,
            const char* report_directory_path,
            Error* error_out) {
    if (width <= 0 || height <= 0) {
        return set_error_code(error_out, Error::kBadInput);
    }
    size_t N = (unsigned)width * (unsigned)height;
    SkString max_path = SkOSPath::Join(name, PATH_MAX_PNG);
    SkString min_path = SkOSPath::Join(name, PATH_MIN_PNG);
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
    if (badness == 0) {
        std::lock_guard<std::mutex> lock(gMutex);
        gErrors.push_back(Run{SkString(backend), SkString(name), 0, 0});
    }
    if (report_directory_path && badness > 0 && report_directory_path[0] != '\0') {
        if (!backend) {
            backend = "skia";
        }
        SkString report_directory = SkOSPath::Join(report_directory_path, backend);
        sk_mkdir(report_directory.c_str());
        SkString report_subdirectory = SkOSPath::Join(report_directory.c_str(), name);
        sk_mkdir(report_subdirectory.c_str());
        SkString error_path = SkOSPath::Join(report_subdirectory.c_str(), PATH_IMG_PNG);
        SkAssertResult(WritePixmapToFile(rgba8888_to_pixmap(pixels, width, height),
                                         error_path.c_str()));
        SkBitmap errorBitmap;
        errorBitmap.allocPixels(SkImageInfo::Make(width, height, kColorType, kAlphaType));
        uint32_t* errors = (uint32_t*)errorBitmap.getPixels();
        for (size_t i = 0; i < N; ++i) {
            int error = get_error(pixels[i], max_pixels[i], min_pixels[i]);
            errors[i] = error > 0 ? 0xFF000000 + (unsigned)error : 0x00000000;
        }
        error_path = SkOSPath::Join(report_subdirectory.c_str(), PATH_ERR_PNG);
        SkAssertResult(WritePixmapToFile(to_pixmap(errorBitmap), error_path.c_str()));

        SkString report_path = SkOSPath::Join(report_subdirectory.c_str(), PATH_REPORT);

        SkString max_path_out = SkOSPath::Join(report_subdirectory.c_str(), PATH_MAX_PNG);
        SkString min_path_out = SkOSPath::Join(report_subdirectory.c_str(), PATH_MIN_PNG);
        (void)copy(assetManager, max_path.c_str(), max_path_out.c_str());
        (void)copy(assetManager, min_path.c_str(), min_path_out.c_str());

        std::lock_guard<std::mutex> lock(gMutex);
        gErrors.push_back(Run{SkString(backend), SkString(name), badness, badPixelCount});
    }
    if (error_out) {
        *error_out = Error::kNone;
    }
    return (float)badness;
}

bool MakeReport(const char* report_directory_path) {
    std::lock_guard<std::mutex> lock(gMutex);
    {
        SkFILEWStream csvOut(SkOSPath::Join(report_directory_path, "out.csv").c_str());
        if (!csvOut.isValid()) {
            return false;
        }
        for (const Run& run : gErrors) {
            SkString line = SkStringPrintf("\"%s\",\"%s\",%d,%d\n",
                                           run.fBackend.c_str(), run.fGM.c_str(),
                                           run.fMaxerror, run.fBadpixels);
            csvOut.write(line.c_str(), line.size());
        }
    }

    SkFILEWStream out(SkOSPath::Join(report_directory_path, PATH_REPORT).c_str());
    if (!out.isValid()) {
        return false;
    }
    out.writeText(
        "<!doctype html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>SkQP Report</title>\n"
        "<style>\n"
        "img { max-width:48%; border:1px green solid; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h1>SkQP Report</h1>\n"
        "<hr>\n");
    for (const Run& run : gErrors) {
        const SkString& backend = run.fBackend;
        const SkString& gm = run.fGM;
        int maxerror = run.fMaxerror;
        int badpixels = run.fBadpixels;
        if (maxerror == 0 && badpixels == 0) {
            continue;
        }
        SkString rdir = SkOSPath::Join(backend.c_str(), gm.c_str());
        SkString text = SkStringPrintf(
            "<h2>%s</h2>\n"
            "backend: %s\n<br>\n"
            "gm name: %s\n<br>\n"
            "maximum error: %d\n<br>\n"
            "bad pixel counts: %d\n<br>\n"
            "<a  href=\"%s/" PATH_IMG_PNG "\">"
            "<img src=\"%s/" PATH_IMG_PNG "\" alt='img'></a>\n"
            "<a  href=\"%s/" PATH_ERR_PNG "\">"
            "<img src=\"%s/" PATH_ERR_PNG "\" alt='err'></a>\n<br>\n"
            "<a  href=\"%s/" PATH_MAX_PNG "\">max</a>\n<br>\n"
            "<a  href=\"%s/" PATH_MIN_PNG "\">min</a>\n<hr>\n\n",
            rdir.c_str(), backend.c_str(), gm.c_str(), maxerror, badpixels,
            rdir.c_str(), rdir.c_str(), rdir.c_str(),
            rdir.c_str(), rdir.c_str(), rdir.c_str());
        out.write(text.c_str(), text.size());
    }
    out.writeText("</body>\n</html>\n");
    return true;
}
}  // namespace gmkb
