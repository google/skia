/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoder.h"

#include "sk_image.h"

#include "sk_types_priv.h"

sk_data_t* sk_image_encode_specific(const sk_image_t* cimage, sk_image_encoder_t encoder, int quality) {
    return ToData(AsImage(cimage)->encode((SkImageEncoder::Type)encoder, quality));
}

sk_image_t* sk_image_new_from_bitmap (const sk_bitmap_t* cbitmap)
{
    return ToImage(SkImage::MakeFromBitmap(*AsBitmap(cbitmap)).release());
}

sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t* cinfo, const void* pixels,
                                     size_t rowBytes) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    return (sk_image_t*)SkImage::MakeRasterCopy(SkPixmap(info, pixels, rowBytes)).release();
}

sk_image_t* sk_image_new_from_encoded(const sk_data_t* cdata, const sk_irect_t* subset) {
    return ToImage(SkImage::MakeFromEncoded(sk_ref_sp(AsData(cdata)),
                                           reinterpret_cast<const SkIRect*>(subset)).release());
}

sk_data_t* sk_image_encode(const sk_image_t* cimage) {
    return ToData(AsImage(cimage)->encode());
}

void sk_image_ref(const sk_image_t* cimage) {
    AsImage(cimage)->ref();
}

void sk_image_unref(const sk_image_t* cimage) {
    SkSafeUnref(AsImage(cimage));
}

int sk_image_get_width(const sk_image_t* cimage) {
    return AsImage(cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return AsImage(cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return AsImage(cimage)->uniqueID();
}
