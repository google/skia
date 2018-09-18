/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_knowledge_DEFINED
#define gm_knowledge_DEFINED

#include <cstdint>

namespace skqp {
class AssetManager;
}

namespace gmkb {

enum class Error {
    kNone,     /**< No error. */
    kBadInput, /**< Error with the given image data. */
    kBadData,  /**< Error with the given gmkb data directory. */
};

/**
Check if the given test image matches the expected results.

Each pixel is an un-pre-multiplied RGBA color:
    uint32_t make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }

The image's rowBytes is width*sizeof(uint32_t):
    uint32_t* get_pixel_addr(uint32_t* pixels, int width, int height, int x, int y) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return &pixels[x + (width * y)];
    }

@param pixels, width, height  the image
@param gm_name                the name of the rendering test that produced the image
@param backend                (optional) name of the backend
@param asset_manager          GM KnowledgeBase data files
@param report_directory_path  (optional) locatation to write report to.
@param error_out              (optional) error return code.

@return 0 if the test passes, otherwise a positive number representing how
         badly it failed.  Return FLT_MAX on error.
 */

float Check(const uint32_t* pixels,
            int width,
            int height,
            const char* name,
            const char* backend,
            skqp::AssetManager* asset_manager,
            const char* report_directory_path,
            Error* error_out);

/**
Call this after running all checks.

@param report_directory_path  locatation to write report to.
*/
bool MakeReport(const char* report_directory_path);
}  // namespace gmkb

#endif  // gm_knowledge_DEFINED
