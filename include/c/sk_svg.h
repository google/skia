/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_svg_DEFINED
#define sk_svg_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_canvas_t* sk_svgcanvas_create(const sk_rect_t* bounds, sk_xmlwriter_t* writer);

SK_C_PLUS_PLUS_END_GUARD

#endif
