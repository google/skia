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

#define IMAGES_DIRECTORY_PATH "images"
#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"
#define PATH_IMG_PNG "image.png"
#define PATH_ERR_PNG "errors.png"
#define PATH_REPORT  "report.html"
#define PATH_CSV     "out.csv"

#ifndef SK_SKQP_GLOBAL_ERROR_TOLERANCE
#define SK_SKQP_GLOBAL_ERROR_TOLERANCE 0
#endif

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

static float set_error_code(gmkb::Error* error_out, gmkb::Error error) {
    SkASSERT(error != gmkb::Error::kNone);
    if (error_out) {
        *error_out = error;
    }
    return FLT_MAX;
}

static bool WritePixmapToFile(const SkPixmap& pixmap, const char* path) {
    SkFILEWStream wStream(path);
    return wStream.isValid() && SkPngEncoder::Encode(&wStream, pixmap, SkPngEncoder::Options());
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

static SkString make_path(const SkString& images_directory,
                          const char* backend,
                          const char* gm_name,
                          const char* thing) {
    auto path = SkStringPrintf("%s_%s_%s", backend, gm_name, thing);
    return SkOSPath::Join(images_directory.c_str(), path.c_str());
}


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
    SkPixmap pm(SkImageInfo::Make(width, height, kColorType, kAlphaType),
                pixels, width * sizeof(uint32_t));
    SkPixmap pm_max = max_image.pixmap();
    SkPixmap pm_min = min_image.pixmap();
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            int error = get_error_with_nearby(x, y, pm, pm_max, pm_min) ;
            if (error > 0) {
                badness = SkTMax(error, badness);
                ++badPixelCount;
            }
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
        SkString images_directory = SkOSPath::Join(report_directory_path, IMAGES_DIRECTORY_PATH);
        sk_mkdir(images_directory.c_str());

        SkString image_path   = make_path(images_directory, backend, name, PATH_IMG_PNG);
        SkString error_path   = make_path(images_directory, backend, name, PATH_ERR_PNG);
        SkString max_path_out = make_path(images_directory, backend, name, PATH_MAX_PNG);
        SkString min_path_out = make_path(images_directory, backend, name, PATH_MIN_PNG);

        SkAssertResult(WritePixmapToFile(rgba8888_to_pixmap(pixels, width, height),
                                         image_path.c_str()));

        SkBitmap errorBitmap;
        errorBitmap.allocPixels(SkImageInfo::Make(width, height, kColorType, kAlphaType));
        for (int y = 0; y < pm.height(); ++y) {
            for (int x = 0; x < pm.width(); ++x) {
                int error = get_error_with_nearby(x, y, pm, pm_max, pm_min);
                *errorBitmap.getAddr32(x, y) =
                         error > 0 ? 0xFF000000 + (unsigned)error : 0xFFFFFFFF;
            }
        }
        SkAssertResult(WritePixmapToFile(errorBitmap.pixmap(), error_path.c_str()));

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
    "      image-rendering: pixelated;\n"
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
    "  var t = backend + \"_\" + gm;\n"
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
    "  var q = \"" IMAGES_DIRECTORY_PATH "/\" + backend + \"_\" + gm + \"_\";\n"
    "  var i = ce(\"img\");\n"
    "  i.src = q + \"" PATH_IMG_PNG "\";\n"
    "  i.alt = \"img\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  i = ce(\"img\");\n"
    "  i.src = q + \"" PATH_ERR_PNG "\";\n"
    "  i.alt = \"err\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  br(b);\n"
    "  ac(b, ct(\"Expectation: \"));\n"
    "  ac(b, ma(q + \"" PATH_MAX_PNG "\", ct(\"max\")));\n"
    "  ac(b, ct(\" | \"));\n"
    "  ac(b, ma(q + \"" PATH_MIN_PNG "\", ct(\"min\")));\n"
    "  ac(b, ce(\"hr\"));\n"
    "  b.id = backend + \":\" + gm;\n"
    "  ac(document.body, b);\n"
    "  l = ce(\"li\");\n"
    "  ac(l, ct(\"[\" + e1 + \"] \"));\n"
    "  ac(l, ma(\"#\" + backend +\":\"+ gm , ct(t)));\n"
    "  ac(document.getElementById(\"toc\"), l);\n"
    "}\n"
    "function main() {\n";

static constexpr char kDocMiddle[] =
    "}\n"
    "</script>\n"
    "</head>\n"
    "<body onload=\"main()\">\n"
    "<h1>SkQP Report</h1>\n";

static constexpr char kDocTail[] =
    "<ul id=\"toc\"></ul>\n"
    "<hr>\n"
    "<p>Left image: test result<br>\n"
    "Right image: errors (white = no error, black = smallest error, red = biggest error)</p>\n"
    "<hr>\n"
    "</body>\n"
    "</html>\n";

static void write(SkWStream* wStream, const SkString& text) {
    wStream->write(text.c_str(), text.size());
}

enum class Backend {
    kUnknown,
    kGLES,
    kVulkan,
};

static Backend get_backend(const SkString& s) {
    if (s.equals("gles")) {
        return Backend::kGLES;
    } else if (s.equals("vk")) {
        return Backend::kVulkan;
    }
    return Backend::kUnknown;
}


bool MakeReport(const char* report_directory_path) {
    int glesErrorCount = 0, vkErrorCount = 0, gles = 0, vk = 0;

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
        auto backend = get_backend(run.fBackend);
        switch (backend) {
            case Backend::kGLES: ++gles; break;
            case Backend::kVulkan: ++vk; break;
            default: break;
        }
        write(&csvOut, SkStringPrintf("\"%s\",\"%s\",%d,%d\n",
                                      run.fBackend.c_str(), run.fGM.c_str(),
                                      run.fMaxerror, run.fBadpixels));
        if (run.fMaxerror == 0 && run.fBadpixels == 0) {
            continue;
        }
        write(&htmOut, SkStringPrintf("  f(\"%s\", \"%s\", %d, %d);\n",
                                      run.fBackend.c_str(), run.fGM.c_str(),
                                      run.fMaxerror, run.fBadpixels));
        switch (backend) {
            case Backend::kGLES: ++glesErrorCount; break;
            case Backend::kVulkan: ++vkErrorCount; break;
            default: break;
        }
    }
    htmOut.writeText(kDocMiddle);
    write(&htmOut, SkStringPrintf("<p>gles errors: %d (of %d)</br>\n"
                                  "vk errors: %d (of %d)</p>\n",
                                  glesErrorCount, gles, vkErrorCount, vk));
    htmOut.writeText(kDocTail);
    return true;
}
}  // namespace gmkb
