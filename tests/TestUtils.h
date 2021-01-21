/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestUtils_DEFINED
#define TestUtils_DEFINED

#include "include/core/SkBitmap.h"
#include "src/gpu/GrDataUtils.h"
#include "tests/Test.h"

class GrSurfaceContext;
class GrSurfaceProxy;
typedef uint32_t GrColor;

// Ensure that reading back from 'srcContext' as RGBA 8888 matches 'expectedPixelValues
void TestReadPixels(skiatest::Reporter*, GrDirectContext*, GrSurfaceContext* srcContext,
                    uint32_t expectedPixelValues[], const char* testName);

// See if trying to write RGBA 8888 pixels to 'dstContext' matches matches the
// expectation ('expectedToWork')
void TestWritePixels(skiatest::Reporter*, GrDirectContext*, GrSurfaceContext* srcContext,
                     bool expectedToWork, const char* testName);

// Ensure that the pixels can be copied from 'proxy' viewed as colorType, to an RGBA 8888
// destination (both texture-backed and rendertarget-backed).
void TestCopyFromSurface(skiatest::Reporter*,
                         GrDirectContext*,
                         sk_sp<GrSurfaceProxy> proxy,
                         GrSurfaceOrigin origin,
                         GrColorType colorType,
                         uint32_t expectedPixelValues[],
                         const char* testName);

// Encodes the bitmap into a data:/image/png;base64,... url suitable to view in a browser after
// printing to a log. If false is returned, dst holds an error message instead of a URI.
bool BipmapToBase64DataURI(const SkBitmap& bitmap, SkString* dst);

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
bool ComparePixels(const GrImageInfo& infoA, const char* a, size_t rowBytesA,
                   const GrImageInfo& infoB, const char* b, size_t rowBytesB,
                   const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error);

/** Convenience version of above that takes SkPixmap inputs. */
bool ComparePixels(const SkPixmap& a, const SkPixmap& b, const float tolRGBA[4],
                   std::function<ComparePixmapsErrorReporter>& error);

/**
 * Convenience version that checks that 'pixmap' is a solid field of 'col'
 */
bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      const float tolRGBA[4],
                      std::function<ComparePixmapsErrorReporter>& error);

/**
 * Checks the ref cnt on a proxy and its backing store. This is only valid if the proxy and the
 * resource are both used on a single thread.
 */
void CheckSingleThreadedProxyRefs(skiatest::Reporter* reporter,
                                  GrSurfaceProxy* proxy,
                                  int32_t expectedProxyRefs,
                                  int32_t expectedBackingRefs);

#endif
