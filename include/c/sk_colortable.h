/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_colortable_DEFINED
#define sk_colortable_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_colortable_unref(sk_colortable_t* ctable);
SK_C_API sk_colortable_t* sk_colortable_new(const sk_pmcolor_t* colors, int count);
SK_C_API int sk_colortable_count(const sk_colortable_t* ctable);
SK_C_API void sk_colortable_read_colors(const sk_colortable_t* ctable, sk_pmcolor_t** colors);

SK_C_PLUS_PLUS_END_GUARD

#endif
