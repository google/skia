/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer) && defined(SK_GRAPHITE) && defined(SK_VULKAN)

extern "C" {

    const char* __tsan_default_suppressions();
    const char* __tsan_default_suppressions() {
        // b/373932392 (Precompile isn't thread safe on Native Vulkan)
        return "race:anv_shader_bin_create";      // Intel Vulkan drivers (mesa-22.1.3).
    }

}

#endif
