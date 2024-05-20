/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureUtils_DEFINED
#define skgpu_graphite_TextureUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/TextureProxyView.h"

#include <functional>
#include <tuple>
#include <utility>

class SkBitmap;
enum SkColorType : int;
class SkImage;
struct SkImageInfo;
struct SkSamplingOptions;

namespace skgpu {
    class RefCntedCallback;
}

namespace skgpu::graphite {

class Caps;
class Context;
class Image;
class Recorder;
class TextureProxyView;

// Create TextureProxyView and SkColorType pair using pixel data in SkBitmap,
// adding any necessary copy commands to Recorder
std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder*,
                                                              const SkBitmap&,
                                                              sk_sp<SkMipmap>,
                                                              Mipmapped,
                                                              skgpu::Budgeted,
                                                              std::string_view label);

sk_sp<TextureProxy> MakePromiseImageLazyProxy(const Caps*,
                                              SkISize dimensions,
                                              TextureInfo,
                                              Volatile,
                                              sk_sp<skgpu::RefCntedCallback> releaseHelper,
                                              SkImages::GraphitePromiseTextureFulfillProc,
                                              SkImages::GraphitePromiseTextureFulfillContext,
                                              SkImages::GraphitePromiseTextureReleaseProc,
                                              std::string_view label);

sk_sp<SkImage> MakeFromBitmap(Recorder*,
                              const SkColorInfo&,
                              const SkBitmap&,
                              sk_sp<SkMipmap>,
                              skgpu::Budgeted,
                              SkImage::RequiredProperties,
                              std::string_view label);

size_t ComputeSize(SkISize dimensions, const TextureInfo&);

sk_sp<Image> CopyAsDraw(Recorder*,
                        const SkImage* image,
                        const SkIRect& subset,
                        const SkColorInfo& dstColorInfo,
                        Budgeted,
                        Mipmapped,
                        SkBackingFit,
                        std::string_view label);

sk_sp<SkImage> RescaleImage(Recorder*,
                            const SkImage* srcImage,
                            SkIRect srcIRect,
                            const SkImageInfo& dstInfo,
                            SkImage::RescaleGamma rescaleGamma,
                            SkImage::RescaleMode rescaleMode);

bool GenerateMipmaps(Recorder*, sk_sp<TextureProxy>, const SkColorInfo&);

// Returns the underlying TextureProxyView if it's a non-YUVA Graphite-backed image.
TextureProxyView AsView(const SkImage*);
inline TextureProxyView AsView(sk_sp<SkImage> image) { return AsView(image.get()); }

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder*,
                                                               const SkImage*,
                                                               SkSamplingOptions);

// Return the color format used for coverage mask textures that are rendered by a GPU
// compute program.
SkColorType ComputeShaderCoverageMaskTargetFormat(const Caps*);

} // namespace skgpu::graphite

namespace skif {

class Backend;

sk_sp<Backend> MakeGraphiteBackend(skgpu::graphite::Recorder* recorder,
                                   const SkSurfaceProps&,
                                   SkColorType);

}  // namespace skif

#endif // skgpu_graphite_TextureUtils_DEFINED
