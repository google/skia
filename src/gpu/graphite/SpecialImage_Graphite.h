/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SpecialImage_Graphite_DEFINED
#define skgpu_graphite_SpecialImage_Graphite_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/graphite/TextureProxyView.h"

#include <cstdint>

class SkColorInfo;
class SkImage;
class SkSurfaceProps;
struct SkIRect;
struct SkImageInfo;

namespace skgpu::graphite {
class Recorder;
class TextureProxyView;
}  // namespace skgpu::graphite

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeGraphite(skgpu::graphite::Recorder*,
                                   const SkIRect& subset,
                                   sk_sp<SkImage>,
                                   const SkSurfaceProps&);

sk_sp<SkSpecialImage> MakeGraphite(const SkIRect& subset,
                                   uint32_t uniqueID,
                                   skgpu::graphite::TextureProxyView,
                                   const SkColorInfo&,
                                   const SkSurfaceProps&);

// NOTE: Unlike Ganesh's SkSpecialImages::AsView(), this will not automatically upload a
// raster image to a new texture
skgpu::graphite::TextureProxyView AsTextureProxyView(const SkSpecialImage*);
inline skgpu::graphite::TextureProxyView AsTextureProxyView(sk_sp<const SkSpecialImage> img) {
    return AsTextureProxyView(img.get());
}

}  // namespace SkSpecialImages

#endif
