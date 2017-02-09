/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnPreMultiplyPriv_DEFINED
#define SkUnPreMultiplyPriv_DEFINED

#include "SkColor.h"

template <bool kSwapRB>
void SkUnpremultiplyRow(uint32_t* dst, const uint32_t* src, int count) {
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < count; i++) {
        uint32_t c = *src++;
        uint8_t r, g, b, a;
        if (kSwapRB) {
            r = (c >> 16) & 0xFF;
            g = (c >>  8) & 0xFF;
            b = (c >>  0) & 0xFF;
            a = (c >> 24) & 0xFF;
        } else {
            r = (c >>  0) & 0xFF;
            g = (c >>  8) & 0xFF;
            b = (c >> 16) & 0xFF;
            a = (c >> 24) & 0xFF;
        }

        if (0 != a && 255 != a) {
            SkUnPreMultiply::Scale scale = table[a];
            r = SkUnPreMultiply::ApplyScale(scale, r);
            g = SkUnPreMultiply::ApplyScale(scale, g);
            b = SkUnPreMultiply::ApplyScale(scale, b);
        }

        *dst++ = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }
}

#endif
