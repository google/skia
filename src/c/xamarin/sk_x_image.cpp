/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoder.h"

#include "xamarin\sk_x_image.h"

#include "..\sk_types_priv.h"
#include "sk_x_types_priv.h"

sk_data_t* sk_image_encode_specific(const sk_image_t* cimage, sk_image_encoder_t encoder, int quality) {
    SkImageEncoder::Type t;
    if (!find_sk(encoder, &t)) {
        return NULL;
    }
    return ToData(AsImage(cimage)->encode(t, quality));
}
