/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SpecialImage_Graphite_DEFINED
#define skgpu_graphite_SpecialImage_Graphite_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstdint>

class SkColorInfo;
class SkSpecialImage;
class SkSurfaceProps;
struct SkIRect;

namespace skgpu::graphite {
class Recorder;
class TextureProxyView;
}  // namespace skgpu::graphite

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeGraphite(skgpu::graphite::Recorder*,
                                   const SkIRect& subset,
                                   uint32_t uniqueID,
                                   skgpu::graphite::TextureProxyView,
                                   const SkColorInfo&,
                                   const SkSurfaceProps&);

}  // namespace SkSpecialImages

#endif
