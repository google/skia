/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GraphteToolUtils_DEFINED
#define GraphteToolUtils_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu::graphite {
struct RecorderOptions;
}

namespace ToolUtils {

skgpu::graphite::RecorderOptions CreateTestingRecorderOptions();

}  // namespace ToolUtils

#endif
