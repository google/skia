/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_image_DEFINED
#define sk_x_image_DEFINED

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
 *  Encode the image's pixels using the specified format and quality
 *  and return the result as a new image in a
 *  sk_data_t, which the caller must manage: call sk_data_unref() when
 *  they are done.
 *
 *  If the image type cannot be encoded, this will return NULL.
 */
SK_API sk_data_t* sk_image_encode_specific(const sk_image_t* cimage, sk_image_encoder_t encoder, int quality);

SK_API sk_image_t* sk_image_new_from_bitmap (const sk_bitmap_t *cbitmap);

SK_C_PLUS_PLUS_END_GUARD

#endif
