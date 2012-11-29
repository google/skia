/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapFactory.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkTemplates.h"

bool SkBitmapFactory::DecodeBitmap(SkBitmap* dst, const SkData* data, Constraints constraint) {
    if (NULL == data || data->size() == 0 || dst == NULL) {
        return false;
    }

    SkMemoryStream stream(data->data(), data->size());
    SkAutoTDelete<SkImageDecoder> decoder (SkImageDecoder::Factory(&stream));
    if (decoder.get() == NULL) {
        return false;
    }

    SkBitmap tmp;
    SkImageDecoder::Mode mode;
    if (kDecodeBoundsOnly_Constraint == constraint) {
        mode = SkImageDecoder::kDecodeBounds_Mode;
    } else {
        mode = SkImageDecoder::kDecodePixels_Mode;
    }

    if (decoder->decode(&stream, &tmp, mode)) {
        tmp.swap(*dst);
        return true;
    } else {
        return false;
    }
}
