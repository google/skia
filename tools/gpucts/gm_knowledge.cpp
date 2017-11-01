/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_knowledge.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

#include "SkOSFile.h"

#include "png_interface.h"

constexpr char GMK_DIR_ENV[] ="GMK_DIR";
constexpr char GMK_REPORT_ENV[] = "GMK_REPORT";
constexpr char PATH_MAX_PNG[] = "max.png";
constexpr char PATH_MIN_PNG[] = "min.png";

static std::string path_join(const std::string& u, const char* v) {
    return u + "/" + v;
}

static std::string getenv_or_abort(const char* key) {
    const char* value = getenv(key);
    if (!value) {
        fprintf(stderr, "Missing environment variable: '%s'.\n", key);
        abort();
    }
    return std::string(value);
}

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

// Assumes that for each GM foo, there are files $GMK_DIR/foo/{max,min}.png

float GMK_Check(GMK_ImageData image, const char* name, const char* backend) {
    using namespace png_interface;

    if (image.width <= 0 || image.height <= 0) {
        return 9009.0f;
    }
    std::string path = path_join(getenv_or_abort(GMK_DIR_ENV), name);
    if (!sk_isdir(path.c_str())) {
        return 9000.0f;
    }
    std::string max_path = path_join(path.c_str(), PATH_MAX_PNG);
    Image max_image = ReadPngRgba8888FromFile(max_path.c_str());
    if (max_image.pixels.empty()) {
        return 9001.0f;
    }
    std::string min_path = path_join(path.c_str(), PATH_MIN_PNG);
    Image min_image = ReadPngRgba8888FromFile(min_path.c_str());
    if (min_image.pixels.empty()) {
        return 9002.0f;
    }
    if (max_image.width != min_image.width || max_image.height != min_image.height) {
        return 9003.0f;
    }
    if (max_image.width != image.width || max_image.height != image.height) {
        return 9004.0f;
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
    if (badness > 0) {
        std::string report_directory = getenv_or_abort(GMK_REPORT_ENV);
        sk_mkdir(report_directory.c_str());
        report_directory = path_join(report_directory, backend);
        sk_mkdir(report_directory.c_str());
        std::string report_subdirectory = path_join(report_directory, name);
        sk_mkdir(report_subdirectory.c_str());
        std::string error_path = path_join(report_subdirectory, "image.png");
        SkAssertResult(WritePngRgba8888ToFile(image.width, image.height, image.pix,
                                              error_path.c_str()));
        std::vector<uint32_t> errors(N);
        for (size_t i = 0; i < N; ++i) {
            int error = get_error(image.pix[i], max_image.pixels[i], min_image.pixels[i]);
            errors[i] = error > 0 ? 0xFF000000 + error : 0x00000000;
        }
        error_path = path_join(report_subdirectory, "errors.png");
        SkAssertResult(WritePngRgba8888ToFile(image.width, image.height, errors.data(),
                                              error_path.c_str()));
        auto report_path = path_join(report_subdirectory, "report.html");
        std::ofstream(report_path.c_str())
               << "maximum error: " << badness << "\n<br>\n"
               << "bad pixel counts: " << badPixelCount << "\n<br>\n"
               << "<a href=\"image.png\"><img src=\"image.png\"></a>\n"
               << "<a href=\"errors.png\"><img src=\"errors.png\"></a>\n<br>\n";
    }
    return (float)badness;
}

bool GMK_IsGoodGM(const char* name) {
    std::string path = path_join(getenv_or_abort(GMK_DIR_ENV), name);
    return sk_isdir(path.c_str());
}

