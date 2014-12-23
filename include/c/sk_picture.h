/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_picture_DEFINED
#define sk_picture_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

sk_picture_recorder_t* sk_picture_recorder_new();
void sk_picture_recorder_delete(sk_picture_recorder_t*);

sk_canvas_t* sk_picture_recorder_begin_recording(sk_picture_recorder_t*, const sk_rect_t*);
sk_picture_t* sk_picture_recorder_end_recording(sk_picture_recorder_t*);

void sk_picture_ref(sk_picture_t*);
void sk_picture_unref(sk_picture_t*);

uint32_t sk_picture_get_unique_id(sk_picture_t*);
sk_rect_t sk_picture_get_bounds(sk_picture_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
