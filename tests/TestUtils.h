/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/gpu/GrDataUtils.h"
#include "tests/Test.h"

class GrSurfaceContext;
class GrSurfaceProxy;
typedef uint32_t GrColor;

// Ensure that reading back from 'srcContext' as RGBA 8888 matches 'expectedPixelValues
void test_read_pixels(skiatest::Reporter*, GrSurfaceContext* srcContext,
                      uint32_t expectedPixelValues[], const char* testName);

// See if trying to write RGBA 8888 pixels to 'dstContext' matches matches the
// expectation ('expectedToWork')
void test_write_pixels(skiatest::Reporter*, GrSurfaceContext* srcContext, bool expectedToWork,
                       const char* testName);

// Ensure that the pixels can be copied from 'proxy' viewed as colorType, to an RGBA 8888
// destination (both texture-backed and rendertarget-backed).
void test_copy_from_surface(skiatest::Reporter*, GrContext*, GrSurfaceProxy* proxy,
                            GrColorType colorType, uint32_t expectedPixelValues[],
                            const char* testName);

// Fills data with a red-green gradient
void fill_pixel_data(int width, int height, GrColor* data);

// Create a solid colored backend texture
bool create_backend_texture(GrContext*, GrBackendTexture* backendTex,
                            const SkImageInfo& ii, const SkColor4f& color,
                            GrMipMapped, GrRenderable);

void delete_backend_texture(GrContext*, const GrBackendTexture& backendTex);

// Checks srcBuffer and dstBuffer contain the same colors
bool does_full_buffer_contain_correct_color(const GrColor* srcBuffer, const GrColor* dstBuffer,
                                            int width, int height);

// Encodes the bitmap into a data:/image/png;base64,... url suitable to view in a browser after
// printing to a log. If false is returned, dst holds an error message instead of a URI.
bool bitmap_to_base64_data_uri(const SkBitmap& bitmap, SkString* dst);

/** Used by compare_pixels. */
using ComparePixmapsErrorReporter = void(int x, int y, const float diffs[4]);

/**
 * Compares pixels pointed to by 'a' with 'infoA' and rowBytesA to pixels pointed to by 'b' with
 * 'infoB' and 'rowBytesB'.
 *
 * If the infos have different dimensions error is called with negative coordinate values and
 * zero diffs and no comparisons are made.
 *
 * Before comparison pixels are converted to a common color type, alpha type, and color space.
 * The color type is always 32 bit float. The alpha type is premul if one of 'infoA' and 'infoB' is
 * premul and the other is unpremul. The color space is linear sRGB if 'infoA' and 'infoB' have
 * different colorspaces, otherwise their common color space is used.
 *
 * 'tolRGBA' expresses the allowed difference between pixels in the comparison space per channel. If
 * pixel components differ more than by 'tolRGBA' in absolute value in any channel then 'error' is
 * called with the coordinate and difference in the comparison space (B - A).
 *
 * The function quits after a single error is reported and returns false if 'error' was called and
 * true otherwise.
 */
bool compare_pixels(const GrPixelInfo& infoA, const char* a, size_t rowBytesA,
                    const GrPixelInfo& infoB, const char* b, size_t rowBytesB,
                    const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error);

/** Convenience version of above that takes SkPixmap inputs. */
bool compare_pixels(const SkPixmap& a, const SkPixmap& b, const float tolRGBA[4],
                    std::function<ComparePixmapsErrorReporter>& error);

/**
 * Convenience version that checks that 'pixmap' is a solid field of 'col'
 */
bool check_solid_pixels(const SkColor4f& col, const SkPixmap& pixmap,
                        const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error);
