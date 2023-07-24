/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpecialImageGanesh_DEFINED
#define SkSpecialImageGanesh_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

#include <cstdint>

class GrRecordingContext;
class SkImage;
class SkSpecialImage;
class SkSpecialSurface;
class SkSurfaceProps;
struct SkIRect;
enum GrSurfaceOrigin : int;
struct SkImageInfo;

namespace skif { class Context; }

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromTextureImage(GrRecordingContext* rContext,
                                           const SkIRect& subset,
                                           sk_sp<SkImage> image,
                                           const SkSurfaceProps& props);

sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext*,
                                          const SkIRect& subset,
                                          uint32_t uniqueID,
                                          GrSurfaceProxyView,
                                          const GrColorInfo&,
                                          const SkSurfaceProps&);

/**
 * Regardless of how the underlying backing data is stored, returns the contents as a
 * GrSurfaceProxyView. The returned view's proxy represents the entire backing image, so texture
 * coordinates must be mapped from the content rect (e.g. relative to 'subset()') to the proxy's
 * space (offset by subset().topLeft()).
 */
GrSurfaceProxyView AsView(GrRecordingContext*, const SkSpecialImage*);
inline GrSurfaceProxyView AsView(GrRecordingContext* rContext, sk_sp<const SkSpecialImage> img) {
    return AsView(rContext, img.get());
}
/**
 *  Returns a version of the passed-in image (possibly the original), that is in the Context's
 *  colorspace and color type. This allows filters that do many
 *  texture samples to guarantee that any color space conversion has happened before running.
 */
sk_sp<SkSpecialImage> ImageToColorSpace(const skif::Context&, SkSpecialImage*);

}  // namespace SkSpecialImages

namespace SkSpecialSurfaces {
sk_sp<SkSpecialSurface> MakeRenderTarget(GrRecordingContext*,
                                         const SkImageInfo&,
                                         const SkSurfaceProps&,
                                         GrSurfaceOrigin);
}

#endif
