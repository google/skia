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
class SkSurfaceProps;
struct SkIRect;

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
inline GrSurfaceProxyView AsView(GrRecordingContext* rContext,
                                 const sk_sp<const SkSpecialImage>& img) {
    return AsView(rContext, img.get());
}

}  // namespace SkSpecialImages

#endif
