/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageDecoder.h"

#include "xamarin/sk_x_imagedecoder.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

void sk_imagedecoder_destructor(sk_imagedecoder_t* cdecoder)
{
    delete AsImageDecoder(cdecoder);
}

sk_imagedecoder_format_t sk_imagedecoder_get_decoder_format(sk_imagedecoder_t* cdecoder)
{
    sk_imagedecoder_format_t cformat;
    if (!find_c(AsImageDecoder(cdecoder)->getFormat(), &cformat)) {
        return cformat;
    }

    return UNKNOWN_SK_IMAGEDECODER_FORMAT;
}

sk_imagedecoder_format_t sk_imagedecoder_get_stream_format(sk_stream_streamrewindable_t* cstream)
{
    sk_imagedecoder_format_t cformat;
    if (find_c(SkImageDecoder::GetStreamFormat(AsStreamRewindable(cstream)), &cformat)) {
        return cformat;
    }
    return UNKNOWN_SK_IMAGEDECODER_FORMAT;
}

sk_string_t* sk_imagedecoder_get_format_name_from_format(sk_imagedecoder_format_t cformat)
{
    SkImageDecoder::Format format;
    if (find_sk(cformat, &format)) {
        return ToString(new SkString(SkImageDecoder::GetFormatName(format)));
    }
    return nullptr;
}

sk_string_t* sk_imagedecoder_get_format_name_from_decoder(sk_imagedecoder_t* cdecoder)
{
    return ToString(new SkString(AsImageDecoder(cdecoder)->getFormatName()));
}

bool sk_imagedecoder_get_skip_writing_zeros(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getSkipWritingZeroes();
}

void sk_imagedecoder_set_skip_writing_zeros(sk_imagedecoder_t* cdecoder, bool skip)
{
    AsImageDecoder(cdecoder)->setSkipWritingZeroes(skip);
}

bool sk_imagedecoder_get_dither_image(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getDitherImage();
}

void sk_imagedecoder_set_dither_image(sk_imagedecoder_t* cdecoder, bool dither)
{
    AsImageDecoder(cdecoder)->setDitherImage(dither);
}

bool sk_imagedecoder_get_prefer_quality_over_speed(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getPreferQualityOverSpeed();
}

void sk_imagedecoder_set_prefer_quality_over_speed(sk_imagedecoder_t* cdecoder, bool qualityOverSpeed)
{
    AsImageDecoder(cdecoder)->setPreferQualityOverSpeed(qualityOverSpeed);
}

bool sk_imagedecoder_get_require_unpremultiplied_colors(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getRequireUnpremultipliedColors();
}

void sk_imagedecoder_set_require_unpremultiplied_colors(sk_imagedecoder_t* cdecoder, bool request)
{
    AsImageDecoder(cdecoder)->setRequireUnpremultipliedColors(request);
}

int sk_imagedecoder_get_sample_size(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getSampleSize();
}

void sk_imagedecoder_set_sample_size(sk_imagedecoder_t* cdecoder, int size)
{
    AsImageDecoder(cdecoder)->setSampleSize(size);
}

void sk_imagedecoder_cancel_decode(sk_imagedecoder_t* cdecoder)
{
    AsImageDecoder(cdecoder)->cancelDecode();
}

bool sk_imagedecoder_should_cancel_decode(sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->shouldCancelDecode();
}

sk_imagedecoder_result_t sk_imagedecoder_decode(sk_imagedecoder_t* cdecoder, sk_stream_t* cstream, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode)
{
    SkImageDecoder* decoder = AsImageDecoder(cdecoder);

    SkImageDecoder::Mode skmode;
    if (!find_sk(mode, &skmode)) {
        skmode = SkImageDecoder::kDecodePixels_Mode;
    }

    SkColorType skpref;
    if (!find_sk(pref, &skpref)) {
        skpref = SkColorType::kUnknown_SkColorType;
    }

    SkImageDecoder::Result skresult = decoder->decode(AsStream(cstream), AsBitmap(bitmap), skpref, skmode);

    sk_imagedecoder_result_t cresult;
    if (!find_c(skresult, &cresult)) {
        cresult = FAILURE_SK_IMAGEDECODER_RESULT;
    }

    return cresult;
}

sk_imagedecoder_t* sk_imagedecoder_factory(sk_stream_streamrewindable_t* cstream)
{
    return (sk_imagedecoder_t*)SkImageDecoder::Factory(AsStreamRewindable(cstream));
}

bool sk_imagedecoder_decode_file(const char* file, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format)
{
    SkImageDecoder::Mode skmode;
    if (!find_sk(mode, &skmode)) {
        skmode = SkImageDecoder::kDecodePixels_Mode;
    }

    SkColorType skpref;
    if (!find_sk(pref, &skpref)) {
        skpref = SkColorType::kUnknown_SkColorType;
    }

    SkImageDecoder::Format skformat;
    bool result = SkImageDecoder::DecodeFile(file, AsBitmap(bitmap), skpref, skmode, &skformat);

    if (!find_c(skformat, format)) {
        *format = UNKNOWN_SK_IMAGEDECODER_FORMAT;
    }

    return result;
}

bool sk_imagedecoder_decode_memory(const void* buffer, size_t size, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format)
{
    SkImageDecoder::Mode skmode;
    if (!find_sk(mode, &skmode)) {
        skmode = SkImageDecoder::kDecodePixels_Mode;
    }

    SkColorType skpref;
    if (!find_sk(pref, &skpref)) {
        skpref = SkColorType::kUnknown_SkColorType;
    }

    SkImageDecoder::Format skformat;
    bool result = SkImageDecoder::DecodeMemory(buffer, size, AsBitmap(bitmap), skpref, skmode, &skformat);

    if (!find_c(skformat, format)) {
        *format = UNKNOWN_SK_IMAGEDECODER_FORMAT;
    }

    return result;
}

bool sk_imagedecoder_decode_stream(sk_stream_streamrewindable_t* cstream,
    sk_bitmap_t* bitmap,
    sk_colortype_t pref,
    sk_imagedecoder_mode_t mode,
    sk_imagedecoder_format_t* format)
{
    SkImageDecoder::Mode skmode;
    if (!find_sk(mode, &skmode)) {
        skmode = SkImageDecoder::kDecodePixels_Mode;
    }

    SkColorType skpref;
    if (!find_sk(pref, &skpref)) {
        skpref = SkColorType::kUnknown_SkColorType;
    }

    SkImageDecoder::Format skformat;
    bool result = SkImageDecoder::DecodeStream(AsStreamRewindable(cstream), AsBitmap(bitmap), skpref, skmode, &skformat);

    if (!find_c(skformat, format)) {
        *format = UNKNOWN_SK_IMAGEDECODER_FORMAT;
    }

    return result;
}
