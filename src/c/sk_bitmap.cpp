/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkDither.h"

#include "sk_bitmap.h"
#include "sk_types_priv.h"


static inline void copyAlpha8ToColor(size_t size, const uint8_t* pixels, sk_color_t* colors)
{
    while (size-- != 0) {
        const uint8_t* addr = pixels++;
        *colors++ = SkColorSetA(0, *addr);
    }
}
static inline void copyRgb565ToColor(size_t size, const uint16_t* pixels, sk_color_t* colors)
{
    while (size-- != 0) {
        const uint16_t* addr = pixels++;
        *colors++ = SkPixel16ToColor(*addr);
    }
}
static inline void copy8888ToColor(size_t size, const uint32_t* pixels, sk_color_t* colors)
{
    while (size-- != 0) {
        const uint32_t* addr = pixels++;
        *colors++ = SkUnPreMultiply::PMColorToColor(*addr);
    }
}

static inline void copyAlpha8FromColor(size_t size, const sk_color_t* colors, uint8_t* pixels)
{
    while (size-- != 0) {
        *pixels++ = SkColorGetA(*colors++);
    }
}
static inline void copyRgb565FromColor(size_t width, size_t height, const sk_color_t* colors, uint16_t* pixels)
{
    for (size_t y = 0; y < height; y++) {
        DITHER_565_SCAN(y);
        for (size_t x = 0; x < width; x++) {
            SkColor c = *colors++;
            *pixels++ = SkDitherRGBTo565(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), DITHER_VALUE(x));
        }
    }
}
static inline void copy8888FromColor(size_t size, const sk_color_t* colors, uint32_t* pixels)
{
    while (size-- != 0) {
        *pixels++ = SkPreMultiplyColor(*colors++);
    }
}


void sk_bitmap_destructor (sk_bitmap_t* cbitmap)
{
    delete AsBitmap(cbitmap);
}

sk_bitmap_t* sk_bitmap_new ()
{
    return (sk_bitmap_t*) new SkBitmap();
}

bool sk_bitmap_get_info(sk_bitmap_t* cbitmap, sk_imageinfo_t* info)
{
    sk_imageinfo_t cinfo;
    bool result = find_c(AsBitmap(cbitmap)->info(), &cinfo);
    if (result) {
        *info = cinfo;
    }
    return result;
}

void* sk_bitmap_get_pixels(sk_bitmap_t* cbitmap, size_t* length)
{
    SkBitmap* bmp = AsBitmap(cbitmap);
    *length = bmp->getSize();
    return bmp->getPixels();
}

size_t sk_bitmap_get_row_bytes(sk_bitmap_t* cbitmap)
{
    return AsBitmap(cbitmap)->rowBytes();
}

size_t sk_bitmap_get_byte_count(sk_bitmap_t* cbitmap)
{
    return AsBitmap(cbitmap)->getSize();
}

void sk_bitmap_reset(sk_bitmap_t* cbitmap)
{
    AsBitmap(cbitmap)->reset();
}

bool sk_bitmap_is_null(sk_bitmap_t* cbitmap)
{
    return AsBitmap(cbitmap)->isNull();
}

bool sk_bitmap_is_immutable(sk_bitmap_t* cbitmap)
{
    return AsBitmap(cbitmap)->isImmutable();
}

void sk_bitmap_set_immutable(sk_bitmap_t* cbitmap)
{
    AsBitmap(cbitmap)->setImmutable();
}

bool sk_bitmap_is_volatile(sk_bitmap_t* cbitmap)
{
    return AsBitmap(cbitmap)->isVolatile();
}

void sk_bitmap_set_volatile(sk_bitmap_t* cbitmap, bool value)
{
    AsBitmap(cbitmap)->setIsVolatile(value);
}

void sk_bitmap_erase(sk_bitmap_t* cbitmap, sk_color_t color)
{
    AsBitmap(cbitmap)->eraseColor(color);
}

void sk_bitmap_erase_rect(sk_bitmap_t* cbitmap, sk_color_t color, sk_irect_t* rect)
{
    AsBitmap(cbitmap)->erase(color, AsIRect(*rect));
}

sk_color_t sk_bitmap_get_pixel_color(sk_bitmap_t* cbitmap, int x, int y)
{
    return AsBitmap(cbitmap)->getColor(x, y);
}

void sk_bitmap_set_pixel_color(sk_bitmap_t* cbitmap, int x, int y, sk_color_t color)
{
    SkBitmap* bmp = AsBitmap(cbitmap);

    SkAutoLockPixels alp(*bmp);

    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8FromColor(1, &color, (uint8_t*)bmp->getAddr8(x, y));
        break;
    case kRGB_565_SkColorType:
        copyRgb565FromColor(1, 1, &color, (uint16_t*)bmp->getAddr16(x, y));
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888FromColor(1, &color, (uint32_t*)bmp->getAddr32(x, y));
        break;
    default:
        break;
    }
}

bool sk_bitmap_copy(sk_bitmap_t* cbitmap, sk_bitmap_t* dst, sk_colortype_t ct)
{
    SkColorType skct;
    if (!find_sk(ct, &skct)) {
        skct = SkColorType::kUnknown_SkColorType;
    }
    return AsBitmap(cbitmap)->copyTo(AsBitmap(dst), skct);
}

bool sk_bitmap_can_copy_to(sk_bitmap_t* cbitmap, sk_colortype_t ct)
{
    SkColorType skct;
    if (!find_sk(ct, &skct)) {
        skct = SkColorType::kUnknown_SkColorType;
    }
    return AsBitmap(cbitmap)->canCopyTo(skct);
}

void sk_bitmap_unlock_pixels(sk_bitmap_t* cbitmap)
{
    AsBitmap(cbitmap)->unlockPixels();
}

void sk_bitmap_lock_pixels(sk_bitmap_t* cbitmap)
{
    AsBitmap(cbitmap)->lockPixels();
}

void sk_bitmap_get_pixel_colors(sk_bitmap_t* cbitmap, sk_color_t* colors)
{
    SkBitmap* bmp = AsBitmap(cbitmap);

    SkAutoLockPixels alp(*bmp);

    size_t size = bmp->height() * bmp->width();
    const void* pixels = bmp->getPixels();
    
    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8ToColor(size, (const uint8_t*)pixels, colors);
        break;
    case kRGB_565_SkColorType:
        copyRgb565ToColor(size, (const uint16_t*)pixels, colors);
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888ToColor(size, (const uint32_t*)pixels, colors);
        break;
    default:
        break;
    }
}

void sk_bitmap_set_pixel_colors(sk_bitmap_t* cbitmap, const sk_color_t* colors)
{
    SkBitmap* bmp = AsBitmap(cbitmap);

    SkAutoLockPixels alp(*bmp);

    size_t width = bmp->width();
    size_t height = bmp->height();
    size_t size = height * width;
    void* pixels = bmp->getPixels();
    
    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8FromColor(size, colors, (uint8_t*)pixels);
        break;
    case kRGB_565_SkColorType:
        copyRgb565FromColor(width, height, colors, (uint16_t*)pixels);
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888FromColor(size, colors, (uint32_t*)pixels);
        break;
    default:
        break;
    }
}

bool sk_bitmap_try_alloc_pixels(sk_bitmap_t* cbitmap, const sk_imageinfo_t* requestedInfo, size_t rowBytes)
{
    SkBitmap* bmp = AsBitmap(cbitmap);
    
    SkImageInfo info;
    if (!find_sk(*requestedInfo, &info)) {
        return false;
    }
    
    return bmp->tryAllocPixels(info, rowBytes);
}


sk_imagedecoder_format_t sk_imagedecoder_get_decoder_format (sk_imagedecoder_t* cdecoder)
{
    sk_imagedecoder_format_t cformat;
    if (!find_c(AsImageDecoder(cdecoder)->getFormat(), &cformat)) {
        return cformat;
    }

    return UNKNOWN_SK_IMAGEDECODER_FORMAT;
}

sk_imagedecoder_format_t sk_imagedecoder_get_stream_format (sk_stream_streamrewindable_t* cstream)
{
    sk_imagedecoder_format_t cformat;
    if (find_c(SkImageDecoder::GetStreamFormat(AsStreamRewindable(cstream)), &cformat)) {
        return cformat;
    }
    return UNKNOWN_SK_IMAGEDECODER_FORMAT;
}

const char* sk_imagedecoder_get_format_name_from_format (sk_imagedecoder_format_t cformat)
{
    SkImageDecoder::Format format;
    if (find_sk(cformat, &format)) {
        return SkImageDecoder::GetFormatName(format);
    }
    return nullptr;
}

const char* sk_imagedecoder_get_format_name_from_decoder (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getFormatName();
}

bool sk_imagedecoder_get_skip_writing_zeros (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getSkipWritingZeroes();
}

void sk_imagedecoder_set_skip_writing_zeros (sk_imagedecoder_t* cdecoder, bool skip)
{
    AsImageDecoder(cdecoder)->setSkipWritingZeroes(skip);
}

bool sk_imagedecoder_get_dither_image (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getDitherImage();
}

void sk_imagedecoder_set_dither_image (sk_imagedecoder_t* cdecoder, bool dither)
{
    AsImageDecoder(cdecoder)->setDitherImage(dither);
}

bool sk_imagedecoder_get_prefer_quality_over_speed (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getPreferQualityOverSpeed();
}

void sk_imagedecoder_set_prefer_quality_over_speed (sk_imagedecoder_t* cdecoder, bool qualityOverSpeed)
{
    AsImageDecoder(cdecoder)->setPreferQualityOverSpeed(qualityOverSpeed);
}

bool sk_imagedecoder_get_require_unpremultiplied_colors (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getRequireUnpremultipliedColors();
}

void sk_imagedecoder_set_require_unpremultiplied_colors (sk_imagedecoder_t* cdecoder, bool request)
{
    AsImageDecoder(cdecoder)->setRequireUnpremultipliedColors(request);
}

int sk_imagedecoder_get_sample_size (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->getSampleSize();
}

void sk_imagedecoder_set_sample_size (sk_imagedecoder_t* cdecoder, int size)
{
    AsImageDecoder(cdecoder)->setSampleSize(size);
}

void sk_imagedecoder_cancel_decode (sk_imagedecoder_t* cdecoder)
{
    AsImageDecoder(cdecoder)->cancelDecode();
}

bool sk_imagedecoder_should_cancel_decode (sk_imagedecoder_t* cdecoder)
{
    return AsImageDecoder(cdecoder)->shouldCancelDecode();
}

sk_imagedecoder_result_t sk_imagedecoder_decode (sk_imagedecoder_t* cdecoder, sk_stream_t* cstream, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode)
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

sk_imagedecoder_t* sk_imagedecoder_factory (sk_stream_streamrewindable_t* cstream)
{
    return (sk_imagedecoder_t*)SkImageDecoder::Factory(AsStreamRewindable(cstream));
}

bool sk_imagedecoder_decode_file (const char* file, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format)
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

bool sk_imagedecoder_decode_memory (const void* buffer, size_t size, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format)
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

bool sk_imagedecoder_decode_stream (sk_stream_streamrewindable_t* cstream, 
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

