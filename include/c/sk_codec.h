/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_codec_DEFINED
#define sk_codec_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API size_t sk_codec_min_buffered_bytes_needed(void);

SK_C_API sk_codec_t* sk_codec_new_from_stream(sk_stream_t* stream, sk_codec_result_t* result);
SK_C_API sk_codec_t* sk_codec_new_from_data(sk_data_t* data);
SK_C_API void sk_codec_destroy(sk_codec_t* codec);
SK_C_API void sk_codec_get_info(sk_codec_t* codec, sk_imageinfo_t* info);
SK_C_API sk_encodedorigin_t sk_codec_get_origin(sk_codec_t* codec);
SK_C_API void sk_codec_get_scaled_dimensions(sk_codec_t* codec, float desiredScale, sk_isize_t* dimensions);
SK_C_API bool sk_codec_get_valid_subset(sk_codec_t* codec, sk_irect_t* desiredSubset);
SK_C_API sk_encoded_image_format_t sk_codec_get_encoded_format(sk_codec_t* codec);
SK_C_API sk_codec_result_t sk_codec_get_pixels(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes, const sk_codec_options_t* options);
SK_C_API sk_codec_result_t sk_codec_start_incremental_decode(sk_codec_t* codec, const sk_imageinfo_t* info, void* pixels, size_t rowBytes, const sk_codec_options_t* options);
SK_C_API sk_codec_result_t sk_codec_incremental_decode(sk_codec_t* codec, int* rowsDecoded);
SK_C_API sk_codec_result_t sk_codec_start_scanline_decode(sk_codec_t* codec, const sk_imageinfo_t* info, const sk_codec_options_t* options);
SK_C_API int sk_codec_get_scanlines(sk_codec_t* codec, void* dst, int countLines, size_t rowBytes);
SK_C_API bool sk_codec_skip_scanlines(sk_codec_t* codec, int countLines);
SK_C_API sk_codec_scanline_order_t sk_codec_get_scanline_order(sk_codec_t* codec);
SK_C_API int sk_codec_next_scanline(sk_codec_t* codec);
SK_C_API int sk_codec_output_scanline(sk_codec_t* codec, int inputScanline);
SK_C_API int sk_codec_get_frame_count(sk_codec_t* codec);
SK_C_API void sk_codec_get_frame_info(sk_codec_t* codec, sk_codec_frameinfo_t* frameInfo);
SK_C_API bool sk_codec_get_frame_info_for_index(sk_codec_t* codec, int index, sk_codec_frameinfo_t* frameInfo);
SK_C_API int sk_codec_get_repetition_count(sk_codec_t* codec);

SK_C_PLUS_PLUS_END_GUARD

#endif
