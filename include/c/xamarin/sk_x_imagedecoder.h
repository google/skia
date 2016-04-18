/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_imagedecoder_DEFINED
#define sk_x_imagedecoder_DEFINED

#include "sk_types.h"
#include "xamarin\sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_imagedecoder_destructor(sk_imagedecoder_t* cdecoder);
SK_API sk_imagedecoder_format_t sk_imagedecoder_get_decoder_format(sk_imagedecoder_t* cdecoder);
SK_API sk_imagedecoder_format_t sk_imagedecoder_get_stream_format(sk_stream_streamrewindable_t* cstream);
SK_API const char* sk_imagedecoder_get_format_name_from_format(sk_imagedecoder_format_t cformat);
SK_API const char* sk_imagedecoder_get_format_name_from_decoder(sk_imagedecoder_t* cdecoder);
SK_API bool sk_imagedecoder_get_skip_writing_zeros(sk_imagedecoder_t* cdecoder);
SK_API void sk_imagedecoder_set_skip_writing_zeros(sk_imagedecoder_t* cdecoder, bool skip);
SK_API bool sk_imagedecoder_get_dither_image(sk_imagedecoder_t* cdecoder);
SK_API void sk_imagedecoder_set_dither_image(sk_imagedecoder_t* cdecoder, bool dither);
SK_API bool sk_imagedecoder_get_prefer_quality_over_speed(sk_imagedecoder_t* cdecoder);
SK_API void sk_imagedecoder_set_prefer_quality_over_speed(sk_imagedecoder_t* cdecoder, bool qualityOverSpeed);
SK_API bool sk_imagedecoder_get_require_unpremultiplied_colors(sk_imagedecoder_t* cdecoder);
SK_API void sk_imagedecoder_set_require_unpremultiplied_colors(sk_imagedecoder_t* cdecoder, bool request);
SK_API int sk_imagedecoder_get_sample_size(sk_imagedecoder_t* cdecoder);
SK_API void sk_imagedecoder_set_sample_size(sk_imagedecoder_t* cdecoder, int size);
SK_API void sk_imagedecoder_cancel_decode(sk_imagedecoder_t* cdecoder);
SK_API bool sk_imagedecoder_should_cancel_decode(sk_imagedecoder_t* cdecoder);
SK_API sk_imagedecoder_result_t sk_imagedecoder_decode(sk_imagedecoder_t* cdecoder, sk_stream_t* cstream, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode);
SK_API sk_imagedecoder_t* sk_imagedecoder_factory(sk_stream_streamrewindable_t* cstream);
SK_API bool sk_imagedecoder_decode_file(const char* file, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format);
SK_API bool sk_imagedecoder_decode_memory(const void* buffer, size_t size, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format);
SK_API bool sk_imagedecoder_decode_stream(sk_stream_streamrewindable_t* cstream, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format);

SK_C_PLUS_PLUS_END_GUARD

#endif
