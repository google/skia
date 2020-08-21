/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_graphics_DEFINED
#define sk_graphics_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD


SK_C_API void sk_graphics_init(void);

// purge
SK_C_API void sk_graphics_purge_font_cache(void);
SK_C_API void sk_graphics_purge_resource_cache(void);
SK_C_API void sk_graphics_purge_all_caches(void);

// font cache
SK_C_API size_t sk_graphics_get_font_cache_used(void);
SK_C_API size_t sk_graphics_get_font_cache_limit(void);
SK_C_API size_t sk_graphics_set_font_cache_limit(size_t bytes);
SK_C_API int sk_graphics_get_font_cache_count_used(void);
SK_C_API int sk_graphics_get_font_cache_count_limit(void);
SK_C_API int sk_graphics_set_font_cache_count_limit(int count);
SK_C_API int sk_graphics_get_font_cache_point_size_limit(void);
SK_C_API int sk_graphics_set_font_cache_point_size_limit(int maxPointSize);

// resource cache
SK_C_API size_t sk_graphics_get_resource_cache_total_bytes_used(void);
SK_C_API size_t sk_graphics_get_resource_cache_total_byte_limit(void);
SK_C_API size_t sk_graphics_set_resource_cache_total_byte_limit(size_t newLimit);
SK_C_API size_t sk_graphics_get_resource_cache_single_allocation_byte_limit(void);
SK_C_API size_t sk_graphics_set_resource_cache_single_allocation_byte_limit(size_t newLimit);

// dump
SK_C_API void sk_graphics_dump_memory_statistics(sk_tracememorydump_t* dump);

SK_C_PLUS_PLUS_END_GUARD

#endif
