/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vulkan/vulkan_core.h"

#ifdef SK_BUILD_FOR_ANDROID
// This is needed to get android extensions for external memory
#include "vulkan/vulkan_android.h"
#endif
