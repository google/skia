/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_codec_DEFINED
#define sk_codec_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API size_t sk_codec_min_buffered_bytes_needed();

SK_C_API sk_codec_t* sk_codec_new_from_stream(sk_stream_t* stream);
SK_C_API sk_codec_t* sk_codec_new_from_data(sk_data_t* data);

SK_C_API void sk_codec_destroy(sk_codec_t* codec);
SK_C_API void sk_codec_get_info(sk_codec_t* codec, sk_imageinfo_t* info);
SK_C_API void sk_codec_get_encodedinfo(sk_codec_t* codec, sk_encodedinfo_t* info);
SK_C_API sk_codec_origin_t sk_codec_get_origin(sk_codec_t* codec);
SK_C_API void sk_codec_get_scaled_dimensions(sk_codec_t* codec, float desiredScale, sk_isize_t* dimensions);
SK_C_API bool sk_codec_get_valid_subset(sk_codec_t* codec, sk_irect_t* desiredSubset);
SK_C_API sk_encoded_format_t sk_codec_get_encoded_format(sk_codec_t* codec);
SK_C_API sk_codec_result_t sk_codec_get_pixels(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes, const sk_codec_options_t* options, sk_color_t ctable[], int* ctableCount);
SK_C_API sk_codec_result_t sk_codec_get_pixels_using_defaults(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes);
SK_C_API sk_codec_result_t sk_codec_start_incremental_decode(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes, const sk_codec_options_t* options, sk_color_t ctable[], int* ctableCount);
SK_C_API sk_codec_result_t sk_codec_incremental_decode(sk_codec_t* codec, int* rowsDecoded);
SK_C_API int sk_codec_get_frame_count(sk_codec_t* codec);
SK_C_API void sk_codec_get_frame_info(sk_codec_t* codec, sk_codec_frameinfo_t* frameInfo);
SK_C_API int sk_codec_get_repetition_count(sk_codec_t* codec);

SK_C_PLUS_PLUS_END_GUARD

#endif
