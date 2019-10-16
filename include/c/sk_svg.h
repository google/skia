/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_svg_DEFINED
#define sk_svg_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_canvas_t* sk_svgcanvas_create(const sk_rect_t* bounds, sk_wstream_t* stream);

SK_C_PLUS_PLUS_END_GUARD

#endif
