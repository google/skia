/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_codec_DEFINED
#define sk_x_codec_DEFINED

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API size_t sk_codec_min_buffered_bytes_needed();

// TODO: png chunk reader
SK_API sk_codec_t* sk_codec_new_from_stream(sk_stream_t* stream);
SK_API sk_codec_t* sk_codec_new_from_data(sk_data_t* data);

SK_API void sk_codec_destroy(sk_codec_t* codec);
SK_API void sk_codec_get_info(sk_codec_t* codec, sk_imageinfo_t* info);
SK_API sk_colorspace_t* sk_codec_get_color_space(sk_codec_t* codec);
SK_API sk_codec_origin_t sk_codec_get_origin(sk_codec_t* codec);
SK_API void sk_codec_get_scaled_dimensions(sk_codec_t* codec, float desiredScale, sk_isize_t* dimensions);
SK_API void sk_codec_get_valid_subset(sk_codec_t* codec, sk_irect_t* desiredSubset);
SK_API sk_encoded_format_t sk_codec_get_encoded_format(sk_codec_t* codec);
SK_API sk_codec_result_t sk_codec_get_pixels(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes, const sk_codec_options_t* options, sk_color_t ctable[], int* ctableCount);
SK_API sk_codec_result_t sk_codec_get_pixels_using_defaults(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes);

// TODO color space
// TODO: SK_API sk_encodedinfo_t sk_codec_get_encoded_info(sk_codec_t* codec);
// TODO: more...

SK_C_PLUS_PLUS_END_GUARD

#endif
