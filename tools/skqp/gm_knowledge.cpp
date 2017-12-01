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
#include <string>
#include <vector>

#include "SkOSFile.h"

#include "png_interface.h"

constexpr char PATH_MAX_PNG[] = "max.png";
constexpr char PATH_MIN_PNG[] = "min.png";
constexpr char PATH_IMG_PNG[] = "image.png";
constexpr char PATH_ERR_PNG[] = "errors.png";
constexpr char PATH_REPORT[] = "report.html";

static std::string path_join(const std::string& u, const char* v) { return u + "/" + v; }

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

float set_error_code(GMKB_Error* error_out, GMKB_Error error) {
    if (error_out) {
        *error_out = error;
    }
    return FLT_MAX;
}

// Assumes that for each GM foo, there are files ${gmkb_directory_path}/foo/{max,min}.png
float GMKB_Check(GMKB_ImageData image,
                 const char* name,
                 const char* backend,
                 const char* gmkb_directory_path,
                 const char* report_directory_path,
                 GMKB_Error* error_out) {
    using namespace png_interface;

    if (image.width <= 0 || image.height <= 0) {
        return set_error_code(error_out, GMKB_Error_bad_input);
    }
    std::string path = path_join(gmkb_directory_path, name);
    if (!sk_isdir(path.c_str())) {
        return set_error_code(error_out, GMKB_Error_bad_data);
    }
    std::string max_path = path_join(path.c_str(), PATH_MAX_PNG);
    Image max_image = ReadPngRgba8888FromFile(max_path.c_str());
    if (max_image.pixels.empty()) {
        return set_error_code(error_out, GMKB_Error_bad_data);
    }
    std::string min_path = path_join(path.c_str(), PATH_MIN_PNG);
    Image min_image = ReadPngRgba8888FromFile(min_path.c_str());
    if (min_image.pixels.empty()) {
        return set_error_code(error_out, GMKB_Error_bad_data);
    }
    if (max_image.width != min_image.width || max_image.height != min_image.height) {
        return set_error_code(error_out, GMKB_Error_bad_data);
    }
    if (max_image.width != image.width || max_image.height != image.height) {
        return set_error_code(error_out, GMKB_Error_bad_input);
    }
    size_t N = (unsigned)image.width * (unsigned)image.height;
    assert(max_image.pixels.size() == N);
    assert(min_image.pixels.size() == N);
    int badness = 0;
    int badPixelCount = 0;
    for (size_t i = 0; i < N; ++i) {
        int error = get_error(image.pix[i], max_image.pixels[i], min_image.pixels[i]);
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
        std::string report_directory = path_join(std::string(report_directory_path), backend);
        sk_mkdir(report_directory.c_str());
        std::string report_subdirectory = path_join(report_directory, name);
        sk_mkdir(report_subdirectory.c_str());
        std::string error_path = path_join(report_subdirectory, PATH_IMG_PNG);
        SkAssertResult(
                WritePngRgba8888ToFile(image.width, image.height, image.pix, error_path.c_str()));
        std::vector<uint32_t> errors(N);
        for (size_t i = 0; i < N; ++i) {
            int error = get_error(image.pix[i], max_image.pixels[i], min_image.pixels[i]);
            errors[i] = error > 0 ? 0xFF000000 + error : 0x00000000;
        }
        error_path = path_join(report_subdirectory, PATH_ERR_PNG);
        SkAssertResult(WritePngRgba8888ToFile(image.width, image.height, errors.data(),
                                              error_path.c_str()));
        auto report_path = path_join(report_subdirectory, PATH_REPORT);
        auto img_path = path_join(path_join(backend, name), PATH_IMG_PNG);
        auto err_path = path_join(path_join(backend, name), PATH_ERR_PNG);
        std::ofstream out(report_path.c_str());
        out << "maximum error: " << badness << "\n<br>\n"
            << "bad pixel counts: " << badPixelCount << "\n<br>\n"
            << "<a href=\"../../" << img_path << "\">"
            << "<img src=\"../../" << img_path << "\"></a>\n"
            << "<a href=\"../../" << err_path << "\">"
            << "<img src=\"../../" << err_path << "\"></a>\n<br>\n";
    }
    if (error_out) {
        *error_out = GMKB_Error_none;
    }
    return (float)badness;
}

bool GMKB_IsGoodGM(const char* name, const char* gmkb_directory_path) {
    std::string path = path_join(gmkb_directory_path, name);
    return sk_isdir(path.c_str());
}
