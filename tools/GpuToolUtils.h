/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GpuToolUtils_DEFINED
#define GpuToolUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"

class SkCanvas;
class SkImage;

#if defined(SK_GRAPHITE)
namespace skgpu::graphite {
struct RecorderOptions;
}
#endif

namespace ToolUtils {

sk_sp<SkImage> MakeTextureImage(SkCanvas* canvas, sk_sp<SkImage> orig);

#if defined(SK_GRAPHITE)
skgpu::graphite::RecorderOptions CreateTestingRecorderOptions();
#endif

}  // namespace ToolUtils

#endif
