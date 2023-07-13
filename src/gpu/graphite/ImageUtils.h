/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ImageUtils_DEFINED
#define skgpu_graphite_ImageUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"

#include <tuple>
#include <utility>

enum SkColorType : int;
class SkImage;
struct SkSamplingOptions;

namespace skgpu::graphite {

class Recorder;
class TextureProxyView;

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
Context MakeGraphiteContext(skgpu::graphite::Recorder* recorder,
                            const ContextInfo& info);
}  // namespace skif

#endif // skgpu_graphite_ImageUtils_DEFINED
