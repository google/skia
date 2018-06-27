/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
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
    return ToCodec(SkCodec::NewFromData(sk_ref_sp(AsData(data))));
}

void sk_codec_destroy(sk_codec_t* codec)
{
    delete AsCodec(codec);
}

void sk_codec_get_info(sk_codec_t* codec, sk_imageinfo_t* info)
{
    from_sk(AsCodec(codec)->getInfo(), info);
}

void sk_codec_get_encodedinfo(sk_codec_t* codec, sk_encodedinfo_t* info)
{
    *info = ToEncodedInfo(AsCodec(codec)->getEncodedInfo());
}

sk_codec_origin_t sk_codec_get_origin(sk_codec_t* codec)
{
    return (sk_codec_origin_t)AsCodec(codec)->getOrigin();
}

void sk_codec_get_scaled_dimensions(sk_codec_t* codec, float desiredScale, sk_isize_t* dimensions)
{
    *dimensions = ToISize(AsCodec(codec)->getScaledDimensions(desiredScale));
}

bool sk_codec_get_valid_subset(sk_codec_t* codec, sk_irect_t* desiredSubset)
{
    return AsCodec(codec)->getValidSubset(AsIRect(desiredSubset));
}

sk_encoded_image_format_t sk_codec_get_encoded_format(sk_codec_t* codec)
{
    return (sk_encoded_image_format_t)AsCodec(codec)->getEncodedFormat();
}

sk_codec_result_t sk_codec_get_pixels(sk_codec_t* codec, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_codec_options_t* coptions, sk_pmcolor_t ctable[], int* ctableCount)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_codec_result_t)AsCodec(codec)->getPixels(info, pixels, rowBytes, AsCodecOptions(coptions), ctable, ctableCount);
}

sk_codec_result_t sk_codec_get_pixels_using_defaults(sk_codec_t* codec, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_codec_result_t)AsCodec(codec)->getPixels(info, pixels, rowBytes);
}

sk_codec_result_t sk_codec_start_incremental_decode(sk_codec_t* codec, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_codec_options_t* coptions, sk_pmcolor_t ctable[], int* ctableCount)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_codec_result_t)AsCodec(codec)->startIncrementalDecode(info, pixels, rowBytes, AsCodecOptions(coptions), ctable, ctableCount);
}

sk_codec_result_t sk_codec_incremental_decode(sk_codec_t* codec, int* rowsDecoded)
{
    return (sk_codec_result_t)AsCodec(codec)->incrementalDecode(rowsDecoded);
}

sk_codec_result_t sk_codec_start_scanline_decode(sk_codec_t* codec, const sk_imageinfo_t* cinfo, const sk_codec_options_t* coptions, sk_pmcolor_t ctable[], int* ctableCount)
{
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_codec_result_t)AsCodec(codec)->startScanlineDecode(info, AsCodecOptions(coptions), ctable, ctableCount);
}

int sk_codec_get_scanlines(sk_codec_t* codec, void* dst, int countLines, size_t rowBytes)
{
    return AsCodec(codec)->getScanlines(dst, countLines, rowBytes);
}

bool sk_codec_skip_scanlines(sk_codec_t* codec, int countLines)
{
    return AsCodec(codec)->skipScanlines(countLines);
}

sk_codec_scanline_order_t sk_codec_get_scanline_order(sk_codec_t* codec)
{
    return (sk_codec_scanline_order_t)AsCodec(codec)->getScanlineOrder();
}

int sk_codec_next_scanline(sk_codec_t* codec)
{
    return AsCodec(codec)->nextScanline();
}

int sk_codec_output_scanline(sk_codec_t* codec, int inputScanline)
{
    return AsCodec(codec)->outputScanline(inputScanline);
}

int sk_codec_get_frame_count(sk_codec_t* codec) {
    return AsCodec(codec)->getFrameInfo().size();
}

void sk_codec_get_frame_info(sk_codec_t* codec, sk_codec_frameinfo_t* frameInfo) {
    std::vector<SkCodec::FrameInfo> frames = AsCodec(codec)->getFrameInfo();
    size_t size = frames.size();
    SkCodec::FrameInfo* cframes = AsFrameInfo(frameInfo);
    for (size_t i = 0; i < size; i++)
        cframes[i] = frames[i];
}

int sk_codec_get_repetition_count(sk_codec_t* codec) {
    return AsCodec(codec)->getRepetitionCount();
}
