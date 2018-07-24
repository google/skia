
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#include "SkTypes.h"

#ifdef SK_VULKAN

// Once our remaining clients are updated to have vulkan_core.h in their builds instead of the old
// vulkan.h only, we can remove the VULKAN_H_ check here. This also requires that the clients
// include their vulkan.h before include any skia files.
#ifndef VULKAN_H_
#include <vulkan/vulkan_core.h>
#endif


#endif

#endif
