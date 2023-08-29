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
#include "src/gpu/graphite/TextureProxyView.h"

#include <functional>
#include <tuple>
#include <utility>

class SkBitmap;
enum SkColorType : int;
class SkImage;
struct SkImageInfo;
struct SkSamplingOptions;

namespace skgpu::graphite {

class Context;
class Recorder;
class TextureProxyView;

// Create TextureProxyView and SkColorType pair using pixel data in SkBitmap,
// adding any necessary copy commands to Recorder
std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder*,
                                                              const SkBitmap&,
                                                              sk_sp<SkMipmap>,
                                                              Mipmapped,
                                                              skgpu::Budgeted);

sk_sp<SkImage> MakeFromBitmap(Recorder*,
                              const SkColorInfo&,
                              const SkBitmap&,
                              sk_sp<SkMipmap>,
                              skgpu::Budgeted,
                              SkImage::RequiredProperties);

size_t ComputeSize(SkISize dimensions, const TextureInfo&);

sk_sp<SkImage> RescaleImage(Recorder*,
                            const SkImage* srcImage,
                            SkIRect srcIRect,
                            const SkImageInfo& dstInfo,
                            SkImage::RescaleGamma rescaleGamma,
                            SkImage::RescaleMode rescaleMode);

bool GenerateMipmaps(Recorder*, sk_sp<TextureProxy>, const SkColorInfo&);

std::tuple<skgpu::graphite::TextureProxyView, SkColorType> AsView(Recorder*,
                                                                  const SkImage*,
                                                                  skgpu::Mipmapped);

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder*,
                                                               const SkImage*,
                                                               SkSamplingOptions);

} // namespace skgpu::graphite

namespace skif {
class Context;
struct ContextInfo;
struct Functors;

Functors MakeGraphiteFunctors(skgpu::graphite::Recorder* recorder);
Context MakeGraphiteContext(skgpu::graphite::Recorder* recorder,
                            const ContextInfo& info);
}  // namespace skif

#endif // skgpu_graphite_TextureUtils_DEFINED
