/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "tests/Test.h"

class GrSurfaceContext;
class GrSurfaceProxy;
typedef uint32_t GrColor;

// Ensure that reading back from 'srcContext' as RGBA 8888 matches 'expectedPixelValues
void test_read_pixels(skiatest::Reporter*,
                      GrSurfaceContext* srcContext, uint32_t expectedPixelValues[],
                      const char* testName);

// See if trying to write RGBA 8888 pixels to 'dstContext' matches matches the
// expectation ('expectedToWork')
void test_write_pixels(skiatest::Reporter*,
                       GrSurfaceContext* srcContext, bool expectedToWork, const char* testName);

// Ensure that the pixels can be copied from 'proxy' to an RGBA 8888 destination (both
// texture-backed and rendertarget-backed).
void test_copy_from_surface(skiatest::Reporter*, GrContext*,
                            GrSurfaceProxy* proxy, uint32_t expectedPixelValues[],
                            bool onlyTestRTConfig, const char* testName);

// Ensure that RGBA 8888 pixels can be copied into 'dstContext'
void test_copy_to_surface(skiatest::Reporter*, GrContext*,
                          GrSurfaceContext* dstContext, const char* testName);

// Fills data with a red-green gradient
void fill_pixel_data(int width, int height, GrColor* data);

enum class Renderable : bool {
    kNo = false,
    kYes = true
};

// Create a solid colored backend texture
bool create_backend_texture(GrContext*, GrBackendTexture* backendTex,
                            const SkImageInfo& ii, GrMipMapped mipMapped, SkColor color,
                            Renderable);

void delete_backend_texture(GrContext*, const GrBackendTexture& backendTex);

// Checks srcBuffer and dstBuffer contain the same colors
bool does_full_buffer_contain_correct_color(GrColor* srcBuffer, GrColor* dstBuffer, int width,
                                            int height);

// Encodes the bitmap into a data:/image/png;base64,... url suitable to view in a browser after
// printing to a log. If false is returned, dst holds an error message instead of a URI.
bool bitmap_to_base64_data_uri(const SkBitmap& bitmap, SkString* dst);
