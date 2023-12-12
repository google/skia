/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef AHardwareBufferUtils_DEFINED
#define AHardwareBufferUtils_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkTypes.h"

#if __ANDROID_API__ >= 26

namespace AHardwareBufferUtils {

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat);

}  // namespace AHardwareBufferUtils

#endif

#endif // AHardwareBufferUtils_DEFINED
