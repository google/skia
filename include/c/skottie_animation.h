/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skottie_DEFINED
#define skottie_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/*
 * skottie::Animation
 */
SK_C_API void skottie_animation_keepalive();

SK_C_API skottie_animation_t* skottie_animation_make_from_string(const char* data, size_t length);
SK_C_API skottie_animation_t* skottie_animation_make_from_stream(sk_stream_t* stream);
SK_C_API skottie_animation_t* skottie_animation_make_from_file(const char* path);

SK_C_API void skottie_animation_ref(skottie_animation_t* instance);
SK_C_API void skottie_animation_unref(skottie_animation_t* instance);

SK_C_API void skottie_animation_delete(skottie_animation_t *instance);

SK_C_API void skottie_animation_render(skottie_animation_t *instance, sk_canvas_t *canvas, sk_rect_t *dst);
SK_C_API void skottie_animation_render_with_flags(skottie_animation_t *instance, sk_canvas_t *canvas, sk_rect_t *dst, skottie_animation_renderflags_t flags);

SK_C_API void skottie_animation_seek(skottie_animation_t *instance, float t, sksg_invalidation_controller_t *ic);
SK_C_API void skottie_animation_seek_frame(skottie_animation_t *instance, float t, sksg_invalidation_controller_t *ic);
SK_C_API void skottie_animation_seek_frame_time(skottie_animation_t *instance, float t, sksg_invalidation_controller_t *ic);

SK_C_API double skottie_animation_get_duration(skottie_animation_t *instance);
SK_C_API double skottie_animation_get_fps(skottie_animation_t *instance);
SK_C_API double skottie_animation_get_in_point(skottie_animation_t *instance);
SK_C_API double skottie_animation_get_out_point(skottie_animation_t *instance);

SK_C_API void skottie_animation_get_version(skottie_animation_t *instance, sk_string_t* version);
SK_C_API void skottie_animation_get_size(skottie_animation_t *instance, sk_size_t* size);

SK_C_PLUS_PLUS_END_GUARD

#endif
