/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_paint_DEFINED
#define sk_paint_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

sk_paint_t* sk_paint_new();
void sk_paint_delete(sk_paint_t*);
bool sk_paint_is_antialias(const sk_paint_t*);
void sk_paint_set_antialias(sk_paint_t*, bool);
sk_color_t sk_paint_get_color(const sk_paint_t*);
void sk_paint_set_color(sk_paint_t*, sk_color_t);

SK_C_PLUS_PLUS_END_GUARD

#endif
