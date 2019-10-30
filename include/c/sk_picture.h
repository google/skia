/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_picture_DEFINED
#define sk_picture_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_picture_recorder_t* sk_picture_recorder_new(void);
SK_C_API void sk_picture_recorder_delete(sk_picture_recorder_t*);
SK_C_API sk_canvas_t* sk_picture_recorder_begin_recording(sk_picture_recorder_t*, const sk_rect_t*);
SK_C_API sk_picture_t* sk_picture_recorder_end_recording(sk_picture_recorder_t*);
SK_C_API sk_drawable_t* sk_picture_recorder_end_recording_as_drawable(sk_picture_recorder_t*);
SK_C_API sk_canvas_t* sk_picture_get_recording_canvas(sk_picture_recorder_t* crec);

SK_C_API void sk_picture_ref(sk_picture_t*);
SK_C_API void sk_picture_unref(sk_picture_t*);
SK_C_API uint32_t sk_picture_get_unique_id(sk_picture_t*);
SK_C_API void sk_picture_get_cull_rect(sk_picture_t*, sk_rect_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
