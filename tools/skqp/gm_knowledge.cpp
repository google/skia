/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_knowledge.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "SkOSFile.h"

#include "png_interface.h"
#include "skqp_asset_manager.h"

constexpr char PATH_MAX_PNG[] = "max.png";
constexpr char PATH_MIN_PNG[] = "min.png";
constexpr char PATH_IMG_PNG[] = "image.png";
constexpr char PATH_ERR_PNG[] = "errors.png";
constexpr char PATH_REPORT[] = "report.html";

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

float set_error_code(gmkb::Error* error_out, gmkb::Error error) {
    if (error_out) {
        *error_out = error;
    }
    return FLT_MAX;
}

namespace gmkb {
// Assumes that for each GM foo, asset_manager has files foo/{max,min}.png
float Check(const uint32_t* pixels,
            int width,
            int height,
            const char* name,
            const char* backend,
            AssetManager* assetManager,
            const char* report_directory_path,
            Error* error_out) {
    using namespace png_interface;

    if (width <= 0 || height <= 0) {
        return set_error_code(error_out, Error::kBadInput);
    }
    size_t N = (unsigned)width * (unsigned)height;
    std::string max_path = path_join(name, PATH_MAX_PNG);
    std::string min_path = path_join(name, PATH_MIN_PNG);
    Image max_image = ReadPngRgba8888FromFile(assetManager, max_path.c_str());
    if (max_image.pixels.empty()) {
        return set_error_code(error_out, Error::kBadData);
    }
    Image min_image = ReadPngRgba8888FromFile(assetManager, min_path.c_str());
    if (min_image.pixels.empty()) {
        return set_error_code(error_out, Error::kBadData);
    }
    if (max_image.width != min_image.width ||
        max_image.height != min_image.height ||
        max_image.pixels.size() != N ||
        min_image.pixels.size() != N)
    {
        return set_error_code(error_out, Error::kBadData);
    }
    if (max_image.width != width || max_image.height != height) {
        return set_error_code(error_out, Error::kBadInput);
    }
    int badness = 0;
    int badPixelCount = 0;
    for (size_t i = 0; i < N; ++i) {
        int error = get_error(pixels[i], max_image.pixels[i], min_image.pixels[i]);
        if (error > 0) {
            badness = std::max(error, badness);
            ++badPixelCount;
        }
    }
    if (report_directory_path && badness > 0 && report_directory_path[0] != '\0') {
        sk_mkdir(report_directory_path);
        if (!backend) {
            backend = "skia";
        }
        std::string report_directory = path_join(report_directory_path, backend);
        sk_mkdir(report_directory.c_str());
        std::string report_subdirectory = path_join(report_directory, name);
        sk_mkdir(report_subdirectory.c_str());
        std::string error_path = path_join(report_subdirectory, PATH_IMG_PNG);
        SkAssertResult(WritePngRgba8888ToFile(width, height, pixels, error_path.c_str()));
        std::vector<uint32_t> errors(N);
        for (size_t i = 0; i < N; ++i) {
            int error = get_error(pixels[i], max_image.pixels[i], min_image.pixels[i]);
            errors[i] = error > 0 ? 0xFF000000 + error : 0x00000000;
        }
        error_path = path_join(report_subdirectory, PATH_ERR_PNG);
        SkAssertResult(WritePngRgba8888ToFile(width, height, errors.data(),
                                              error_path.c_str()));
        auto report_path = path_join(report_subdirectory, PATH_REPORT);
        auto rdir = path_join("..", "..", backend, name);

        auto max_path_out = path_join(report_subdirectory, PATH_MAX_PNG);
        auto min_path_out = path_join(report_subdirectory, PATH_MIN_PNG);
        (void)assetManager->copy(max_path.c_str(), max_path_out.c_str());
        (void)assetManager->copy(min_path.c_str(), min_path_out.c_str());

        std::ofstream out(report_path.c_str());
        out << "backend: " << backend << "\n<br>\n"
            << "gm name: " << name << "\n<br>\n"
            << "maximum error: " << badness << "\n<br>\n"
            << "bad pixel counts: " << badPixelCount << "\n<br>\n"
            << "<a href=\""  << rdir << "/" << PATH_IMG_PNG << "\">"
            << "<img src=\"" << rdir << "/" << PATH_IMG_PNG << "\" alt='img'></a>\n"
            << "<a href=\""  << rdir << "/" << PATH_ERR_PNG << "\">"
            << "<img src=\"" << rdir << "/" << PATH_ERR_PNG << "\" alt='err'></a>\n<br>\n"
            << "<a href=\""  << rdir << "/" << PATH_MAX_PNG << "\">max</a>\n<br>\n"
            << "<a href=\""  << rdir << "/" << PATH_MIN_PNG << "\">min</a>\n<hr>\n";
    }
    if (error_out) {
        *error_out = Error::kNone;
    }
    return (float)badness;
}

bool IsGoodGM(const char* name, AssetManager* assetManager) {
    std::string max_path = path_join(name, PATH_MAX_PNG);
    std::string min_path = path_join(name, PATH_MIN_PNG);
    return assetManager->exists(max_path.c_str())
        && assetManager->exists(min_path.c_str());
}
}  // namespace gmkb
