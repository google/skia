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
#define PATH_CSV      "out.csv"

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
        if (SkCodec::kSuccess != codec->getPixels(bitmap.pixmap())) {
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
float Check(const uint32_t* pixels,
            int width,
            int height,
            const char* name,
            const char* backend,
            skqp::AssetManager* assetManager,
            const char* report_directory_path,
            Error* error_out) {
    if (report_directory_path && report_directory_path[0]) {
        SkASSERT_RELEASE(sk_isdir(report_directory_path));
    }
    if (width <= 0 || height <= 0) {
        return set_error_code(error_out, Error::kBadInput);
    }
    size_t N = (unsigned)width * (unsigned)height;
    constexpr char PATH_ROOT[] = "gmkb";
    SkString img_path = SkOSPath::Join(PATH_ROOT, name);
    SkString max_path = SkOSPath::Join(img_path.c_str(), PATH_MAX_PNG);
    SkString min_path = SkOSPath::Join(img_path.c_str(), PATH_MIN_PNG);
    SkBitmap max_image = ReadPngRgba8888FromFile(assetManager, max_path.c_str());
    SkBitmap min_image = ReadPngRgba8888FromFile(assetManager, min_path.c_str());
    if (max_image.isNull() || min_image.isNull()) {
        // No data.
        if (error_out) {
            *error_out = Error::kNone;
        }
        return 0;
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
            errors[i] = error > 0 ? 0xFF000000 + (unsigned)error : 0xFFFFFFFF;
        }
        error_path = SkOSPath::Join(report_subdirectory.c_str(), PATH_ERR_PNG);
        SkAssertResult(WritePixmapToFile(errorBitmap.pixmap(), error_path.c_str()));

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

static constexpr char kDocHead[] =
    "<!doctype html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "<meta charset=\"UTF-8\">\n"
    "<title>SkQP Report</title>\n"
    "<style>\n"
    "img { max-width:48%; border:1px green solid;\n"
    "      background-image:url('data:image/png;base64,iVBORw0KGgoA"
    "AAANSUhEUgAAABAAAAAQCAAAAAA6mKC9AAAAAXNSR0IArs4c6QAAAAJiS0dEAP+H"
    "j8y/AAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAB3RJTUUH3gUBEi4DGRAQYgAAAB1J"
    "REFUGNNjfMoAAVJQmokBDdBHgPE/lPFsYN0BABdaAwN6tehMAAAAAElFTkSuQmCC"
    "'); }\n"
    "</style>\n"
    "<script>\n"
    "function ce(t) { return document.createElement(t); }\n"
    "function ct(n) { return document.createTextNode(n); }\n"
    "function ac(u,v) { return u.appendChild(v); }\n"
    "function br(u) { ac(u, ce(\"br\")); }\n"
    "function ma(s, c) { var a = ce(\"a\"); a.href = s; ac(a, c); return a; }\n"
    "function f(backend, gm, e1, e2) {\n"
    "  var b = ce(\"div\");\n"
    "  var x = ce(\"h2\");\n"
    "  var t = backend + \"/\" + gm;\n"
    "  ac(x, ct(t));\n"
    "  ac(b, x);\n"
    "  ac(b, ct(\"backend: \" + backend));\n"
    "  br(b);\n"
    "  ac(b, ct(\"gm name: \" + gm));\n"
    "  br(b);\n"
    "  ac(b, ct(\"maximum error: \" + e1));\n"
    "  br(b);\n"
    "  ac(b, ct(\"bad pixel counts: \" + e2));\n"
    "  br(b);\n"
    "  var i = ce(\"img\");\n"
    "  i.src = t + \"/image.png\";\n"
    "  i.alt = \"img\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  i = ce(\"img\");\n"
    "  i.src = t + \"/errors.png\";\n"
    "  i.alt = \"img\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  br(b);\n"
    "  ac(b, ct(\"Expectation: \"));\n"
    "  ac(b, ma(t + \"/max.png\", ct(\"max\")));\n"
    "  ac(b, ct(\" | \"));\n"
    "  ac(b, ma(t + \"/min.png\", ct(\"min\")));\n"
    "  ac(b, ce(\"hr\"));\n"
    "  ac(document.body, b);\n"
    "}\n"
    "function main() {\n";

static constexpr char kDocTail[] =
    "}\n"
    "</script>\n"
    "</head>\n"
    "<body onload=\"main()\">\n"
    "<h1>SkQP Report</h1>\n"
    "<p>Left image: test result<br>\n"
    "Right image: errors (white = no error, black = smallest error, red = biggest error)</p>\n"
    "<hr>\n"
    "</body>\n"
    "</html>\n";

static void write(SkWStream* wStream, const SkString& text) {
    wStream->write(text.c_str(), text.size());
}

bool MakeReport(const char* report_directory_path) {
    SkASSERT_RELEASE(sk_isdir(report_directory_path));
    std::lock_guard<std::mutex> lock(gMutex);
    SkFILEWStream csvOut(SkOSPath::Join(report_directory_path, PATH_CSV).c_str());
    SkFILEWStream htmOut(SkOSPath::Join(report_directory_path, PATH_REPORT).c_str());
    SkASSERT_RELEASE(csvOut.isValid());
    if (!csvOut.isValid() || !htmOut.isValid()) {
        return false;
    }
    htmOut.writeText(kDocHead);
    for (const Run& run : gErrors) {
        write(&csvOut, SkStringPrintf("\"%s\",\"%s\",%d,%d\n",
                                      run.fBackend.c_str(), run.fGM.c_str(),
                                      run.fMaxerror, run.fBadpixels));
        if (run.fMaxerror == 0 && run.fBadpixels == 0) {
            continue;
        }
        write(&htmOut, SkStringPrintf("  f(\"%s\", \"%s\", %d, %d);\n",
                                      run.fBackend.c_str(), run.fGM.c_str(),
                                      run.fMaxerror, run.fBadpixels));
    }
    htmOut.writeText(kDocTail);
    return true;
}
}  // namespace gmkb
