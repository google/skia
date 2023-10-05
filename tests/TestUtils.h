/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestUtils_DEFINED
#define TestUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"

#include <cstdint>
#include <functional>
#include <memory>

class GrDirectContext;
class GrRecordingContext;
class GrSurfaceProxy;
class SkPixmap;
enum class GrColorType;
namespace skiatest { class Reporter; }
namespace skgpu::ganesh {
class SurfaceContext;
}
typedef uint32_t GrColor;

// Ensure that reading back from 'srcContext' as RGBA 8888 matches 'expectedPixelValues
void TestReadPixels(skiatest::Reporter*,
                    GrDirectContext*,
                    skgpu::ganesh::SurfaceContext*,
                    uint32_t expectedPixelValues[],
                    const char* testName);

// See if trying to write RGBA 8888 pixels to 'dstContext' matches the
// expectation ('expectedToWork')
void TestWritePixels(skiatest::Reporter*,
                     GrDirectContext*,
                     skgpu::ganesh::SurfaceContext*,
                     bool expectedToWork,
                     const char* testName);

// Ensure that the pixels can be copied from 'proxy' viewed as colorType, to an RGBA 8888
// destination (both texture-backed and rendertarget-backed).
void TestCopyFromSurface(skiatest::Reporter*,
                         GrDirectContext*,
                         sk_sp<GrSurfaceProxy> proxy,
                         GrSurfaceOrigin origin,
                         GrColorType colorType,
                         uint32_t expectedPixelValues[],
                         const char* testName);

/** Used by compare_pixels. */
using ComparePixmapsErrorReporter = void(int x, int y, const float diffs[4]);

/**
 * Compares pixels pointed to by 'a' to pixels pointed to by 'b'.
 *
 * If the pixmaps have different dimensions error is called with negative coordinate values and
 * zero diffs and no comparisons are made.
 *
 * Before comparison pixels are converted to a common color type, alpha type, and color space.
 * The color type is always 32 bit float. The alpha type is premul if one of the pixmaps is
 * premul and the other is unpremul. The color space is linear sRGB if the pixmaps have
 * different colorspaces, otherwise their common color space is used.
 *
 * 'tolRGBA' expresses the allowed difference between pixels in the comparison space per channel. If
 * pixel components differ more than by 'tolRGBA' in absolute value in any channel then 'error' is
 * called with the coordinate and difference in the comparison space (B - A).
 *
 * The function quits after a single error is reported and returns false if 'error' was called and
 * true otherwise.
 */
bool ComparePixels(const GrCPixmap& a,
                   const GrCPixmap& b,
                   const float tolRGBA[4],
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

// Makes either a SurfaceContext, SurfaceFillContext, or a SurfaceDrawContext, depending on
// GrRenderable and the GrImageInfo.
// The texture format is the default for the provided color type.
std::unique_ptr<skgpu::ganesh::SurfaceContext> CreateSurfaceContext(
        GrRecordingContext*,
        const GrImageInfo&,
        SkBackingFit = SkBackingFit::kExact,
        GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
        GrRenderable = GrRenderable::kNo,
        int sampleCount = 1,
        skgpu::Mipmapped = skgpu::Mipmapped::kNo,
        GrProtected = GrProtected::kNo,
        skgpu::Budgeted = skgpu::Budgeted::kYes);

#endif
