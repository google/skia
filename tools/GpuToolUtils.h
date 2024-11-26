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

class SkCanvas;
class SkImage;

namespace ToolUtils {

sk_sp<SkImage> MakeTextureImage(SkCanvas* canvas, sk_sp<SkImage> orig);

}  // namespace ToolUtils

#endif
