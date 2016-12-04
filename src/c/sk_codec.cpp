/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"

#include "sk_codec.h"

#include "sk_types_priv.h"

size_t sk_codec_min_buffered_bytes_needed()
{
    return SkCodec::MinBufferedBytesNeeded();
}

sk_codec_t* sk_codec_new_from_stream(sk_stream_t* stream)
{
    return ToCodec(SkCodec::NewFromStream(AsStream(stream)));
}

sk_codec_t* sk_codec_new_from_data(sk_data_t* data)
{
    return ToCodec(SkCodec::NewFromData(AsData(data)));
}

void sk_codec_destroy(sk_codec_t* codec)
{
    delete AsCodec(codec);
}

void sk_codec_get_info(sk_codec_t* codec, sk_imageinfo_t* info)
{
    from_sk(AsCodec(codec)->getInfo(), info);
}

sk_colorspace_t* sk_codec_get_color_space(sk_codec_t* codec)
{
    return ToColorSpace(AsCodec(codec)->getColorSpace());
}

sk_codec_origin_t sk_codec_get_origin(sk_codec_t* codec)
{
    return (sk_codec_origin_t)AsCodec(codec)->getOrigin();
}

void sk_codec_get_scaled_dimensions(sk_codec_t* codec, float desiredScale, sk_isize_t* dimensions)
{
    *dimensions = ToISize(AsCodec(codec)->getScaledDimensions(desiredScale));
}

void sk_codec_get_valid_subset(sk_codec_t* codec, sk_irect_t* desiredSubset)
{
    AsCodec(codec)->getValidSubset(AsIRect(desiredSubset));
}

sk_encoded_format_t sk_codec_get_encoded_format(sk_codec_t* codec)
{
    return (sk_encoded_format_t)AsCodec(codec)->getEncodedFormat();
}

sk_codec_result_t sk_codec_get_pixels(sk_codec_t* codec, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_codec_options_t* coptions, sk_color_t ctable[], int* ctableCount)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkCodec::Options options;
    if (!from_c(*coptions, &options)) {
        return INVALID_PARAMETERS_SK_CODEC_RESULT;
    }
    return (sk_codec_result_t)AsCodec(codec)->getPixels(info, pixels, rowBytes, &options, ctable, ctableCount);
}

sk_codec_result_t sk_codec_get_pixels_using_defaults(sk_codec_t* codec, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_codec_result_t)AsCodec(codec)->getPixels(info, pixels, rowBytes);
}
