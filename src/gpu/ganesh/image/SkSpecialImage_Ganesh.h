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

#include <cstdint>

class GrRecordingContext;
class GrSurfaceProxyView;
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

}  // namespace SkSpecialImages

#endif
